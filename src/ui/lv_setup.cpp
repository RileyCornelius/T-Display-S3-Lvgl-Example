#define TOUCH_MODULES_CST_SELF // Must be defined before including TouchLib.h
#include <TouchLib.h>
#include <TFT_eSPI.h>
#include <lvgl.h>
#include "lv_setup.h"
#include "../pin_config.h"

/* Screen dimensions */
#define LV_SCREEN_WIDTH 320
#define LV_SCREEN_HEIGHT 170
#define LV_BUF_SIZE (LV_SCREEN_WIDTH * LV_SCREEN_HEIGHT)

TFT_eSPI tft = TFT_eSPI(LV_SCREEN_HEIGHT, LV_SCREEN_WIDTH);
TouchLib touch(Wire, PIN_IIC_SDA, PIN_IIC_SCL, CTS820_SLAVE_ADDRESS, PIN_TOUCH_RES);

/* Display rendering callback */
static void lv_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h);
    lv_disp_flush_ready(disp);
}

/* Read the touch screen callback*/
static void lv_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
    if (touch.read())
    {
        TP_Point t = touch.getPoint(0);
        data->point.x = LV_SCREEN_WIDTH - t.x; // Invert x
        data->point.y = t.y;
        data->state = LV_INDEV_STATE_PR;
        // Serial.print("x: " + String(LV_SCREEN_WIDTH - t.x) + " y: " + String(t.y) + " p: " + String(t.pressure) + " \n");
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

/* Setup lvgl with display and touch pad */
void lv_begin()
{
    /* Serial init */
    Serial.begin(115200);

    /* Enable display */
    pinMode(PIN_LCD_POWER_ON, OUTPUT);
    digitalWrite(PIN_LCD_POWER_ON, HIGH); // Turn on the display enable pin

    /* TFT init */
    tft.begin();
    tft.setRotation(3); // Landscape orientation

    /* Call before other lv functions */
    lv_init();

    /* Initialize the display buffer */
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t *lv_disp_buf;
    lv_disp_buf = (lv_color_t *)heap_caps_malloc(LV_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    lv_disp_draw_buf_init(&draw_buf, lv_disp_buf, NULL, LV_BUF_SIZE);

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_SCREEN_WIDTH;
    disp_drv.ver_res = LV_SCREEN_HEIGHT;
    disp_drv.flush_cb = lv_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* Initialize touch screen if available */
    if (touch.init())
    {
        touch.setRotation(1);
        /*Initialize the input device driver*/
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        indev_drv.read_cb = lv_touchpad_read;
        lv_indev_drv_register(&indev_drv);
        Serial.println("Touch screen initialized");
    }
    else
    {
        Serial.println("Touch screen initialization failed");
    }

    Serial.printf("Lvgl v%d.%d.%d initialized\n", lv_version_major(), lv_version_minor(), lv_version_patch());
}

/* Handles updating the display and touch events */
void lv_handler()
{
    lv_task_handler();
    delay(5);
}