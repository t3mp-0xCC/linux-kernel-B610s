/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "yaffs_nand.h"
#include "yaffs_tagscompat.h"

#include "yaffs_getblockinfo.h"
#include "yaffs_summary.h"

#include "linux/mtd/mtd.h"
#include "yaffs_linux.h"

int yaffs_add_prior_gc_chunk(struct yaffs_dev *dev, int nand_chunk, unsigned obj_id)
{
    struct yaffs_gc_chunks *gc_chunk = NULL;
    struct yaffs_gc_chunks *p_gc_chunk = NULL;

    if (!dev->param.scan_ecc_enable)
        return 0;

    /* If obj_id:0 this is not used */
    if ((obj_id == 0) ||
            obj_id == YAFFS_OBJECTID_CHECKPOINT_DATA)
    {
        return 0;
    }

    yaffs_trace(YAFFS_TRACE_ERROR, "---yaffs_add_prior_gc_chunk---");
    p_gc_chunk = dev->prior_gc_chunk_head;
    while (p_gc_chunk)
    {
        if (nand_chunk == p_gc_chunk->chunk_id)
        {
            yaffs_trace(YAFFS_TRACE_ALWAYS, "The chunk has already in the gc  %d", nand_chunk);
            return 0;
        }

        p_gc_chunk = p_gc_chunk->next;
    }

    gc_chunk = (struct yaffs_gc_chunks *)kmalloc(sizeof(struct yaffs_gc_chunks), GFP_KERNEL);
    if (!gc_chunk)
    {
        yaffs_trace(YAFFS_TRACE_ALWAYS, "malloc gc_chunk failed!");
        return - ENOMEM;
    }

    gc_chunk->chunk_id = nand_chunk;
    gc_chunk->obj_id = obj_id;
    gc_chunk->next = NULL;

    if (!dev->prior_gc_chunk_head)
    {
        dev->prior_gc_chunk_head = gc_chunk;
    }
    else
    {
        p_gc_chunk = dev->prior_gc_chunk_head;
        while (p_gc_chunk->next)
        {
            p_gc_chunk = p_gc_chunk->next;
        }
        p_gc_chunk->next = gc_chunk;
    }

    yaffs_trace(YAFFS_TRACE_ALWAYS,
                "add chunk success, chunk id [%d] obj id [%d]",
                gc_chunk->chunk_id, gc_chunk->obj_id);

    return 0;
}

void yaffs_free_prior_gc_chunk(struct yaffs_dev *dev)
{
    struct yaffs_gc_chunks *gc_chunk = NULL;

    if (NULL == dev)
    {
        yaffs_trace(YAFFS_TRACE_ALWAYS,
                    "dev null, free prior gc chunk failed!");
        return;
    }

    if (!dev->param.scan_ecc_enable)
        return;

    while (dev->prior_gc_chunk_head)
    {
        gc_chunk = dev->prior_gc_chunk_head;
        dev->prior_gc_chunk_head = dev->prior_gc_chunk_head->next;
        kfree(gc_chunk);
    }

    dev->prior_gc_chunk_head = NULL;
}
static int apply_chunk_offset(struct yaffs_dev *dev, int chunk)
{
	return chunk - dev->chunk_offset;
}

int yaffs_rd_chunk_tags_nand(struct yaffs_dev *dev, int nand_chunk,
			     u8 *buffer, struct yaffs_ext_tags *tags)
{
	int result;
	struct yaffs_ext_tags local_tags;
        struct mtd_info *mtd = yaffs_dev_to_mtd(dev);
	int flash_chunk = apply_chunk_offset(dev, nand_chunk);

	dev->n_page_reads++;

	/* If there are no tags provided use local tags. */
	if (!tags)
		tags = &local_tags;

	result = dev->tagger.read_chunk_tags_fn(dev, flash_chunk, buffer, tags);
    if (tags && (tags->ecc_result > YAFFS_ECC_RESULT_FIXED \
            || (tags->ecc_result == YAFFS_ECC_RESULT_FIXED && mtd->exceed_threshold)))
    {

		struct yaffs_block_info *bi;
		bi = yaffs_get_block_info(dev,
					  nand_chunk /
					  dev->param.chunks_per_block);
		yaffs_handle_chunk_error(dev, bi);
	}

	return result;
}

int yaffs_wr_chunk_tags_nand(struct yaffs_dev *dev,
				int nand_chunk,
				const u8 *buffer, struct yaffs_ext_tags *tags)
{
	int result;
	int flash_chunk = apply_chunk_offset(dev, nand_chunk);

	dev->n_page_writes++;

	if (!tags) {
		yaffs_trace(YAFFS_TRACE_ERROR, "Writing with no tags");
		BUG();
		return YAFFS_FAIL;
	}

	tags->seq_number = dev->seq_number;
	tags->chunk_used = 1;
	yaffs_trace(YAFFS_TRACE_WRITE,
		"Writing chunk %d tags %d %d",
		nand_chunk, tags->obj_id, tags->chunk_id);

	result = dev->tagger.write_chunk_tags_fn(dev, flash_chunk,
							buffer, tags);

	yaffs_summary_add(dev, tags, nand_chunk);

	return result;
}

int yaffs_mark_bad(struct yaffs_dev *dev, int block_no)
{
	block_no -= dev->block_offset;
	dev->n_bad_markings++;

	if (dev->param.disable_bad_block_marking)
		return YAFFS_OK;

	return dev->tagger.mark_bad_fn(dev, block_no);
}


int yaffs_query_init_block_state(struct yaffs_dev *dev,
				 int block_no,
				 enum yaffs_block_state *state,
				 u32 *seq_number)
{
	block_no -= dev->block_offset;
	return dev->tagger.query_block_fn(dev, block_no, state, seq_number);
}

int yaffs_erase_block(struct yaffs_dev *dev, int block_no)
{
	int result;

	block_no -= dev->block_offset;
	dev->n_erasures++;
	result = dev->drv.drv_erase_fn(dev, block_no);
	return result;
}

int yaffs_init_nand(struct yaffs_dev *dev)
{
	if (dev->drv.drv_initialise_fn)
		return dev->drv.drv_initialise_fn(dev);
	return YAFFS_OK;
}

int yaffs_deinit_nand(struct yaffs_dev *dev)
{
	if (dev->drv.drv_deinitialise_fn)
		return dev->drv.drv_deinitialise_fn(dev);
	return YAFFS_OK;
}
