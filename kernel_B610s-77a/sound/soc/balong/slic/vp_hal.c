/** \file vp_hal.c
 * vp_hal.c
 *
 * This file contains the platform dependent code for the Hardware Abstraction
 * Layer (HAL). This is example code only to be used by the customer to help
 * clarify HAL requirements.
 *
 * Copyright (c) 2011, Microsemi Corporation
 */
#include <linux/delay.h>
#include "spi.h"
#include "sys_service.h"
#include "snd_param.h"
#include "vp_hal.h"


#define NO_OP              0x06
#define CSLAC_EC_REG_WRT   0x4A   /* Same for all CSLAC devices */

/* longer than 11Tpclk + 6Tfclk*/
#define dtime              0x1


/*****************************************************************************
 * HAL functions for CSLAC devices. Not necessary for VCP
 ****************************************************************************/
/**
 * VpMpiCmd()
 *  This function executes a Device MPI command through the MPI port. It
 * executes both read and write commands. The read or write operation is
 * determined by the "cmd" argument (odd = read, even = write). The caller must
 * ensure that the data array is large enough to hold the data being collected.
 * Because this command used hardware resources, this procedure is not
 * re-entrant.
 *
 * Note: For API-II to support multi-threading, this function has to write to
 * the EC register of the device to set the line being controlled, in addition
 * to the command being passed. The EC register write/read command is the same
 * for every CSLAC device and added to this function. The only exception is
 * if the calling function is accessing the EC register (read), in which case
 * the EC write cannot occur.
 *
 * This example assumes the implementation of two byte level commands:
 *
 *    MpiReadByte(VpDeviceIdType deviceId, uint8 *data);
 *    MpiWriteByte(VpDeviceIdType deviceId, uint8 data);
 *
 * Preconditions:
 *  The device must be initialized.
 *
 * Postconditions:
 *   The data pointed to by dataPtr, using the command "cmd", with length
 * "cmdLen" has been sent to the MPI bus via the chip select associated with
 * deviceId.
 */
 /*lint -save -e737 -e958*/
void
VpMpiCmd(
    VpDeviceIdType deviceId,    /**< Chip select, connector and 3 or 4 wire
                                                     * interface for command */
    uint8 ecVal,        /**< Value to write to the EC register */
    uint8 cmd,          /**< Command number */
    uint8 cmdLen,       /**< Number of bytes used by command (cmd) */
    uint8 *dataPtr)     /**< Pointer to the data location */
{
    int   index = 0;
    uint8 buff[4] = {CSLAC_EC_REG_WRT,ecVal,cmd,NO_OP};

    VpSysEnterCritical(deviceId, VP_MPI_CRITICAL_SEC);

    /*buff[0] = CSLAC_EC_REG_WRT;
    buff[1] = ecVal;
    buff[2] = cmd;
    buff[3] = NO_OP;*/
    for(index = 0;index < 3;index++)
    {
        (void)slic_spi_send(0,SPI_CS_SLIC,&buff[index],1);
        udelay(dtime);
    }

    if (cmd & 0x01)
    {
        (void)slic_spi_send(0,SPI_CS_SLIC,&buff[3],1);
        udelay(dtime);

        for(index = 0;index < cmdLen;index++)
        {
            (void)slic_spi_recv(0, SPI_CS_SLIC, (u8*)&dataPtr[index],1,&buff[3],1);
            udelay(dtime);
        }
    }
    else
    {
        for(index = 0;index < cmdLen;index++)
        {
            (void)slic_spi_send(0,SPI_CS_SLIC,(u8*)&dataPtr[index],1);
            udelay(dtime);
        }
    }
    VpSysExitCritical(deviceId, VP_MPI_CRITICAL_SEC);
    return;
    /*lint -restore*/
} /* End VpMpiCmd */

