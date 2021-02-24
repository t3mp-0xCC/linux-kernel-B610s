/* vp_hal.h
 *
 * This file defines the interface between the VoicePath API and the Hardware
 * Abstraction Layer (HAL).  The types and functions declared in this file are
 * platform-dependent.  The functions are defined in ve_hal.c.  Porting the
 * VoicePath API to a new host processor consists of modifying the typedefs
 * in this file, setting the HBI_PINCONFIG define below, and modifying the
 * function implementations in vp_hal.c.
 *
 * Copyright (c) 2011, Microsemi Corporation
 */
#ifndef _VP_HAL_H
#define _VP_HAL_H

#include "vp_api_types.h"

EXTERN void
VpMpiCmd(
    VpDeviceIdType deviceId,    /**< Chip select, connector and 3 or 4 wire
                                 * interface for command
                                 */
    uint8 ecVal,        /**< Value to write to the EC register */
    uint8 cmd,          /**< Command number */
    uint8 cmdLen,       /**< Number of bytes used by command (cmd) */
    uint8 *dataPtr);
#endif /* _VP_HAL_H */

