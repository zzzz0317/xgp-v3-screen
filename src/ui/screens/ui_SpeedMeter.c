// SPDX-License-Identifier: GPL-3.0-or-later
// AmpliFi Style Speed Meter Screen for XiGuaPi V3

#include "../ui.h"
#include <stdlib.h>
#include <stdio.h>

lv_obj_t * ui_SpeedMeter = NULL;
lv_obj_t * ui_barSpeedDownload = NULL;
lv_obj_t * ui_barSpeedUpload = NULL;
lv_obj_t * ui_valSpeedDownload = NULL;
lv_obj_t * ui_valSpeedUpload = NULL;
lv_obj_t * ui_lblSpeedUnit = NULL;
lv_obj_t * ui_lblSpeedIface = NULL;
lv_obj_t * ui_lblSpeedConns = NULL;
lv_obj_t * ui_valTotalDl = NULL;
lv_obj_t * ui_valTotalUl = NULL;
lv_obj_t * ui_valSpeedClients = NULL;
lv_obj_t * ui_valSpeedPeak = NULL;

static lv_obj_t * ui_txtSpeedDlHeader = NULL;
static lv_obj_t * ui_txtSpeedUlHeader = NULL;
static lv_obj_t * ui_txtSpeedDlArrow = NULL;
static lv_obj_t * ui_txtSpeedUlArrow = NULL;

void ui_event_SpeedMeter(lv_event_t * e)
{
    lv_event_code_t event_code = lv_event_get_code(e);

    if(event_code == LV_EVENT_SCREEN_LOADED) {
        int standby_sec = 180; // 3 menit (180 detik)
        const char *env_sec = getenv("XGP_SPEED_STANDBY_TIME");
        if(!env_sec) env_sec = getenv("XGP_SPEED_BOOT_TIME");
        if(env_sec) {
            int val = atoi(env_sec);
            if(val > 0) standby_sec = val;
        }
        _ui_screen_change(&ui_SystemInfo, LV_SCR_LOAD_ANIM_MOVE_TOP, 300, standby_sec * 1000, &ui_SystemInfo_screen_init);
    }
}

void ui_SpeedMeter_screen_init(void)
{
    ui_SpeedMeter = lv_obj_create(NULL);
    lv_obj_remove_flag(ui_SpeedMeter, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(ui_SpeedMeter, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_SpeedMeter, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* --- TOP DOWNLOAD SECTION --- */
    ui_txtSpeedDlHeader = lv_label_create(ui_SpeedMeter);
    lv_obj_set_size(ui_txtSpeedDlHeader, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_txtSpeedDlHeader, LV_ALIGN_TOP_LEFT);
    lv_obj_set_x(ui_txtSpeedDlHeader, 16);
    lv_obj_set_y(ui_txtSpeedDlHeader, 4);
    lv_label_set_text(ui_txtSpeedDlHeader, "DOWNLOAD");
    lv_obj_set_style_text_color(ui_txtSpeedDlHeader, lv_color_hex(0x00D2FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_txtSpeedDlHeader, &ui_font_MiSans16, LV_PART_MAIN);

    ui_lblSpeedIface = lv_label_create(ui_SpeedMeter);
    lv_obj_set_size(ui_lblSpeedIface, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_lblSpeedIface, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_x(ui_lblSpeedIface, -16);
    lv_obj_set_y(ui_lblSpeedIface, 4);
    lv_label_set_text(ui_lblSpeedIface, "br-lan");
    lv_obj_set_style_text_color(ui_lblSpeedIface, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_lblSpeedIface, &ui_font_MiSans16, LV_PART_MAIN);

    /* Top Cyan Bar */
    ui_barSpeedDownload = lv_bar_create(ui_SpeedMeter);
    lv_bar_set_range(ui_barSpeedDownload, 0, 100);
    lv_bar_set_value(ui_barSpeedDownload, 0, LV_ANIM_OFF);
    lv_obj_set_size(ui_barSpeedDownload, 288, 8);
    lv_obj_set_align(ui_barSpeedDownload, LV_ALIGN_CENTER);
    lv_obj_set_x(ui_barSpeedDownload, 0);
    lv_obj_set_y(ui_barSpeedDownload, -64);
    lv_obj_set_style_bg_color(ui_barSpeedDownload, lv_color_hex(0x0A2332), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_barSpeedDownload, 255, LV_PART_MAIN);
    lv_obj_set_style_radius(ui_barSpeedDownload, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_barSpeedDownload, lv_color_hex(0x00D2FF), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(ui_barSpeedDownload, 255, LV_PART_INDICATOR);
    lv_obj_set_style_radius(ui_barSpeedDownload, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);

    /* --- BOTTOM UPLOAD SECTION --- */
    /* Bottom Purple Bar */
    ui_barSpeedUpload = lv_bar_create(ui_SpeedMeter);
    lv_bar_set_range(ui_barSpeedUpload, 0, 100);
    lv_bar_set_value(ui_barSpeedUpload, 0, LV_ANIM_OFF);
    lv_obj_set_size(ui_barSpeedUpload, 288, 8);
    lv_obj_set_align(ui_barSpeedUpload, LV_ALIGN_CENTER);
    lv_obj_set_x(ui_barSpeedUpload, 0);
    lv_obj_set_y(ui_barSpeedUpload, 62);
    lv_obj_set_style_bg_color(ui_barSpeedUpload, lv_color_hex(0x181530), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ui_barSpeedUpload, 255, LV_PART_MAIN);
    lv_obj_set_style_radius(ui_barSpeedUpload, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_barSpeedUpload, lv_color_hex(0x6366F1), LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa(ui_barSpeedUpload, 255, LV_PART_INDICATOR);
    lv_obj_set_style_radius(ui_barSpeedUpload, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);

    ui_txtSpeedUlHeader = lv_label_create(ui_SpeedMeter);
    lv_obj_set_size(ui_txtSpeedUlHeader, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_txtSpeedUlHeader, LV_ALIGN_BOTTOM_LEFT);
    lv_obj_set_x(ui_txtSpeedUlHeader, 16);
    lv_obj_set_y(ui_txtSpeedUlHeader, -4);
    lv_label_set_text(ui_txtSpeedUlHeader, "UPLOAD");
    lv_obj_set_style_text_color(ui_txtSpeedUlHeader, lv_color_hex(0x818CF8), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_txtSpeedUlHeader, &ui_font_MiSans16, LV_PART_MAIN);

    ui_lblSpeedConns = lv_label_create(ui_SpeedMeter);
    lv_obj_set_size(ui_lblSpeedConns, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_lblSpeedConns, LV_ALIGN_BOTTOM_RIGHT);
    lv_obj_set_x(ui_lblSpeedConns, -16);
    lv_obj_set_y(ui_lblSpeedConns, -4);
    lv_label_set_text(ui_lblSpeedConns, "Active: 0");
    lv_obj_set_style_text_color(ui_lblSpeedConns, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_lblSpeedConns, &ui_font_MiSans16, LV_PART_MAIN);

    /* --- CENTER SECTION (3-COLUMN LAYOUT) --- */
    
    /* 1. LEFT COLUMN: Total Bandwidth (Odometer) */
    lv_obj_t * col_left = lv_obj_create(ui_SpeedMeter);
    lv_obj_remove_style_all(col_left);
    lv_obj_set_size(col_left, 86, 76);
    lv_obj_set_align(col_left, LV_ALIGN_CENTER);
    lv_obj_set_x(col_left, -112);
    lv_obj_set_y(col_left, -1);

    lv_obj_t * lbl_tot_title = lv_label_create(col_left);
    lv_obj_set_size(lbl_tot_title, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(lbl_tot_title, LV_ALIGN_TOP_LEFT);
    lv_obj_set_y(lbl_tot_title, 2);
    lv_label_set_text(lbl_tot_title, "Total Data");
    lv_obj_set_style_text_color(lbl_tot_title, lv_color_hex(0x718096), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_tot_title, &ui_font_MiSans16, LV_PART_MAIN);

    ui_valTotalDl = lv_label_create(col_left);
    lv_obj_set_size(ui_valTotalDl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_valTotalDl, LV_ALIGN_TOP_LEFT);
    lv_obj_set_y(ui_valTotalDl, 24);
    lv_label_set_text(ui_valTotalDl, "v 0.0 MB");
    lv_obj_set_style_text_color(ui_valTotalDl, lv_color_hex(0x00D2FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_valTotalDl, &ui_font_MiSans16, LV_PART_MAIN);

    ui_valTotalUl = lv_label_create(col_left);
    lv_obj_set_size(ui_valTotalUl, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_valTotalUl, LV_ALIGN_TOP_LEFT);
    lv_obj_set_y(ui_valTotalUl, 46);
    lv_label_set_text(ui_valTotalUl, "^ 0.0 MB");
    lv_obj_set_style_text_color(ui_valTotalUl, lv_color_hex(0x818CF8), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_valTotalUl, &ui_font_MiSans16, LV_PART_MAIN);

    /* 2. CENTER COLUMN: AmpliFi Main Speed Core */
    lv_obj_t * col_center = lv_obj_create(ui_SpeedMeter);
    lv_obj_remove_style_all(col_center);
    lv_obj_set_size(col_center, 120, 88);
    lv_obj_set_align(col_center, LV_ALIGN_CENTER);
    lv_obj_set_x(col_center, 0);
    lv_obj_set_y(col_center, -1);

    /* Download Row */
    lv_obj_t * dl_row = lv_obj_create(col_center);
    lv_obj_remove_style_all(dl_row);
    lv_obj_set_size(dl_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(dl_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dl_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(dl_row, 6, 0);
    lv_obj_set_align(dl_row, LV_ALIGN_CENTER);
    lv_obj_set_y(dl_row, -22);

    ui_txtSpeedDlArrow = lv_label_create(dl_row);
    lv_label_set_text(ui_txtSpeedDlArrow, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_color(ui_txtSpeedDlArrow, lv_color_hex(0x00D2FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_txtSpeedDlArrow, LV_FONT_DEFAULT, LV_PART_MAIN);

    ui_valSpeedDownload = lv_label_create(dl_row);
    lv_label_set_text(ui_valSpeedDownload, "0");
    lv_obj_set_style_text_color(ui_valSpeedDownload, lv_color_hex(0x00D2FF), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_valSpeedDownload, &ui_font_MiSans24, LV_PART_MAIN);

    /* Upload Row */
    lv_obj_t * ul_row = lv_obj_create(col_center);
    lv_obj_remove_style_all(ul_row);
    lv_obj_set_size(ul_row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(ul_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ul_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ul_row, 6, 0);
    lv_obj_set_align(ul_row, LV_ALIGN_CENTER);
    lv_obj_set_y(ul_row, 12);

    ui_txtSpeedUlArrow = lv_label_create(ul_row);
    lv_label_set_text(ui_txtSpeedUlArrow, LV_SYMBOL_UP);
    lv_obj_set_style_text_color(ui_txtSpeedUlArrow, lv_color_hex(0x818CF8), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_txtSpeedUlArrow, LV_FONT_DEFAULT, LV_PART_MAIN);

    ui_valSpeedUpload = lv_label_create(ul_row);
    lv_label_set_text(ui_valSpeedUpload, "0");
    lv_obj_set_style_text_color(ui_valSpeedUpload, lv_color_hex(0x818CF8), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_valSpeedUpload, &ui_font_MiSans24, LV_PART_MAIN);

    /* Speed Unit */
    ui_lblSpeedUnit = lv_label_create(col_center);
    lv_obj_set_size(ui_lblSpeedUnit, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_lblSpeedUnit, LV_ALIGN_CENTER);
    lv_obj_set_y(ui_lblSpeedUnit, 38);
    lv_label_set_text(ui_lblSpeedUnit, "Mbps");
    lv_obj_set_style_text_color(ui_lblSpeedUnit, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_lblSpeedUnit, &ui_font_MiSans16, LV_PART_MAIN);

    /* 3. RIGHT COLUMN: Clients & Peak Speed */
    lv_obj_t * col_right = lv_obj_create(ui_SpeedMeter);
    lv_obj_remove_style_all(col_right);
    lv_obj_set_size(col_right, 86, 76);
    lv_obj_set_align(col_right, LV_ALIGN_CENTER);
    lv_obj_set_x(col_right, 112);
    lv_obj_set_y(col_right, -1);

    lv_obj_t * lbl_cli_title = lv_label_create(col_right);
    lv_obj_set_size(lbl_cli_title, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(lbl_cli_title, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_y(lbl_cli_title, 2);
    lv_label_set_text(lbl_cli_title, "Clients");
    lv_obj_set_style_text_color(lbl_cli_title, lv_color_hex(0x718096), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_cli_title, &ui_font_MiSans16, LV_PART_MAIN);

    ui_valSpeedClients = lv_label_create(col_right);
    lv_obj_set_size(ui_valSpeedClients, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_valSpeedClients, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_y(ui_valSpeedClients, 20);
    lv_label_set_text(ui_valSpeedClients, "0 Online");
    lv_obj_set_style_text_color(ui_valSpeedClients, lv_color_hex(0x34D399), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_valSpeedClients, &ui_font_MiSans16, LV_PART_MAIN);

    lv_obj_t * lbl_peak_title = lv_label_create(col_right);
    lv_obj_set_size(lbl_peak_title, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(lbl_peak_title, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_y(lbl_peak_title, 38);
    lv_label_set_text(lbl_peak_title, "Peak DL");
    lv_obj_set_style_text_color(lbl_peak_title, lv_color_hex(0x718096), LV_PART_MAIN);
    lv_obj_set_style_text_font(lbl_peak_title, &ui_font_MiSans16, LV_PART_MAIN);

    ui_valSpeedPeak = lv_label_create(col_right);
    lv_obj_set_size(ui_valSpeedPeak, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_align(ui_valSpeedPeak, LV_ALIGN_TOP_RIGHT);
    lv_obj_set_y(ui_valSpeedPeak, 54);
    lv_label_set_text(ui_valSpeedPeak, "0 M");
    lv_obj_set_style_text_color(ui_valSpeedPeak, lv_color_hex(0xFBBF24), LV_PART_MAIN);
    lv_obj_set_style_text_font(ui_valSpeedPeak, &ui_font_MiSans16, LV_PART_MAIN);

    lv_obj_add_event_cb(ui_SpeedMeter, ui_event_SpeedMeter, LV_EVENT_ALL, NULL);
}

void ui_SpeedMeter_screen_destroy(void)
{
    if(ui_SpeedMeter) {
        lv_obj_del(ui_SpeedMeter);
    }

    ui_SpeedMeter = NULL;
    ui_barSpeedDownload = NULL;
    ui_barSpeedUpload = NULL;
    ui_valSpeedDownload = NULL;
    ui_valSpeedUpload = NULL;
    ui_lblSpeedUnit = NULL;
    ui_lblSpeedIface = NULL;
    ui_lblSpeedConns = NULL;
    ui_valTotalDl = NULL;
    ui_valTotalUl = NULL;
    ui_valSpeedClients = NULL;
    ui_valSpeedPeak = NULL;
    ui_txtSpeedDlHeader = NULL;
    ui_txtSpeedUlHeader = NULL;
    ui_txtSpeedDlArrow = NULL;
    ui_txtSpeedUlArrow = NULL;
}

