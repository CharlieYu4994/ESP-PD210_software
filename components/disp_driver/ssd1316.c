/**
 * Copyright (c) 2022 CharlieYu4994
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#include "../../lvgl/lvgl.h"
#include "ssd1316.h"

#define OLED_I2C_ADDRESS 0x3C
#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_COLUMNS 128
#define OLED_PAGES 4
#define OLED_PIXEL_PER_PAGE 8

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE 0x80
#define OLED_CONTROL_BYTE_CMD_STREAM 0x00
#define OLED_CONTROL_BYTE_DATA_STREAM 0x40

// Fundamental commands
#define OLED_CMD_SET_CONTRAST 0x81 // follow with contrast
#define OLED_CMD_DISPLAY_RAM 0xA4
#define OLED_CMD_DISPLAY_ALLON 0xA5
#define OLED_CMD_DISPLAY_NORMAL 0xA6
#define OLED_CMD_DISPLAY_INVERTED 0xA7
#define OLED_CMD_SET_REF 0xAD // follow with mode
#define OLED_CMD_DISPLAY_OFF 0xAE
#define OLED_CMD_DISPLAY_ON 0xAF

// Addressing Command Table
#define OLED_CMD_SET_MEMORY_ADDR_MODE 0x20 // follow with mode
#define OLED_CMD_SET_COLUMN_RANGE 0x21     // follow with start and end
#define OLED_CMD_SET_PAGE_RANGE 0x22       // follow with start and end

// Hardware Config
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP 0xA1
#define OLED_CMD_SET_MUX_RATIO 0xA8
#define OLED_CMD_SET_COM_SCAN_MODE_NORMAL 0xC0
#define OLED_CMD_SET_COM_SCAN_MODE_REMAP 0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3
#define OLED_CMD_SET_COM_PIN_MAP 0xDA
#define OLED_CMD_NOP 0xE3

// Timing and Driving Scheme
#define OLED_CMD_SET_DISPLAY_CLK_DIV 0xD5
#define OLED_CMD_SET_PRECHARGE 0xD9
#define OLED_CMD_SET_VCOMH_DESELCT 0xDB

// Charge Pump
#define OLED_CMD_SET_CHARGE_PUMP 0x8D

void ssd1316_init(void)
{
    uint8_t conf[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_DISPLAY_OFF,
        // clock config
        OLED_CMD_SET_DISPLAY_CLK_DIV,
        0x80, // div ratio = 1, frequency setting = 8
        OLED_CMD_SET_VCOMH_DESELCT,
        0x40, // 0.89 * VCC ?
        // GDRAM mapping config
        OLED_CMD_SET_MUX_RATIO,
        0x1F, // 32MUX
        OLED_CMD_SET_DISPLAY_OFFSET,
        0x00, // no shift
        OLED_CMD_SET_DISPLAY_START_LINE,
        OLED_CMD_SET_SEGMENT_REMAP,       // column address 127 to SEG0
        OLED_CMD_SET_COM_SCAN_MODE_REMAP, // scan from COM31 to COM0
        OLED_CMD_SET_COM_PIN_MAP,
        0x12, // disable SEG left/right remap, alternative SEG pin configuration
        // charge pump config
        OLED_CMD_SET_PRECHARGE,
        0x22, // phase1 2DCLK, phase2 2DCLK
        OLED_CMD_SET_CHARGE_PUMP,
        0x15, // 7.5V out, enable charge pump
        // set contrast and turn on display
        OLED_CMD_SET_CONTRAST,
        0x45, // contrast
        OLED_CMD_DISPLAY_RAM};

    uint8_t err = ssd1316_i2c_write(0, OLED_I2C_ADDRESS, conf[0], &conf[1], sizeof(conf));
    assert(0 == err);
}

void ssd1316_flush(lv_disp_drv_t *drv, const lv_area_t *area,
                   lv_color_t *color_map)
{
    uint8_t row1 = area->y1 >> 3;
    uint8_t row2 = area->y2 >> 3;

    uint8_t conf[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_SET_MEMORY_ADDR_MODE,
        0x00, // horizontal addressing mode
        // set vertical range
        OLED_CMD_SET_COLUMN_RANGE,
        (uint8_t)area->x1,
        (uint8_t)area->x2,
        // set horizontal range
        OLED_CMD_SET_PAGE_RANGE,
        row1,
        row2,
    };

    uint8_t err = ssd1316_i2c_write(0, OLED_I2C_ADDRESS, conf[0], &conf[1], sizeof(conf));
    assert(0 == err);
    err = ssd1316_i2c_write(0, OLED_I2C_ADDRESS, OLED_CONTROL_BYTE_DATA_STREAM,
                            color_map, OLED_COLUMNS * (1 + row2 - row1));
    assert(0 == err);
}

void ssd1316_rounder(lv_disp_drv_t *disp_drv, lv_area_t *area)
{
    uint8_t hor_max = disp_drv->hor_res;
    uint8_t ver_max = disp_drv->ver_res;

    area->x1 = 0;
    area->y1 = 0;
    area->x2 = hor_max - 1;
    area->y2 = ver_max - 1;
}

void ssd1316_set_px(lv_disp_drv_t *disp_drv, uint8_t *buf, lv_coord_t buf_w,
                       lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa)
{
    uint16_t byte_index = x + ((y >> 3) * buf_w);
    uint8_t bit_index = y & 0x7;

    if ((color.full == 0) && (LV_OPA_TRANSP != opa))
    {
        buf[byte_index] |= (0x01 << bit_index);
    }
    else
    {
        buf[byte_index] &= ~(0x01 << bit_index);
    }
}

void ssd1316_sleep_in(void)
{
    ssd1316_i2c_write(0, OLED_I2C_ADDRESS, OLED_CONTROL_BYTE_CMD_SINGLE,
                      OLED_CMD_DISPLAY_OFF, 1);
}

void ssd1316_sleep_out(void)
{
    ssd1316_i2c_write(0, OLED_I2C_ADDRESS, OLED_CONTROL_BYTE_CMD_SINGLE,
                      OLED_CMD_DISPLAY_ON, 1);
}