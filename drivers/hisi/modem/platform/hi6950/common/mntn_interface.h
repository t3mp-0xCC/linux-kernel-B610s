#ifndef __MNTN_INTERFACE_H__
#define __MNTN_INTERFACE_H__ 
#define MNTN_BASE_ADDR (DDR_MNTN_ADDR)
#define MNTN_BASE_SIZE (DDR_MNTN_SIZE)
#define MNTN_BASEINFO_ADDR (MNTN_BASE_ADDR)
#define MNTN_BASEINFO_SIZE (0x200)
#define MNTN_AREA_AP_ADDR (MNTN_BASEINFO_ADDR+MNTN_BASEINFO_SIZE)
#define MNTN_AREA_AP_SIZE (0x30000)
#define MNTN_AREA_CP_ADDR (MNTN_AREA_AP_ADDR+MNTN_AREA_AP_SIZE)
#define MNTN_AREA_CP_SIZE (0x70000)
#define MNTN_AREA_MDMAP_ADDR (MNTN_AREA_CP_ADDR+MNTN_AREA_CP_SIZE)
#define MNTN_AREA_MDMAP_SIZE (0x28000)
#define MNTN_AREA_LPM3_ADDR (MNTN_AREA_MDMAP_ADDR+MNTN_AREA_MDMAP_SIZE)
#define MNTN_AREA_LPM3_SIZE (0x8000)
#define MNTN_AREA_TEEOS_ADDR (MNTN_AREA_LPM3_ADDR+MNTN_AREA_LPM3_SIZE)
#define MNTN_AREA_TEEOS_SIZE (0x20000)
#define MNTN_AREA_RESERVE_ADDR (MNTN_AREA_TEEOS_ADDR+MNTN_AREA_TEEOS_SIZE)
#define MNTN_AREA_RESERVE_SIZE (MNTN_BASE_ADDR+MNTN_BASE_SIZE-MNTN_AREA_RESERVE_ADDR)
#endif
