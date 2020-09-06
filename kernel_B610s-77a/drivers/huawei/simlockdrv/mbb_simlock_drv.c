/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2013-2015. All rights reserved.
 *
 * mobile@huawei.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "bsp_icc.h"

#ifdef BSP_CONFIG_BOARD_SOLUTION

#define WAIT_CCORE_TIMEOUT  (500) /*等待1秒*/
#define SIMLOCK_LEN_MAX (8) /*simlock密码长度限制*/
#define SIMLOCK_ERR (-1)
#define SIMLCOK_OK (0)
unsigned rt_real_result = 0;

enum
{
    
    CCORE_RET_VERIFY_OK = 0,
    CCORE_RET_VERIFY_FAIL = 1,
    CCORE_RET_TIMEOUT = 2,
};

/*A核发simlock到C核后,等待C核的答复*/
struct completion ack_from_ccore = {0};
typedef struct __simlock_req
{
    char simlock_key[16];
}simlock_req;
/******************************************************
  Function:  simlock_verify_req
  Description: A核发起simlock校验请求
  Input:  key-simlock密码长度,8字节
  output:  none
  return: BSP_OK:发送成功 BSP_ERROR:执行错误
  author: g175336
  date: 2012.01.06
******************************************************/
int simlock_verify_req( char* sim_key )
{
    int ret_val = 0;
    unsigned int key_len = 0;
    simlock_req msg = {0};
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_SIMLOCK;
    
    if ( NULL == sim_key )
    {
        printk( KERN_ERR "%s: str is NULL\r\n", __func__ );
        return SIMLOCK_ERR;
    }

    key_len = strlen(sim_key);
    if ( SIMLOCK_LEN_MAX < key_len )
    {
        printk( KERN_ERR "%s: str is too long: %u\r\n", __func__, key_len );
        return SIMLOCK_ERR;
    }

    printk( KERN_DEBUG"\n%s: simlock verify enter, sim_key = %s, key_len = %d.\n\n", __func__, sim_key, key_len);

    /*初始化核间通信传递的消息结构*/

    ( void )strncpy( msg.simlock_key, sim_key ,key_len );
    
    /*发送消息到C核,在C核也要使用同样的结构解析*/

    ret_val = bsp_icc_send(ICC_CPU_MODEM, icc_channel_id, 
                                    (unsigned char*)&msg, sizeof(simlock_req));
    if ( ret_val != (int)(sizeof(simlock_req)) )
    {
        printk( KERN_ERR"%s : send simlock msg fail !!!\n", __func__);
        return SIMLOCK_ERR;
    }
    return SIMLCOK_OK;
}


int simlock_get_verify_ack(unsigned int chanid ,unsigned int len,void* pdata)
{
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_SIMLOCK;
    unsigned int msg = 0;
    int msg_len = 0;
    if ( icc_channel_id != chanid )
    {
        printk(KERN_ERR"%s : simlock: channel_id doesn't match\n", __func__);
    }
    msg_len = bsp_icc_read(icc_channel_id, (u8*)&msg, sizeof(msg));
    if(msg_len != (int)(sizeof(unsigned int)))
    {
        printk(KERN_ERR"%s : simlock: read len(%d) != expected len(%d) !\n", 
            msg_len, sizeof(msg));
        return SIMLOCK_ERR;
    }

    /*首先解析校验结果*/
    rt_real_result = msg;

    printk(KERN_DEBUG"%s: rt_real_result = %d\r\n", __func__, rt_real_result );

    /*给A核信号*/
    complete( &ack_from_ccore );
    
    return SIMLCOK_OK;
}



int simlock_wait_verify_ack( void )
{ 
    unsigned int remain_time = 0;

    /*总共等待1秒*/
    remain_time = wait_for_completion_timeout( &ack_from_ccore, WAIT_CCORE_TIMEOUT );
    if( 0 == remain_time )
    {
        /*超时返回失败*/
        printk(KERN_ERR"%s:time out!!! \r\n ", __func__ ); 
        return CCORE_RET_TIMEOUT;
    }
    else
    {
        printk(KERN_DEBUG"%s: in time, remain_time=%d, ret: %d\r\n ", 
            __func__, remain_time, rt_real_result );

        /*未超时,查看是否校验成功*/
        if ( 0 == rt_real_result ) /*成功*/
        {
            return CCORE_RET_VERIFY_OK ;
        }
        else
        {
            return CCORE_RET_VERIFY_FAIL;
        }
    }
}



int simlock_call_test( char* sim_key )
{
    int ret_send = SIMLOCK_ERR;
    int ret_recv = CCORE_RET_VERIFY_FAIL;

    /*向C核发送查询消息*/
    ret_send = simlock_verify_req( sim_key );

    if(SIMLCOK_OK == ret_send)
    {
        /*等待C核反馈*/
        ret_recv = simlock_wait_verify_ack();
        printk( KERN_DEBUG"simlock_wait_verify_ack ret_recv = %d\r\n", ret_recv );
        
    }
    else
    {
        printk( KERN_ERR "%s: fail, ret_send is %d\r\n", ret_send );
        return SIMLOCK_ERR;
    }

    return ret_recv;
}


/******************************************************
  Function:  simlock_file_verify
  Description: 提供给APP的文件节点
  Input:  标准接口输入
  output:  none
  return: 
  author: g175336
  date: 2012.01.06
******************************************************/
static ssize_t simlock_file_verify(struct file *file,/*lint !e10 !e401*/
                                        const char __user *buf,
                                        size_t count,
                                        loff_t *ppos)
{
    int ret_val = 0;
    char simlock_key[16] = {0};
    printk(KERN_DEBUG"\n%s: simlock_file_verify : enter,  count = %d, buf = %s\n\n", 
            __func__, count, buf);

    /* for pclint */
    if (NULL == file && NULL == ppos)
    {
        ;
    }

    if ( count > SIMLOCK_LEN_MAX )
    {
        printk( KERN_ERR "%s: count is too long: %d\r\n", __func__, count );
        ret_val = -EINVAL;
        goto ret_err;
    }

    ret_val = copy_from_user((void*)simlock_key, buf, count);
    if( 0 != ret_val )
    {
        printk( KERN_ERR "%s: copy fail: %d\r\n", __func__, ret_val );
        goto ret_err;
    }

    /*发送前要先初始化变量*/
    rt_real_result = 0;
    init_completion( &ack_from_ccore );

    ret_val = simlock_call_test(simlock_key);

    printk(KERN_DEBUG"%s: simlock verify : %d\r\n", __func__, ret_val );
    
   
ret_err:
    return ret_val;       
}


/******************************************************
  Function:  simlock_file_open
  Description: 提供给APP的文件节点
  Input:  标准接口输入
  output:  none
  return: 
  author: g175336
  date: 2012.01.06
******************************************************/
static int simlock_file_open(struct inode *inode, struct file *file)/*lint !e10*/
{
    printk( KERN_DEBUG,"%s\r\n", __func__ );

    if (NULL != inode && NULL != file)
    {
        ;
    }
    return 0;
}
/******************************************************
  Function:  simlock_file_release
  Description: 提供给APP的文件节点
  Input:  标准接口输入
  output:  none
  return: 校验正确/错误/超时
  author: g175336
  date: 2012.01.06
******************************************************/
static int simlock_file_release(struct inode *inode, struct file *file)
{
    printk( KERN_DEBUG,"%s\r\n", __func__ );

    if (NULL != inode && NULL != file)
    {
        ;
    }
    return 0;
}

static struct file_operations misc_simlock_file_ops = 
{
    .owner = THIS_MODULE,
    .write = simlock_file_verify,/*lint !e65*/
/*    .unlocked_ioctl = simlock_file_ioctl,*/
    .open = simlock_file_open,
    .release = simlock_file_release,
};

static struct miscdevice simlock_misc = 
{
    .name = "simlock_file",
    .minor = MISC_DYNAMIC_MINOR,
    .fops = &misc_simlock_file_ops,
};


int simlock_multi_acore_init( void )
{
    unsigned int ret_val = 0;
    unsigned int icc_channel_id = ICC_CHN_IFC << 16 | IFC_RECV_FUNC_M2M_SIMLOCK;
    
    printk(KERN_DEBUG"%s: begin...\r\n", __func__ );

    ret_val |= bsp_icc_event_register(icc_channel_id, \
        simlock_get_verify_ack, NULL, NULL, NULL);
    
    if( SIMLCOK_OK != ret_val )
    {
        printk( KERN_ERR "%s: ifc reg fail:%d\r\n", __func__, ret_val );
        goto ret_out;
    }

    ret_val = misc_register(&simlock_misc); /* register device */
    if( ret_val )
    {
        printk( KERN_ERR "%s: misc reg fail:%d\r\n", __func__, ret_val );
        goto ret_out;
    }

    rt_real_result = 0;
    init_completion( &ack_from_ccore );

ret_out:
    return ret_val;
}
late_initcall(simlock_multi_acore_init);
#endif

