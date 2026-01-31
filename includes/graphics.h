/***************************************************************************//**
 * @file
 * @brief Draws the graphics on the display
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEMO_VERSION "Demo v1.0"

/*******************************************************************************
 *****************************   PROTOTYPES   **********************************
 ******************************************************************************/
void GRAPHICS_Init(void);
void GRAPHICS_ShowStatus(bool si7013_status, bool lowBat);
void GRAPHICS_Draw_Weather_Station(int32_t tempData, int32_t rhData,
		bool lowBat, int32_t temp_min_mC, int32_t temp_max_mC,
		int32_t humidity_min, int32_t humidity_max);
void GRAPHICS_DrawMenu(int32_t selectedPage, bool lowBat);
void GRPAHICS_DrawTimeAdj(int32_t pos_h, uint32_t time, int32_t offset,
		bool blink, bool lowBat);
void GRAPHICS_DrawAlarmSet(uint32_t alarmTime, AlarmType type, Day day,
		int8_t sel, bool blink, bool lowBat);

#ifdef __cplusplus
}
#endif

#endif /* __GRAHPHICS_H */
