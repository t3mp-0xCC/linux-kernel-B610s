#ifndef _IOS_AO_DRV_MACRO_H_
#define _IOS_AO_DRV_MACRO_H_ 
#include "bsp_memmap.h"
#define IOS_AO_BASE_ADDR HI_IOS_AO_REGBASE_ADDR
#define IOS_AO_MF_OFFSET_ADDR 0xC00
#define IOS_AO_AF_OFFSET_ADDR 0xD00
#define IOS_AO_IOM_OFFSET_ADDR 0xE00
#define IOS_AO_MF_ADDR (IOS_AO_BASE_ADDR+IOS_AO_MF_OFFSET_ADDR)
#define IOS_AO_AF_ADDR (IOS_AO_BASE_ADDR+IOS_AO_AF_OFFSET_ADDR)
#define IOS_AO_IOM_ADDR (IOS_AO_BASE_ADDR+IOS_AO_IOM_OFFSET_ADDR)
#define IOS_SR_BITPOS 0
#define IOS_SR_BITWIDTH 1
#define IOS_RET_SR_BITMASK (((1<<IOS_SR_BITWIDTH)-1)<<IOS_RET_SR_BITPOS)
#define IOS_DS_BITPOS 1
#define IOS_DS_BITWIDTH 3
#define IOS_DS_BITMASK (((1<<IOS_DS_BITWIDTH)-1)<<IOS_DS_BITPOS)
#define IOS_PS_BITPOS 4
#define IOS_PS_BITWIDTH 2
#define IOS_PS_BITMASK (((1<<IOS_PS_BITWIDTH)-1)<<IOS_PS_BITPOS)
#define IOS_OE_BITPOS 6
#define IOS_OE_BITWIDTH 1
#define IOS_OE_BITMASK (((1<<IOS_OE_BITWIDTH)-1)<<IOS_OE_BITPOS)
#define IOS_EG_BITPOS 7
#define IOS_EG_BITWIDTH 1
#define IOS_EG_BITMASK (((1<<IOS_EG_BITWIDTH)-1)<<IOS_EG_BITPOS)
#define IOS_NA 0x0
#define IOS_PU 0x12
#define IOS_PD 0x22
#define IOS_SUS 0x32
#define IOS_INPUT 0x02
#define IOS_OUTPUT 0x42
#define IOS_2MA 0x0
#define IOS_4MA 0x1
#define IOS_6MA 0x2
#define IOS_8MA 0x3
#define IOS_10MA 0x4
#define IOS_12MA 0x5
#define IOS_14MA 0x6
#define IOS_16MA 0x7
#define IOS_EG 0x80
#define IOS_DG 0x00
#define IOS_AO_IOM_CTRL0 (IOS_AO_IOM_ADDR+0x000)
#define IOS_AO_IOM_CTRL1 (IOS_AO_IOM_ADDR+0x004)
#define IOS_AO_IOM_CTRL10 (IOS_AO_IOM_ADDR+0x028)
#define IOS_AO_IOM_CTRL11 (IOS_AO_IOM_ADDR+0x02C)
#define IOS_AO_IOM_CTRL12 (IOS_AO_IOM_ADDR+0x030)
#define IOS_AO_IOM_CTRL2 (IOS_AO_IOM_ADDR+0x008)
#define IOS_AO_IOM_CTRL3 (IOS_AO_IOM_ADDR+0x00C)
#define IOS_AO_IOM_CTRL4 (IOS_AO_IOM_ADDR+0x010)
#define IOS_AO_IOM_CTRL5 (IOS_AO_IOM_ADDR+0x014)
#define IOS_AO_IOM_CTRL6 (IOS_AO_IOM_ADDR+0x018)
#define IOS_AO_IOM_CTRL7 (IOS_AO_IOM_ADDR+0x01C)
#define IOS_AO_IOM_CTRL8 (IOS_AO_IOM_ADDR+0x020)
#define IOS_AO_IOM_CTRL9 (IOS_AO_IOM_ADDR+0x024)
#define IOS_AO_AF_CTRL0 (IOS_AO_AF_ADDR+0x000)
#define IOS_AO_MF_CTRL0 (IOS_AO_MF_ADDR+0x000)
#define OUTSET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL0 SETBITVALUE32(IOS_AO_IOM_CTRL0,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL1 SETBITVALUE32(IOS_AO_IOM_CTRL1,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL10 SETBITVALUE32(IOS_AO_IOM_CTRL10,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL11 SETBITVALUE32(IOS_AO_IOM_CTRL11,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL12 SETBITVALUE32(IOS_AO_IOM_CTRL12,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL2 SETBITVALUE32(IOS_AO_IOM_CTRL2,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL3 SETBITVALUE32(IOS_AO_IOM_CTRL3,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL4 SETBITVALUE32(IOS_AO_IOM_CTRL4,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL5 SETBITVALUE32(IOS_AO_IOM_CTRL5,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL6 SETBITVALUE32(IOS_AO_IOM_CTRL6,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL7 SETBITVALUE32(IOS_AO_IOM_CTRL7,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL8 SETBITVALUE32(IOS_AO_IOM_CTRL8,IOS_EG_BITMASK,IOS_DG )
#define OUTSET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_OE_BITMASK,IOS_OUTPUT)
#define INSET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_OE_BITMASK,IOS_INPUT )
#define PDSET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_PS_BITMASK,IOS_PD )
#define PUSET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_PS_BITMASK,IOS_PU )
#define SUSSET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_PS_BITMASK,IOS_SUS )
#define NASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_PS_BITMASK,IOS_NA )
#define I2MASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_DS_BITMASK,IOS_2MA )
#define I4MASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_DS_BITMASK,IOS_4MA )
#define I6MASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_DS_BITMASK,IOS_6MA )
#define I8MASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_DS_BITMASK,IOS_8MA )
#define I10MASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_DS_BITMASK,IOS_10MA )
#define I12MASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_DS_BITMASK,IOS_12MA )
#define I14MASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_DS_BITMASK,IOS_14MA )
#define I16MASET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_DS_BITMASK,IOS_16MA )
#define EGSET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_EG_BITMASK,IOS_EG )
#define DGSET_IOS_AO_IOM_CTRL9 SETBITVALUE32(IOS_AO_IOM_CTRL9,IOS_EG_BITMASK,IOS_DG )
#define SET_IOS_PCIE0_WAKE_CTRL1_1 SETREG32(IOS_AO_AF_CTRL0,1<<0)
#define CLR_IOS_PCIE0_WAKE_CTRL1_1 CLRREG32(IOS_AO_AF_CTRL0,1<<0)
#define SET_IOS_PCIE1_WAKE_CTRL1_1 SETREG32(IOS_AO_AF_CTRL0,1<<1)
#define CLR_IOS_PCIE1_WAKE_CTRL1_1 CLRREG32(IOS_AO_AF_CTRL0,1<<1)
#define SET_IOS_GPIO0_0_CTRL1_1 SETREG32(IOS_AO_MF_CTRL0,1<<0)
#define CLR_IOS_GPIO0_0_CTRL1_1 CLRREG32(IOS_AO_MF_CTRL0,1<<0)
#define SET_IOS_GPIO0_1_CTRL1_1 SETREG32(IOS_AO_MF_CTRL0,1<<1)
#define CLR_IOS_GPIO0_1_CTRL1_1 CLRREG32(IOS_AO_MF_CTRL0,1<<1)
#endif
