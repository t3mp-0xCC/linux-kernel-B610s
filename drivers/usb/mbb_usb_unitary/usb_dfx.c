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

#include "usb_config.h"
#ifdef USB_DFX

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/device.h>
#include <linux/sysfs.h>
#include <linux/timer.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/unaligned.h>
#include <linux/kallsyms.h>
#include <linux/kobject.h>
#include <asm/io.h>
#include "usb_debug.h"

#define USB_DFX_SYSFS_NAME   "usb_dfx"
#define COMMA_SIGN      ','
#define BLANK_SIGN      ' '
#define MAX_NAME_LEN    256
#define MAX_VALUE_LEN   256
#define MAX_ARG_COUNT    7
#define HEX_FLAG        "0x"
#define ERR_FAILED      -1
#define ERR_SUCCESS     0
#define INVALID_VALUE   -1
#define HELP_COMMAND    "help"
#define ADDR_OFFSET(a) (a * 0x01)
#define ARG1     0
#define ARG2     1
#define ARG3     2
#define ARG4     3
#define ARG5     4
#define ARG6     5
#define STR_END    '\0'
#define ONE_PARAM     1
#define TWO_PARAM     2
#define THREE_PARAM   3
#define FOUR_PARAM    4
#define FIVE_PARAM    5
#define SIX_PARAM     6

struct timer_list usb_timer;
typedef unsigned int ( *call_ptr )( int arg1, ... );
typedef unsigned int ( *call_void )( void );

typedef struct param
{
    char param_value[MAX_VALUE_LEN];
} ARG_INFO;

static ARG_INFO arg[MAX_ARG_COUNT];
static int  value[MAX_ARG_COUNT] = {0};
static char timer_func_name[MAX_NAME_LEN] = {0};
static int  timer_param_num = 0;
static int  timer_value[MAX_ARG_COUNT] = {0};


static char* filter_blank( char* string )
{
    int len = 0;
    char* p = NULL;
    if ( NULL == string )
    {
        return NULL;
    }

    while ( *string == BLANK_SIGN )
    {
        string++;
    }
    len = strlen( string );
    if ( len == 0 )
    {
        return NULL;
    }
    p = strchr( string, BLANK_SIGN );
    if ( NULL != p )
    {
        *p = STR_END;
    }
    return string;
}
static void  usb_dfx_help( void )
{
    DBG_T( MBB_DEBUG, "************************ Help Informations **************************\n" );
    DBG_T( MBB_DEBUG, "*                                                                   *\n" );
    DBG_T( MBB_DEBUG, "*                                                                   *\n" );
    DBG_T( MBB_DEBUG, "* Value_set Node is used to set Global variable value. as:          *\n" );
    DBG_T( MBB_DEBUG, "* echo variable_name,value >>/sys/usb_dfx/value_set                 *\n" );
    DBG_T( MBB_DEBUG, "*                                                                   *\n" );
    DBG_T( MBB_DEBUG, "* Func_call Node is used to Realizes the function call. as:         *\n" );
    DBG_T( MBB_DEBUG, "* echo func_name,value,... >>/sys/usb_dfx/func_call                 *\n" );
    DBG_T( MBB_DEBUG, "*                                                                   *\n" );
    DBG_T( MBB_DEBUG, "* Func_timer_call node is used to Realizes the function call every  *\n" );
    DBG_T( MBB_DEBUG, "* time mins. as:                                                    *\n" );
    DBG_T( MBB_DEBUG, "* echo func_name,time,value,...>>/sys/class/usb_dfx/func_timer_call *\n" );
    DBG_T( MBB_DEBUG, "*                                                                   *\n" );
    DBG_T( MBB_DEBUG, "* if you want to delete the func's time that had set, you can do as:*\n" );
    DBG_T( MBB_DEBUG, "* echo func_name,0,value,... >> /sys/class/usb_dfx/func_timer_call  *\n" );
    DBG_T( MBB_DEBUG, "*                                                                   *\n" );
    DBG_T( MBB_DEBUG, "*                                                                   *\n" );
    DBG_T( MBB_DEBUG, "******************************* Author ******************************\n" );

}

static int  transform_string_format( char* string )
{
    int nValude  = 0;
    int ret = 0;
    if ( NULL == string )
    {
        DBG_E( MBB_DEBUG, "pls input the format as: <name,value>\n" );
        /*�����ʽ����*/
        return  ERR_FAILED;
    }

    if ( NULL == strstr( string, HEX_FLAG ) )
    {
        ret = sscanf( string, "%d", &nValude );
    }
    else
    {
        ret = sscanf( string, "%x", &nValude );
    }
	/*�ɹ�д�����ݵ�����1*/
    if (ret > 0)
    {
        return nValude;

    }
    else
    {
        return ERR_FAILED;
    }
}


static int usb_value_set_parse( const char* buf, size_t size )
{
    char name[MAX_NAME_LEN] = {0};
    char param_value[MAX_VALUE_LEN] = {0};
    char* p = NULL;  //����ָ���
    char* filter_name = NULL;  
    int result  = INVALID_VALUE;
    int ip_addr = INVALID_VALUE;
    void __iomem * ip_addr_mem ;
    int i = 0;
    u32 ip_value = 0;
    u32* value_addr = NULL;
    if ( NULL == buf )
    {
        DBG_E( MBB_DEBUG, "pls input the format as: <name,value>\n" );
        return ERR_FAILED;
    }

    memset( name , 0 , sizeof( name ) - 1 );
    memset( param_value , 0 , sizeof( param_value ) - 1 );

    if ( NULL == strchr( buf, COMMA_SIGN ) )
    {
        strncpy( name, buf, size - 1 );
        if ( 0 == strcmp( filter_blank ( name ), HELP_COMMAND ) )
        {
            usb_dfx_help();
            return ERR_SUCCESS;
        }
        DBG_E( MBB_DEBUG, "pls input the format as: <name,value>\n" );
        return ERR_FAILED;
    }
    p = strchr( buf, COMMA_SIGN );
    strncpy( name, buf, p - buf );
    p++;
    strncpy( param_value, p, strlen( p ) );
    filter_name = filter_blank ( param_value );
    if (!filter_name)
    {
        DBG_E( MBB_DEBUG, "null addr filter_name\n" );
        return -EINVAL;
    }
            
    result = transform_string_format( filter_name );
    if ( INVALID_VALUE != result )
    {
        DBG_T( MBB_DEBUG, "usb_value_set_parse --> <%s>: <%d> \n", name , result );
        if ( NULL == strstr( name, HEX_FLAG ) )
        {
            filter_name = filter_blank ( name );
            if (!filter_name)
            {
                DBG_E( MBB_DEBUG, "null addr filter_name\n" );
                return -EINVAL;
            }
            
            value_addr = ( u32* )kallsyms_lookup_name( filter_name ) ;
            if ( NULL != value_addr )
            {
                *value_addr = result;
                DBG_T( MBB_DEBUG, "usb_value_set_parse --> <%s> ip_addr --> <%p> \n", name,
                       value_addr );
            }
            else
            {
                DBG_E( MBB_DEBUG, "NULL ADDR!!\n" );
                return ERR_FAILED;
            }
        }
        else
        {
            ip_addr = transform_string_format( filter_blank ( name ) );
            ip_addr_mem = (void __iomem *)ip_addr;
            if ( INVALID_VALUE != ip_addr_mem )
            {
                for ( i = 0 ; i <= result; i++ )
                {
                    ip_value = readl( ip_addr_mem + ADDR_OFFSET( i ) );
                    DBG_T( MBB_DEBUG, "ip_value_%d --> <0x%08x> \n", i, ip_value );
                }
            }
        }
        return ERR_SUCCESS;
    }
    return ERR_FAILED;
}

/*****************************************************************
Parameters    :  usb_value_set
Description   :  ȫ�ֱ������¸�ֵ
 Input        : buf:  echo ��������
                size: �ַ�����С
 Return Value : �ɹ�����:������Ϣ�ĳ���
                ʧ�ܷ���:ERR_FAILED
  1.Date      : 2014/6/6
 Modification : Created function
*****************************************************************/
static ssize_t usb_value_set( struct device* dev, struct device_attribute* attr,
                                const char* buf, size_t size )
{
    int err = INVALID_VALUE;
    err = usb_value_set_parse( buf, size );
    if ( err < 0 )
    {
        DBG_E( MBB_DEBUG, "usb_value_set_parse err!\n" );
        return -EINVAL;
    }
    return size;
}

/*****************************************************************
Parameters    :  usb_func_call
Return        :
Description   :  ���Ѻ���
*****************************************************************/
static ssize_t usb_func_call( struct device* dev, struct device_attribute* attr,
                                const char* buf, size_t size )
{
    char func_name[MAX_NAME_LEN] = {0};
    int param_num = 1;
    call_ptr func_addr;
    call_void void_func_addr;
    char* p = NULL;  //����ָ���
    char* q = NULL;  //����ո��
    int i = 0;
    char* filter_name = NULL;  
    USB_ULONG func_ret = 0;
    memset( func_name, 0, sizeof( func_name ) );
    memset( arg, 0, sizeof( arg ) );
    memset( value, 0, sizeof( value ) );

    if ( NULL == buf )
    {
        DBG_E( MBB_DEBUG, "pls input the format as: <func_name,value,...> \n" );
        return -EINVAL;
    }

    if ( NULL == strchr( buf, COMMA_SIGN ) )
    {
        strncpy( func_name, buf, size - 1 );
        if ( 0 == strcmp( func_name, HELP_COMMAND ) )
        {
            usb_dfx_help();
            return size;
        }
        filter_name = filter_blank ( func_name );
        if (!filter_name)
        {
            DBG_E( MBB_DEBUG, "null addr filter_name\n" );
            return -EINVAL;
        }
        
        void_func_addr = ( call_void ) kallsyms_lookup_name( filter_name);
        if ( !void_func_addr )
        {
            DBG_E( MBB_DEBUG, "null addr void_func_addr\n" );
            return -EINVAL;
        }

        func_ret = ( USB_ULONG )void_func_addr();
        DBG_E( MBB_DEBUG, "func return 0x%lx\n", func_ret );
        return size;
    }
    p = strchr( buf, COMMA_SIGN );
    if ( NULL == p )
    {
        DBG_E( MBB_DEBUG, "p null\n" );
        return -EINVAL;
    }
    strncpy( func_name, buf, p - buf );
    p++;
    q = p;
    while ( strchr( q, COMMA_SIGN ) )
    {
        param_num++;
        q = strchr( q, COMMA_SIGN );
        if ( NULL == q )
        {
            DBG_E( MBB_DEBUG, "q null\n" );
            return -EINVAL;
        }
        q++;
    }

    //ѭ������
    for ( i = 0 ; i < param_num; i++ )
    {
        q = strchr( p, COMMA_SIGN );
        if ( NULL != q )
        {
            strncpy( arg[i].param_value, p, q - p );
            value[i] = transform_string_format( filter_blank( arg[i].param_value ) );
            p = q + 1;
        }
        else
        {
            strncpy( arg[i].param_value, p, strlen( p ) );
            value[i] = transform_string_format( filter_blank ( arg[i].param_value ) );
        }
        DBG_I( MBB_DEBUG, "param_value is %d\n", value[i] );
    }
    
    filter_name = filter_blank ( func_name );
    if (!filter_name)
    {
        DBG_E( MBB_DEBUG, "null addr filter_name\n" );
        return -EINVAL;
    }
    
    func_addr = ( call_ptr ) kallsyms_lookup_name( filter_name );
    if ( !func_addr )
    {
        DBG_E( MBB_DEBUG, "null addr func_addr\n" );
        return -EINVAL;
    }

    switch ( param_num )
    {
        case ONE_PARAM:
            func_ret = ( USB_ULONG )func_addr( value[ARG1] );
            break;
        case TWO_PARAM:
            func_ret = ( USB_ULONG )func_addr( value[ARG1], value[ARG2] );
            break;
        case THREE_PARAM:
            func_ret = ( USB_ULONG )func_addr( value[ARG1], value[ARG2], value[ARG3] );
            break;
        case FOUR_PARAM:
            func_ret = ( USB_ULONG )func_addr( value[ARG1], value[ARG2], value[ARG3], value[ARG4] );
            break;
        case FIVE_PARAM:
            func_ret = ( USB_ULONG )func_addr( value[ARG1], value[ARG2], value[ARG3], value[ARG4], value[ARG5] );
            break;

    }
    DBG_E( MBB_DEBUG, "func return 0x%lx\n", func_ret );
    return size;
}
/*****************************************************************
Parameters    :  timer_function_call
Return        :
Description   :  ��ʱ���Ѻ�������
*****************************************************************/
void timer_function_call( unsigned long data )
{
    call_ptr func_addr = NULL;
    call_void void_func_addr = NULL;
    int time = 0;
    time = timer_value[ARG1];
    char* name = NULL;
	
    name = filter_blank ( timer_func_name ) ;
	if(NULL != name)
	{
		func_addr = ( call_ptr ) kallsyms_lookup_name(name);
	}
    if ( !func_addr )
    {
        DBG_E( MBB_DEBUG, "null addr func_addr\n" );
        return ;
    }
    switch ( timer_param_num )
    {
        case ONE_PARAM:
            void_func_addr = ( call_void ) kallsyms_lookup_name( filter_blank ( timer_func_name ) );
            void_func_addr();
            break;
        case TWO_PARAM:
            func_addr( timer_value[ARG2] );
            break;
        case THREE_PARAM:
            func_addr( timer_value[ARG2], timer_value[ARG3] );
            break;
        case FOUR_PARAM:
            func_addr( timer_value[ARG2], timer_value[ARG3], timer_value[ARG4] );
            break;
        case FIVE_PARAM:
            func_addr( timer_value[ARG2], timer_value[ARG3], timer_value[ARG4], timer_value[ARG5] );
            break;
        case SIX_PARAM:
            func_addr( timer_value[ARG2], timer_value[ARG3], timer_value[ARG4], timer_value[ARG5], timer_value[ARG6] );
            break;
    }
    mod_timer( &usb_timer, jiffies + ( time * HZ ) );
    return;
}

/*****************************************************************
Parameters    :  usb_timer_call
Return        :
Description   :  ��ʱ���Ѻ�������
*****************************************************************/
static ssize_t usb_timer_call( struct device* dev, struct device_attribute* attr,
                                const char* buf, size_t size )
{
    char* p = NULL;  //����ָ���
    char* q = NULL;  //����ո��
    char* filter_name = NULL;  
    call_void void_func_addr;
    int i = 0;
    int time = 0;
    memset( arg, 0, sizeof( arg ) );
    memset( timer_value, 0, sizeof( timer_value ) );
    memset( timer_func_name, 0, sizeof( timer_func_name ) );
    timer_param_num = 1;

    DBG_I( MBB_DEBUG, "usb_timer_call enter!\n " );

    if ( NULL == buf )
    {
        DBG_E( MBB_DEBUG, "pls input the format as: <func_name,time,value,...> \n" );
        return -EINVAL;
    }

    if ( NULL == strchr( buf, COMMA_SIGN ) )
    {
        strncpy( timer_func_name, buf, size - 1 );
        if ( 0 == strcmp( timer_func_name, HELP_COMMAND ) )
        {
            usb_dfx_help();
            return size;
        }
        filter_name = filter_blank( timer_func_name );
        if (!filter_name)
        {
            DBG_E( MBB_DEBUG, "null addr filter_name\n" );
            return -EINVAL;
        }
        
        void_func_addr = ( call_void ) kallsyms_lookup_name( filter_name );
        if ( !void_func_addr )
        {
            DBG_E( MBB_DEBUG, "null addr func_addr\n" );
            return -EINVAL;
        }
        void_func_addr();
        return size;
    }
    p = strchr( buf, COMMA_SIGN );
    strncpy( timer_func_name, buf, p - buf );
    p++;
    q = p;
    while ( strchr( q, COMMA_SIGN ) )
    {
        timer_param_num++;
        q = strchr( q, COMMA_SIGN );
        q++;
    }
    for ( i = 0 ; i < timer_param_num; i++ )
    {
        q = strchr( p, COMMA_SIGN );
        if ( NULL != q )
        {
            strncpy( arg[i].param_value, p, q - p );
            timer_value[i] = transform_string_format( filter_blank( arg[i].param_value ) );
            p = q + 1;
        }
        else
        {
            strncpy( arg[i].param_value, p, strlen( p ) );
            timer_value[i] = transform_string_format( filter_blank( arg[i].param_value ) );
        }
    }
    time = timer_value[ARG1];

    if ( 0 == time )
    {
        DBG_T( MBB_DEBUG, "del timer  %d\n", time );
        if ( timer_pending( &usb_timer ) )
        {
            DBG_T( MBB_DEBUG, "*********** Timer Delete Successful ***********\n" );
            del_timer( &usb_timer );
        }
        else
        {
            DBG_T( MBB_DEBUG, "<%s> hav't runing before ! can't del !\n", timer_func_name );
        }
        return -EINVAL;
    }
    if ( timer_pending( &usb_timer ) )
    {
        DBG_T( MBB_DEBUG, "del the old timer and creat a new one \n" );
        del_timer( &usb_timer );
    }

    usb_timer.expires = jiffies + ( time * HZ );
    usb_timer.data = 0;
    usb_timer.function = timer_function_call;
    add_timer( &usb_timer );
    DBG_T( MBB_DEBUG, "*********** Begin To Start Timer ***********\n" );
    DBG_T( MBB_DEBUG, "*                                          *\n" );
    DBG_T( MBB_DEBUG, "***************** PLS Wait *****************\n" );

    return size;
}

static DEVICE_ATTR( value_set, S_IWUSR, NULL, usb_value_set );
static DEVICE_ATTR( func_call, S_IWUSR, NULL, usb_func_call );
static DEVICE_ATTR( func_timer_call, S_IWUSR, NULL, usb_timer_call );

static struct attribute* dev_attrs[] =
{
    &dev_attr_value_set.attr,
    &dev_attr_func_call.attr,
    &dev_attr_func_timer_call.attr,
    NULL,
};


static struct attribute_group dev_attr_grp =
{
    .attrs = dev_attrs,
};

/*****************************************************************
Parameters    :  usb_dfx_init
Return        :
Description   :  usb_dfx��ʼ��
*****************************************************************/
static int __init usb_dfx_init( void )
{
    int res = INVALID_VALUE;
    struct kobject* dev_kobj = NULL;
    struct kobject* usb_kobj = NULL;
    memset( arg , 0 , sizeof( arg ) - 1 );
    dev_kobj = kobject_create_and_add( USB_DFX_SYSFS_NAME, usb_kobj );
    res = sysfs_create_group( dev_kobj, &dev_attr_grp );
    if ( res )
    {
        DBG_E( MBB_DEBUG, "sysfs_create_group err\n" );
        return res;
    }
    init_timer( &usb_timer );
    return ERR_SUCCESS;
}

static void __exit usb_dfx_exit( void )
{
    del_timer( &usb_timer );
    DBG_I( MBB_DEBUG, "----->usb_dfx_exit\n" );
}

module_init( usb_dfx_init );
module_exit( usb_dfx_exit );
#endif /* USB_DFX*/
