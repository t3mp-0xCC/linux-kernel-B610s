/* Copyright (c) 2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/clk.h>
#include <linux/of_platform.h>
#include <linux/debugfs.h>

#include "coresight-priv.h"
#include "coresight.h"

#define NO_SINK		(-1)

struct dentry *debug_parent;

static int curr_sink = NO_SINK;
static LIST_HEAD(coresight_orph_conns);
static LIST_HEAD(coresight_devs);
static DEFINE_SEMAPHORE(coresight_mutex);

static int coresight_find_link_inport(struct coresight_device *csdev)
{
	int i;
	struct coresight_device *parent;
	struct coresight_connection *conn;

	parent = container_of(csdev->path_link.next, struct coresight_device,
			     path_link);
	for (i = 0; i < parent->nr_conns; i++) {
		conn = &parent->conns[i];
		if (conn->child_dev == csdev)
			return conn->child_port;
	}

	pr_err("coresight: couldn't find inport, parent: %d, child: %d\n",
	       parent->id, csdev->id);
	return 0;
}

static int coresight_find_link_outport(struct coresight_device *csdev)
{
	int i;
	struct coresight_device *child;
	struct coresight_connection *conn;

	child = container_of(csdev->path_link.prev, struct coresight_device,
			      path_link);
	for (i = 0; i < csdev->nr_conns; i++) {
		conn = &csdev->conns[i];
		if (conn->child_dev == child)
			return conn->outport;
	}

	pr_err("coresight: couldn't find outport, parent: %d, child: %d\n",
	       csdev->id, child->id);
	return 0;
}

static int coresight_enable_sink(struct coresight_device *csdev)
{
	int ret;

	if (csdev->refcnt.sink_refcnt == 0) {
		if (csdev->ops->sink_ops->enable) {
			ret = csdev->ops->sink_ops->enable(csdev);
			if (ret)
				goto err;
			csdev->enable = true;
		}
	}
	csdev->refcnt.sink_refcnt++;

	return 0;
err:
	return ret;
}

static void coresight_disable_sink(struct coresight_device *csdev)
{
	if (csdev->refcnt.sink_refcnt == 1) {
		if (csdev->ops->sink_ops->disable) {
			csdev->ops->sink_ops->disable(csdev);
			csdev->enable = false;
		}
	}
	csdev->refcnt.sink_refcnt--;
}

static int coresight_enable_link(struct coresight_device *csdev)
{
	int ret;
	int link_subtype;
	int refport, inport, outport;

	inport = coresight_find_link_inport(csdev);
	outport = coresight_find_link_outport(csdev);

	link_subtype = csdev->subtype.link_subtype;
	if (link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_MERG)
		refport = inport;
	else if (link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_SPLIT)
		refport = outport;
	else
		refport = 0;

	if (csdev->refcnt.link_refcnts[refport] == 0) {
		if (csdev->ops->link_ops->enable) {
			ret = csdev->ops->link_ops->enable(csdev, inport,
							   outport);
			if (ret)
				goto err;
			csdev->enable = true;
		}
	}
	csdev->refcnt.link_refcnts[refport]++;

	return 0;
err:
	return ret;
}

static void coresight_disable_link(struct coresight_device *csdev)
{
	int link_subtype;
	int refport, inport, outport;

	inport = coresight_find_link_inport(csdev);
	outport = coresight_find_link_outport(csdev);

	link_subtype = csdev->subtype.link_subtype;
	if (link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_MERG)
		refport = inport;
	else if (link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_SPLIT)
		refport = outport;
	else
		refport = 0;

	if (csdev->refcnt.link_refcnts[refport] == 1) {
		if (csdev->ops->link_ops->disable) {
			csdev->ops->link_ops->disable(csdev, inport, outport);
			csdev->enable = false;
		}
	}
	csdev->refcnt.link_refcnts[refport]--;
}

static int coresight_enable_source(struct coresight_device *csdev)
{
	int ret;

	if (csdev->refcnt.source_refcnt == 0) {
		if (csdev->ops->source_ops->enable) {
			ret = csdev->ops->source_ops->enable(csdev);
			if (ret)
				goto err;
			csdev->enable = true;
		}
	}
	csdev->refcnt.source_refcnt++;

	return 0;
err:
	return ret;
}

static void coresight_disable_source(struct coresight_device *csdev)
{
	if (csdev->refcnt.source_refcnt == 1) {
		if (csdev->ops->source_ops->disable) {
			csdev->ops->source_ops->disable(csdev);
			csdev->enable = false;
		}
	}
	csdev->refcnt.source_refcnt--;
}

static struct list_head *coresight_build_path(struct coresight_device *csdev,
					      struct list_head *path)
{
	int i;
	struct list_head *p;
	struct coresight_connection *conn;

	if (csdev->id == curr_sink) {
		list_add_tail(&csdev->path_link, path);
		return path;
	}

	for (i = 0; i < csdev->nr_conns; i++) {
		conn = &csdev->conns[i];
		p = coresight_build_path(conn->child_dev, path);
		if (p) {
			list_add_tail(&csdev->path_link, p);
			return p;
		}
	}

    /* reach the last node, node type: SINK or LINKSINK */
    if((0 == csdev->nr_conns) && (csdev->type == CORESIGHT_DEV_TYPE_LINKSINK || csdev->type == CORESIGHT_DEV_TYPE_SINK))
    {
        list_add_tail(&csdev->path_link, path);
        return path;
    }

	return NULL;
}

static void coresight_release_path(struct list_head *path)
{
	struct coresight_device *cd, *temp;

	list_for_each_entry_safe(cd, temp, path, path_link)
		list_del(&cd->path_link);
}

void coresight_set_path_to_enable(struct coresight_device * csdev)
{
    struct coresight_device *cd;
    int link_subtype;
    int refport, inport, outport;
    LIST_HEAD(path);

	if (csdev->type != CORESIGHT_DEV_TYPE_SOURCE) {
		printk("%s: wrong device type in 0x%x\n", __FUNCTION__, csdev->type);
		return;
	}

    down(&coresight_mutex);
	coresight_build_path(csdev, &path);

	list_for_each_entry(cd, &path, path_link) 
    {
		if(cd == list_first_entry(&path, struct coresight_device, path_link))
        {
			cd->refcnt.sink_refcnt++;
		}
        else if(list_is_last(&cd->path_link, &path))
        {
            cd->refcnt.source_refcnt++;
		}
        else
        {
            inport = coresight_find_link_inport(cd);
            outport = coresight_find_link_outport(cd);
            
            link_subtype = cd->subtype.link_subtype;
            if (link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_MERG)
            {
                refport = inport;
            }
            else if(link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_SPLIT)
            {
                refport = outport;
            }
            else
            {
                refport = 0;
            }
            
            cd->refcnt.link_refcnts[refport]++;
		}

        cd->enable = true;
	}

    coresight_release_path(&path);
    up(&coresight_mutex);
}

void coresight_get_path_status(struct coresight_device * csdev)
{
    struct coresight_device *cd;
    int link_subtype;
    int refport, inport, outport;
    LIST_HEAD(path);

	if (csdev->type != CORESIGHT_DEV_TYPE_SOURCE) {
		printk("%s: wrong device type in 0x%x\n", __FUNCTION__, csdev->type);
		return;
	}

    down(&coresight_mutex);
	coresight_build_path(csdev, &path);

    printk("coresight path status:\n");
	list_for_each_entry(cd, &path, path_link) 
    {
		if(cd == list_first_entry(&path, struct coresight_device, path_link))
        {
			printk("sink refcnt :%d status: %s\n", cd->refcnt.sink_refcnt, (cd->enable == true)?"enable":"disable");
		}
        else if(list_is_last(&cd->path_link, &path))
        {
            printk("soure refcnt:%d status: %s\n", cd->refcnt.source_refcnt, (cd->enable == true)?"enable":"disable");
		}
        else
        {
            inport = coresight_find_link_inport(cd);
            outport = coresight_find_link_outport(cd);
            
            link_subtype = cd->subtype.link_subtype;
            if (link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_MERG)
            {
                refport = inport;
            }
            else if(link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_SPLIT)
            {
                refport = outport;
            }
            else
            {
                refport = 0;
            }
            
            printk("link refcnt :%d status: %s\n", cd->refcnt.link_refcnts[refport], (cd->enable == true)?"enable":"disable");
		}
	}

    coresight_release_path(&path);
    up(&coresight_mutex);
}

static int coresight_enable_path(struct list_head *path, bool incl_source)
{
	int ret = 0;
	struct coresight_device *cd;

	list_for_each_entry(cd, path, path_link) {
		if (cd == list_first_entry(path, struct coresight_device,
					   path_link)) {
			ret = coresight_enable_sink(cd);
		} else if (list_is_last(&cd->path_link, path)) {
			if (incl_source)
				ret = coresight_enable_source(cd);
		} else {
			ret = coresight_enable_link(cd);
		}
		if (ret)
			goto err;
	}
	return 0;
err:
	list_for_each_entry_continue_reverse(cd, path, path_link) {
		if (cd == list_first_entry(path, struct coresight_device,
					   path_link)) {
			coresight_disable_sink(cd);
		} else if (list_is_last(&cd->path_link, path)) {
			if (incl_source)
				coresight_disable_source(cd);
		} else {
			coresight_disable_link(cd);
		}
	}
	return ret;
}

static void coresight_disable_path(struct list_head *path, bool incl_source)
{
	struct coresight_device *cd;

	list_for_each_entry(cd, path, path_link) {
		if (cd == list_first_entry(path, struct coresight_device,
					   path_link)) {
			coresight_disable_sink(cd);
		} else if (list_is_last(&cd->path_link, path)) {
			if (incl_source)
				coresight_disable_source(cd);
		} else {
			coresight_disable_link(cd);
		}
	}
}

static int coresight_switch_sink(struct coresight_device *csdev)
{
	int ret = 0;
	LIST_HEAD(path);
	struct coresight_device *cd;

	if (IS_ERR_OR_NULL(csdev))
		return -EINVAL;

	down(&coresight_mutex);
	if (csdev->id == curr_sink)
		goto out;

	list_for_each_entry(cd, &coresight_devs, dev_link) {
		if (cd->type == CORESIGHT_DEV_TYPE_SOURCE && cd->enable) {
			coresight_build_path(cd, &path);
			coresight_disable_path(&path, false);
			coresight_release_path(&path);
		}
	}
	curr_sink = csdev->id;
	list_for_each_entry(cd, &coresight_devs, dev_link) {
		if (cd->type == CORESIGHT_DEV_TYPE_SOURCE && cd->enable) {
			coresight_build_path(cd, &path);
			ret = coresight_enable_path(&path, false);
			coresight_release_path(&path);
			if (ret)
				goto err;
		}
	}
out:
	up(&coresight_mutex);
	return 0;
err:
	list_for_each_entry(cd, &coresight_devs, dev_link) {
		if (cd->type == CORESIGHT_DEV_TYPE_SOURCE && cd->enable)
			coresight_disable_source(cd);
	}
	pr_err("coresight: sink switch failed, sources disabled; try again\n");
	return ret;
}

int coresight_enable(struct coresight_device *csdev)
{
	int ret = 0;
	LIST_HEAD(path);

	if (IS_ERR_OR_NULL(csdev))
		return -EINVAL;

	/* down(&coresight_mutex); */
	if (csdev->type != CORESIGHT_DEV_TYPE_SOURCE) {
		ret = -EINVAL;
		/* pr_err("coresight: wrong device type in %s\n", __func__); */
		goto out;
	}
	if (csdev->enable)
		goto out;

	coresight_build_path(csdev, &path);
	ret = coresight_enable_path(&path, true);
	coresight_release_path(&path);
/*
    if (ret)
		pr_err("coresight: enable failed\n");
*/
out:
	/* up(&coresight_mutex); */
	return ret;
}
EXPORT_SYMBOL_GPL(coresight_enable);

void coresight_disable(struct coresight_device *csdev)
{
	LIST_HEAD(path);

	if (IS_ERR_OR_NULL(csdev))
		return;

	/* down(&coresight_mutex); */
	if (csdev->type != CORESIGHT_DEV_TYPE_SOURCE) {
		/* pr_err("coresight: wrong device type in %s\n", __func__); */
		goto out;
	}
	if (!csdev->enable)
		goto out;

	coresight_build_path(csdev, &path);
	coresight_disable_path(&path, true);
	coresight_release_path(&path);
out:
	/* up(&coresight_mutex); */
    return;
}
EXPORT_SYMBOL_GPL(coresight_disable);

void coresight_abort(void)
{
	struct coresight_device *cd;

	if (down_trylock(&coresight_mutex)) {
		pr_err("coresight: abort could not be processed\n");
		return;
	}
	if (curr_sink == NO_SINK)
		goto out;

	list_for_each_entry(cd, &coresight_devs, dev_link) {
		if (cd->id == curr_sink) {
			if (cd->enable && cd->ops->sink_ops->abort) {
				cd->ops->sink_ops->abort(cd);
				cd->enable = false;
			}
		}
	}
out:
	up(&coresight_mutex);
}
EXPORT_SYMBOL_GPL(coresight_abort);

static ssize_t debugfs_curr_sink_write(struct file *file,
				       const char __user *buf,
				       size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	struct coresight_device *csdev = file->private_data;

	if (sscanf(buf, "%lx", &val) != 1)
		return -EINVAL;

	if (val) {
		ret = coresight_switch_sink(csdev);
		if (ret)
			return ret;
	} else
		return -EINVAL;

	return count;
}

static ssize_t debugfs_curr_sink_read(struct file *file, char __user *user_buf,
				      size_t count, loff_t *ppos)
{
	int ret;
	struct coresight_device *csdev = file->private_data;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);

	ret =  scnprintf(buf, PAGE_SIZE, "%u\n",
			 csdev->id == curr_sink ? 1 : 0);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static const struct file_operations debugfs_curr_sink_ops = {
	.open = simple_open,
	.read = debugfs_curr_sink_read,
	.write = debugfs_curr_sink_write,
};

static const struct coresight_ops_entry debugfs_curr_sink_entry = {
	.name = "curr_sink",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_curr_sink_ops,
};

static ssize_t debugfs_enable_write(struct file *file,
				    const char __user *buf,
				    size_t count, loff_t *ppos)
{
	int ret;
	unsigned long val;
	struct coresight_device *csdev = file->private_data;

	if (sscanf(buf, "%lx", &val) != 1)
		return -EINVAL;

	if (val) {
		ret = coresight_enable(csdev);
		if (ret)
			return ret;
	} else
		coresight_disable(csdev);

	return count;
}

static ssize_t debugfs_enable_read(struct file *file, char __user *user_buf,
				   size_t count, loff_t *ppos)
{
	int ret;
	struct coresight_device *csdev = file->private_data;
	char *buf = kmalloc(PAGE_SIZE, GFP_KERNEL);

	ret = scnprintf(buf, PAGE_SIZE, "%u\n", (unsigned)csdev->enable);
	ret = simple_read_from_buffer(user_buf, count, ppos, buf, ret);

	kfree(buf);
	return ret;
}

static const struct file_operations debugfs_enable_ops = {
	.open = simple_open,
	.read = debugfs_enable_read,
	.write = debugfs_enable_write,
};

static const struct coresight_ops_entry debugfs_enable_entry = {
	.name = "enable",
	.mode =  S_IRUGO | S_IWUSR,
	.ops = &debugfs_enable_ops,
};

static const struct coresight_ops_entry *coresight_grps_sink[] = {
	&debugfs_curr_sink_entry,
	NULL,
};

static const struct coresight_ops_entry *coresight_grps_source[] = {
	&debugfs_enable_entry,
	NULL,
};

struct coresight_group_entries {
	const char *name;
	const struct coresight_ops_entry **entries;
};

struct coresight_group_entries coresight_debugfs_entries[] = {
	{
		.name = "none",
	},
	{
		.name = "sink",
		.entries = coresight_grps_sink,
	},
	{
		.name = "link",
	},
	{
		.name = "linksink",
	},
	{
		.name = "source",
		.entries = coresight_grps_source,
	},
};

static void coresight_device_release(struct device *dev)
{
	struct coresight_device *csdev = to_coresight_device(dev);
	kfree(csdev);
}

static void coresight_fixup_orphan_conns(struct coresight_device *csdev)
{
	struct coresight_connection *conn, *temp;

	list_for_each_entry_safe(conn, temp, &coresight_orph_conns, link) {
		if (conn->child_id == csdev->id) {
			conn->child_dev = csdev;
			list_del(&conn->link);
		}
	}
}

static void coresight_fixup_device_conns(struct coresight_device *csdev)
{
	int i;
	struct coresight_device *cd;
	bool found;

	for (i = 0; i < csdev->nr_conns; i++) {
		found = false;
		list_for_each_entry(cd, &coresight_devs, dev_link) {
			if (csdev->conns[i].child_id == cd->id) {
				csdev->conns[i].child_dev = cd;
				found = true;
				break;
			}
		}
		if (!found)
			list_add_tail(&csdev->conns[i].link,
				      &coresight_orph_conns);
	}
}

static struct dentry *coresight_debugfs_desc_init(
				struct coresight_device *csdev,
				const struct coresight_ops_entry **debugfs_ops)
{
	int i = 0;
	struct dentry *parent;
	struct device *dev = &csdev->dev;
	const struct coresight_ops_entry *ops_entry, **ops_entries;

	parent = debugfs_create_dir(dev_name(dev), debug_parent);
	if (IS_ERR(parent))
		return NULL;

	/* device-specific ops */
	while (debugfs_ops && debugfs_ops[i]) {
		ops_entry = debugfs_ops[i];
		debugfs_create_file(ops_entry->name, ops_entry->mode,
				    parent, dev_get_drvdata(dev->parent),
				    ops_entry->ops);
		i++;
	}

	/* group-specific ops */
	i = 0;
	ops_entries = coresight_debugfs_entries[csdev->type].entries;

	while (ops_entries && ops_entries[i]) {
		debugfs_create_file(ops_entries[i]->name, ops_entries[i]->mode,
				    parent, csdev, ops_entries[i]->ops);
		i++;
	}

	return parent;
}

struct coresight_device *coresight_register(struct coresight_desc *desc)
{
	int i;
	int ret;
	int link_subtype;
	int nr_refcnts;
	int *refcnts = NULL;
	struct coresight_device *csdev;
	struct coresight_connection *conns;

	if (IS_ERR_OR_NULL(desc))
		return ERR_PTR(-EINVAL);

	csdev = kzalloc(sizeof(*csdev), GFP_KERNEL);
	if (!csdev) {
		ret = -ENOMEM;
		goto err_kzalloc_csdev;
	}

	csdev->id = desc->pdata->id;

	if (desc->type == CORESIGHT_DEV_TYPE_LINK ||
	    desc->type == CORESIGHT_DEV_TYPE_LINKSINK) {
		link_subtype = desc->subtype.link_subtype;
		if (link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_MERG)
			nr_refcnts = desc->pdata->nr_inports;
		else if (link_subtype == CORESIGHT_DEV_SUBTYPE_LINK_SPLIT)
			nr_refcnts = desc->pdata->nr_outports;
		else
			nr_refcnts = 1;

		refcnts = kzalloc(sizeof(*refcnts) * nr_refcnts, GFP_KERNEL);
		if (!refcnts) {
			ret = -ENOMEM;
			goto err_kzalloc_refcnts;
		}
		csdev->refcnt.link_refcnts = refcnts;
	}

	csdev->nr_conns = desc->pdata->nr_outports;
	conns = kzalloc(sizeof(*conns) * csdev->nr_conns, GFP_KERNEL);
	if (!conns) {
		ret = -ENOMEM;
		goto err_kzalloc_conns;
	}
	for (i = 0; i < csdev->nr_conns; i++) {
		conns[i].outport = desc->pdata->outports[i];
		conns[i].child_id = desc->pdata->child_ids[i];
		conns[i].child_port = desc->pdata->child_ports[i];
	}
	csdev->conns = conns;

	csdev->type = desc->type;
	csdev->subtype = desc->subtype;
	csdev->ops = desc->ops;
	csdev->owner = desc->owner;

	csdev->dev.parent = desc->dev;
	csdev->dev.release = coresight_device_release;
	dev_set_name(&csdev->dev, "%s", desc->pdata->name);

	down(&coresight_mutex);
	if (desc->pdata->default_sink) {
		if (curr_sink == NO_SINK) {
			curr_sink = csdev->id;
		} else {
			ret = -EINVAL;
			goto err_default_sink;
		}
	}

	coresight_fixup_device_conns(csdev);
	ret = device_register(&csdev->dev);
	if (ret)
		goto err_dev_reg;

	csdev->de = coresight_debugfs_desc_init(csdev, desc->debugfs_ops);
	if (!csdev->de)
		goto err_dev_reg;

	coresight_fixup_orphan_conns(csdev);

	list_add_tail(&csdev->dev_link, &coresight_devs);
	up(&coresight_mutex);

	return csdev;
err_dev_reg:
	put_device(&csdev->dev);
err_default_sink:
	up(&coresight_mutex);
	kfree(conns);
err_kzalloc_conns:
	kfree(refcnts);
err_kzalloc_refcnts:
	kfree(csdev);
err_kzalloc_csdev:
	return ERR_PTR(ret);
}
EXPORT_SYMBOL_GPL(coresight_register);

void coresight_unregister(struct coresight_device *csdev)
{
	if (IS_ERR_OR_NULL(csdev))
		return;

	if (get_device(&csdev->dev)) {
		device_unregister(&csdev->dev);
		put_device(&csdev->dev);
	}

	debugfs_remove_recursive(csdev->de);
}
EXPORT_SYMBOL_GPL(coresight_unregister);

static int debugfs_coresight_init(void)
{
	debug_parent = debugfs_create_dir("coresight", 0);
	if (IS_ERR(debug_parent))
		return -1;

	return 0;
}

static struct of_device_id coresight_match[] = {
	{.compatible = "arm,coresight"},
	{}
};

static int __init coresight_init(void)
{
	if (debugfs_coresight_init())
		pr_info("coresight: problem creating debugfs entry\n");

	return of_platform_bus_probe(NULL, coresight_match, NULL);
}
subsys_initcall(coresight_init);

static void __exit coresight_exit(void) {}
module_exit(coresight_exit);

MODULE_LICENSE("GPL v2");
