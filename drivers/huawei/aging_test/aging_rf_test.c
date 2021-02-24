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


#include "aging_rf_test.h"

static struct aging_rf_test_para rf_test_para[] =
{
    {0,0,0},     /*band0*/
    {6,0,9},     /*band1*/
    {6,1,9},     /*band2*/
    {6,2,9},     /*band3*/
    {6,3,9},     /*band4*/
    {6,7,9},     /*band5*/
    {6,21,9},   /*band6*/
    {6,10,9},     /*band7*/
    {6,6,9},     /*band8*/
    {6,13,9},     /*band9*/
    {6,14,9},     /*band10*/
    {6,5,9},     /*band11*/
    {6,16,9},     /*band12*/
    {5,17,9},     /*band13*/
    {6,19,9},     /*band14*/
    {6,0,9},     /*band15*/
    {6,0,9},     /*band16*/
    {6,18,9},     /*band17*/
    {6,8,9},     /*band18*/
    {6,22,9},     /*band19*/
    {6,20,9},     /*band20*/
    {6,23,9},     /*band21*/
    {6,0,9},     /*band22*/
    {6,0,9},     /*band23*/
    {6,4,9},     /*band24*/
    {6,31,9},     /*band25*/
    {0,0,0},     /*band26*/
    {0,0,0},     /*band27*/
    {0,0,0},     /*band28*/
    {0,0,0},     /*band29*/
    {0,0,0},     /*band30*/
    {0,0,0},     /*band31*/
    {0,0,0},     /*band32*/
    {7,25,12},     /*band33*/
    {2,24,6},     /*band34,TDS*/
    {7,12,12},     /*band35*/
    {7,9,12},     /*band36*/
    {7,26,12},     /*band37*/
    {7,15,12},     /*band38*/
    {2,27,6},     /*band39,TDS*/
    {7,11,12},     /*band40*/
    {7,28,12},     /*band41*/
    {7,29,12},     /*band42*/
    {7,30,12}, /*band43*/
    {7,30,12}      /*band44,����*/
};

/*RF���AT��������*/
static unsigned char g_atcmd_fchan[AGING_ATCMD_LEN]  = {0};
static unsigned char g_atcmd_tselrf[AGING_ATCMD_LEN] = {0};
static aging_atcmd_info atcmd_info[] =
{
    {AG_ATCMD_CFUN0, "AT+CFUN=0\r\n"},
    {AG_ATCMD_TMOD1, "AT^TMODE=1\r\n"},
    {AG_ATCMD_FCHAN, g_atcmd_fchan},
    {AG_ATCMD_FTXON0, "AT^FTXON=0\r\n"},
    {AG_ATCMD_TSELRF, g_atcmd_tselrf},
    {AG_ATCMD_FWAVE, "AT^FWAVE=0,2300\r\n"},
    {AG_ATCMD_FTXON1, "AT^FTXON=1\r\n"},
};

/*�������нṹ����*/
static aging_work_struct aging_work = {0};
static unsigned int rf_init_flag = 0;  /*rf���Գ�ʼ����־*/
static unsigned int rf_exit_flag = 0;  /*���Խ�����־*/
static struct workqueue_struct * aging_rf_test_workqueue = NULL;
static DEFINE_MUTEX(aging_rf_mutex); /* rf������Դ������ */


static int aging_rf_index_select(aging_work_struct *rf_work)
{
    aging_rf_work_data *rf_work_data = NULL;
    unsigned short b_num = 0;
    unsigned short off_index = 0;  /*band���ƫ��*/
    bool b_select = false;

    if(NULL == rf_work)
    {
        aging_print_error("Input para error.\n");
        return -1;
    }
    rf_work_data = (aging_rf_work_data *)(rf_work->work_data);
    b_num = rf_work->priv->aging_test_nv.rf_test.band_num;
    if((rf_work_data->work_index >= b_num) || (rf_work_data->work_index >= RF_BAND_MAX_NUM))
    {
        aging_print_error("band index overrun, index = %d, band num = %d.\n",
                            rf_work_data->work_index, b_num);
        return -1;
    }
    else if(AG_WORK_START == rf_work->work_state)
    {
        /*��ʼ����index��0*/
        rf_work_data->work_index = 0;
    }
    else
    {
        /*����index*/
        if(b_num == (rf_work_data->work_index + 1))
        {
            rf_work_data->work_index = 0; /*circleһ�����´�0��ʼ*/
            (rf_work_data->circle_flag)++;
            
        }
        else
        {
            (rf_work_data->work_index)++;
        }
        /*1��circle�����Ҵ���lte band,����������lte*/
        if((0 < rf_work_data->circle_flag) && (0 < rf_work_data->lte_band_num))
        {
            off_index = rf_work_data->work_index;
            do
            {
                /*�ҵ�˳���һ��lte���Ե�*/
                if(AG_RF_LTE == rf_work->priv->aging_test_nv.rf_test.rf_parametr[off_index].mode)
                {
                    b_select = true;
                    break;
                }
                else if(b_num == (off_index + 1))
                {
                    off_index = 0;
                    (rf_work_data->circle_flag)++;
                }
                else
                {
                    off_index++;
                }
                
            }while(off_index != rf_work_data->work_index); /*ѭ������1������*/
            if(true != b_select)
            {
                /*��lte��δ�ҵ�ֱ�ӷ��ش���*/
                return -1;
            }
            rf_work_data->work_index = off_index;
        }
    }

    return 0;    
}

/******************************************************************
*������   : aging_rf_test_open
*�������� : ����ĳ����ŵ�rf �ϻ�����
*������� : aging_test_priv *priv -- �ϻ������豸�ṹָ��
                 int index -- nv�����õĲ��������е����
*������� : ��
*����ֵ   : ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ
*�޸���ʷ :
******************************************************************/
static int aging_rf_test_open(struct aging_test_priv *priv, unsigned short index)
{
    int ret = -1;
    int i = 0;
    unsigned int atlist_num = 0;
    unsigned int bandlist_num = 0;
    unsigned short band = 0;
    unsigned short freq = 0;

    if((NULL == priv) || (index >= priv->aging_test_nv.rf_test.band_num))
    {
        aging_print_error("Input para error, index = %d.\n", index);
        return -1;
    }

    atlist_num = (unsigned int)(sizeof(atcmd_info) / sizeof(atcmd_info[0]));
    bandlist_num = (unsigned int)(sizeof(rf_test_para) / sizeof(rf_test_para[0]));
    band = priv->aging_test_nv.rf_test.rf_parametr[index].band;
    freq = priv->aging_test_nv.rf_test.rf_parametr[index].frequency;
    if((band >= bandlist_num) || (AG_ATCMD_MAX != atlist_num))
    {
        aging_print_error("band or cmdlen error, band = %d, bandnum = %d, cmdnum = %d.\n",
                            band, bandlist_num, atlist_num);
        return -1;
    }

    /*��䷢��Ƶ����Ϣ�����AT����*/
    (void)memset((void *)(atcmd_info[AG_ATCMD_FCHAN].atname), 0, AGING_ATCMD_LEN);
    (void)memset((void *)(atcmd_info[AG_ATCMD_TSELRF].atname), 0, AGING_ATCMD_LEN);
    (void)snprintf(atcmd_info[AG_ATCMD_FCHAN].atname, AGING_ATCMD_LEN,
        "AT^FCHAN=%d,%d,%d\r\n", rf_test_para[band].mode, rf_test_para[band].band_switch, freq);
    (void)snprintf(atcmd_info[AG_ATCMD_TSELRF].atname, AGING_ATCMD_LEN,
        "AT^TSELRF=%d\r\n", rf_test_para[band].path);

    /*˳��ִ��AT���ʼ�ϻ�����*/
    for(i = 0; i < atlist_num; i++)
    {
        if((atcmd_info[i].atcmd != (unsigned int)i)
            || (NULL == atcmd_info[i].atname))
        {
            aging_print_error("atcmd not in order or atname not exist, %d.\n", i);
            return -1;
        }
        ret = At_CmdStreamPreProc(0, atcmd_info[i].atname,
                    strlen(atcmd_info[i].atname));
        if(0 != ret)
        {
            aging_print_error("at index %d send failed.\n", i);
            return -1;
        }
        msleep(AGING_ATCMD_DELAY_TIME);
    }

    return 0;
}

/******************************************************************
*������   : aging_rf_test_close
*�������� : �رյ�ǰband��rf �ϻ�����
*������� : ��
*������� : ��
*����ֵ   : ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ
*�޸���ʷ :
******************************************************************/
static int aging_rf_test_close(void)
{
    int ret = -1;

    /*����AT����ر���Ƶ����*/
    if((AG_ATCMD_FTXON0 == atcmd_info[AG_ATCMD_FTXON0].atcmd)
        && (NULL != atcmd_info[AG_ATCMD_FTXON0].atname))
    {
        ret = At_CmdStreamPreProc(0, atcmd_info[AG_ATCMD_FTXON0].atname,
                    strlen(atcmd_info[AG_ATCMD_FTXON0].atname));
        if(0 != ret)
        {
            aging_print_error("close aging rf test failed.\n");
            ret = -1;
        }
    }

    return ret;
}

/********************************************************
*������   : aging_rf_test_workfun
*�������� : rf �ϻ�����work����
*������� : void *p����:struct platform_device *pdev
*������� : ��
*����ֵ   : ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ
*�޸���ʷ :
********************************************************/
static void aging_rf_test_workfun(struct work_struct *work)
{
    int ret = -1;
    aging_work_struct *pa_work = NULL;
    unsigned long ldelay_time = 0;
    aging_rf_work_data *rf_work_data = NULL;
    if(NULL == work)
    {
        aging_print_error("Input para error.\n");
        return;
    }

    pa_work = to_aging_work(work);
    if((NULL == pa_work) || (NULL == pa_work->priv)
        || (NULL == pa_work->work_data))
    {
        aging_print_error("aging work struct get failed.\n");
        return;
    }
    rf_work_data = (aging_rf_work_data *)(pa_work->work_data);

    mutex_lock(&aging_rf_mutex);
    if(0 != rf_exit_flag)
    {
        /*���˳�,������״̬*/
        aging_print_error("rf aging work has exited, %d.\n", rf_exit_flag);
        mutex_unlock(&aging_rf_mutex);
        return;
    }
    /*����״̬ѡ����*/
    switch(pa_work->work_state)
    {
        case AG_WORK_START:
        case AG_WORK_RUN:
        {
            /*��ʼ����*/
            /*ѡ��ǰ��Ҫ���Ե�band index*/
            ret = aging_rf_index_select(pa_work);
            if(0 != ret)
            {
                aging_print_error("aging index select error.\n");
                goto WORK_FAIL;
            }
            ret = aging_rf_test_open(pa_work->priv, rf_work_data->work_index);
            if(0 != ret)
            {
                aging_print_error("aging test open error, index = %d.\n",
                                    rf_work_data->work_index);
                goto WORK_FAIL;
            }
            /*�������������ö�ʱ,�ȴ�ʱ��Ϊ����ʱ��*/
            ldelay_time = (unsigned long)(pa_work->priv->aging_test_nv.rf_test.on_time * AGING_MIN_TO_MSEC);
            pa_work->work_state = AG_WORK_REST; /*��һ���׶���Ϣ*/
            /*���¼���work*/
            (void)queue_delayed_work(aging_rf_test_workqueue,
                &(pa_work->d_work), msecs_to_jiffies(ldelay_time));
            break; 
        }
        case AG_WORK_REST:
        {
            /*��ʼ��Ϣ*/
            ret = aging_rf_test_close();
            if(0 != ret)
            {
                aging_print_error("aging test close error, index = %d.\n",
                                    rf_work_data->work_index);
                goto WORK_FAIL;
            }
            /*�رպ��������ö�ʱ,�ȴ�ʱ��Ϊ��Ϣʱ��*/
            ldelay_time = (unsigned long)(pa_work->priv->aging_test_nv.rf_test.off_time * AGING_MIN_TO_MSEC);
            pa_work->work_state = AG_WORK_RUN;  /*��һ���׶ι���*/
            /*��Ϣ�׶β�����index,���¼���work*/
            (void)queue_delayed_work(aging_rf_test_workqueue,
                &(pa_work->d_work), msecs_to_jiffies(ldelay_time));
            break;
        }
        default:
        {
            /*��������������*/
            aging_print_error("default case = %d, do nothing.\n", pa_work->work_state);
            break;
        }
    }
    mutex_unlock(&aging_rf_mutex);

    return;

WORK_FAIL:
    /*ʧ������fail״̬���˳�*/
    mutex_unlock(&aging_rf_mutex);
    AGING_STATUS_SET_FAIL(RF_TEST_MODULE, pa_work->priv->aging_total_status);
    return;
}

/********************************************************
*������   : aging_rf_test_thread
*�������� : rf �ϻ������߳�
*������� : void *p����:struct platform_device *pdev
*������� : ��
*����ֵ   : ִ�гɹ�����0��ʧ�ܷ��ط�0ֵ
*�޸���ʷ :
*           2015-7-21 �쳬 ��������
********************************************************/
int aging_rf_test_thread(void *p)
{
    struct aging_test_priv *priv = NULL;
    aging_rf_work_data *rf_work_data = NULL;

    if((NULL == p) || (0 == rf_init_flag))
    {
        aging_print_error("p is NULL or rf test not init, %d!!\n", rf_init_flag);
        return 0;
    }

    priv = platform_get_drvdata((struct platform_device *)p);
    rf_work_data = (aging_rf_work_data *)(aging_work.work_data);
    if((NULL == priv) || (NULL == rf_work_data))
    {
        aging_print_error("priv or rf data is NULL!!\n");
        return -1;
    }
    
    aging_print_error("rf aging test start.\n");
    /*������Դ��Ҫ����*/
    mutex_lock(&aging_rf_mutex);
    if(0 != rf_exit_flag)
    {
        /*�����˳�ֱ�ӷ���*/
        aging_print_error("rf aging work has exited, %d.\n", rf_exit_flag);
        mutex_unlock(&aging_rf_mutex);
        return 0;
    }
    rf_work_data->work_index = 0;
    aging_work.work_state = AG_WORK_START;
    /*�״�������ʼ�����������*/
    (void)queue_delayed_work(aging_rf_test_workqueue,
                &(aging_work.d_work), (unsigned long)AG_WORK_START);
    mutex_unlock(&aging_rf_mutex);

    return 0;
}


int aging_rf_test_init(void *p)
{
    int i = 0;
    struct aging_test_priv *priv = NULL;
    aging_rf_work_data *rf_work_data = NULL;
    if(NULL == p)
    {
        aging_print_error("p is NULL !!\n");
        return -EINVAL;
    }

    rf_init_flag = 0;
    priv = platform_get_drvdata((struct platform_device *)p);
    if(NULL == priv)
    {
        aging_print_error("priv is NULL !!\n");
        return -EINVAL;
    }
    
    if(AGING_ENABLE != priv->aging_test_nv.rf_test.rf_test_enable)
    {
        aging_print_error("rf test disabled in nv!!\n");
        return 0;
    }

    /*��ʼ��work����*/
    rf_work_data = (aging_rf_work_data *)kmalloc(sizeof(aging_rf_work_data), GFP_KERNEL);
    if(NULL == rf_work_data)
    {
        aging_print_error("malloc rf data failed!!\n");
        return -1;
    }
    (void)memset(rf_work_data, 0, sizeof(aging_rf_work_data));
    /*��������lte band����*/
    for(i = 0; i < priv->aging_test_nv.rf_test.band_num; i++)
    {
        if(AG_RF_LTE == priv->aging_test_nv.rf_test.rf_parametr[i].mode)
        {
            (rf_work_data->lte_band_num)++;
        }
    }

    aging_rf_test_workqueue = create_workqueue(AGING_WORK_NAME);
    if(NULL == aging_rf_test_workqueue)
    {
        aging_print_error("create workqueue failed.\n");
        kfree(rf_work_data);
        rf_work_data = NULL;
        return -1;
    }
    aging_work.work_data = (void *)rf_work_data;
    aging_work.priv = priv;
    INIT_DELAYED_WORK(&(aging_work.d_work), aging_rf_test_workfun);

    rf_init_flag = 1;
    aging_print_error("rf aging test init.\n");

    return 0;
}


int aging_rf_test_exit(void *p)
{
    int ret = -1;

    if((NULL == p) || (0 == rf_init_flag))
    {
        /*�˴��������쳣*/
        return 0;
    }

    mutex_lock(&aging_rf_mutex);
    /*׼�����������ñ�־*/
    rf_exit_flag = 1;
    (void)cancel_delayed_work(&(aging_work.d_work));
    flush_workqueue(aging_rf_test_workqueue);
    aging_work.work_state = AG_WORK_END;
    destroy_workqueue(aging_rf_test_workqueue);

    /*�ͷ�������ڴ�*/
    if(NULL != aging_work.work_data)
    {
        kfree(aging_work.work_data);
        aging_work.work_data = NULL;
    }
        
    /*�ر���Ƶ����*/
    ret = aging_rf_test_close();
    if(0 != ret)
    {
        aging_print_error("aging test close error.\n");
    }
    mutex_unlock(&aging_rf_mutex);

    aging_print_error("rf aging test exit.\n");
    return 0;
}

