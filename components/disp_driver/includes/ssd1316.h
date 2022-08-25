/**
 * Copyright (c) 2022 CharlieYu4994
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#ifndef SSD1316_H
#define SSD1316_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "../../lvgl/lvgl.h"
#include "../i2c_manager/i2c_manager.h"

    // for i2c_manager
    void ssd1316_i2c_locking(void *leader);

    void ssd1316_init(void);
    void ssd1316_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
    void ssd1316_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area);
    void ssd1316_set_px(lv_disp_drv_t *disp_drv, uint8_t *buf, lv_coord_t buf_w,
                           lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa);
    void ssd1316_sleep_in(void);
    void ssd1316_sleep_out(void);

#ifdef __cplusplus
}
#endif

#endif /*SSD1316_H*/
