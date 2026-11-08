// SPDX-License-Identifier: GPL-3.0-or-later
// AmpliFi Style Speed Meter Screen for XiGuaPi V3

#ifndef _UI_SPEEDMETER_H
#define _UI_SPEEDMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"

extern lv_obj_t * ui_SpeedMeter;
extern lv_obj_t * ui_barSpeedDownload;
extern lv_obj_t * ui_barSpeedUpload;
extern lv_obj_t * ui_valSpeedDownload;
extern lv_obj_t * ui_valSpeedUpload;
extern lv_obj_t * ui_lblSpeedUnit;
extern lv_obj_t * ui_lblSpeedIface;
extern lv_obj_t * ui_lblSpeedConns;
extern lv_obj_t * ui_valTotalDl;
extern lv_obj_t * ui_valTotalUl;
extern lv_obj_t * ui_valSpeedClients;
extern lv_obj_t * ui_valSpeedPeak;

void ui_SpeedMeter_screen_init(void);
void ui_SpeedMeter_screen_destroy(void);
void ui_event_SpeedMeter(lv_event_t * e);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif

