/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-03     Rick       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "signal_led.h"
#include "pin_config.h"

//定义信号灯对象句柄
static led_t *off_red = RT_NULL;
static led_t *loss_red = RT_NULL;
static led_t *on_green = RT_NULL;
static led_t *beep = RT_NULL;
static led_t *loss_beep = RT_NULL;
static led_t *key_beep = RT_NULL;
static led_t *learn_beep = RT_NULL;
static led_t *key_led = RT_NULL;
static led_t *beep_three = RT_NULL;
static led_t *off_red_led_three = RT_NULL;
static led_t *wifi_green_led = RT_NULL;
static led_t *wifi_red_led = RT_NULL;
static led_t *wifi_blue_led = RT_NULL;

extern uint32_t Gateway_ID;

//定义内存操作函数接口
led_mem_opreation_t led_mem_opreation;

static void gw_green_on(void *param)
{
    ws2812b_green(0,1);
}

static void gw_green_off(void *param)
{
    ws2812b_green(0,0);
}

static void gw_red_on(void *param)
{
    ws2812b_red(0,1);
}

static void gw_red_off(void *param)
{
    ws2812b_red(0,0);
}

static void gw_blue_on(void *param)
{
    ws2812b_blue(0,1);
}

static void gw_blue_off(void *param)
{
    ws2812b_blue(0,0);
}

static void off_red_on(void *param)
{
    ws2812b_red(1,1);
}

static void off_red_off(void *param)
{
    ws2812b_red(1,0);
}

static void on_green_on(void *param)
{
    ws2812b_green(2,1);
}

static void on_green_off(void *param)
{
    ws2812b_green(2,0);
}

static void beep_on(void *param)
{
    rt_pin_mode(BEEP_PIN,PIN_MODE_OUTPUT);
    rt_pin_write(BEEP_PIN,PIN_HIGH);
}

static void beep_close(void *param)
{
    rt_pin_mode(BEEP_PIN,PIN_MODE_OUTPUT);
    rt_pin_write(BEEP_PIN,PIN_LOW);
}

static void led_run(void *parameter)
{
    ws2812b_init();
    while(1)
    {
        rt_thread_mdelay(LED_TICK_TIME);
        led_ticks();
        RGB_SendArray();
    }
}

void beep_three_times(void)
{
    led_start(beep_three);
    led_start(off_red_led_three);
}
void wifi_communication_blink(void)
{
    led_start(wifi_blue_led);
}
void wifi_led(uint8_t num)
{
    switch(num)
    {
    case 0://无设备
        led_stop(wifi_red_led);
        led_stop(wifi_green_led);
        break;
    case 1://心跳成功
        led_stop(wifi_red_led);
        led_set_mode(wifi_green_led, LOOP_PERMANENT,"200,0,");
        led_start(wifi_green_led);
        break;
    case 2://心跳失败
        led_stop(wifi_green_led);
        led_set_mode(wifi_red_led, LOOP_PERMANENT,"200,0,");
        led_start(wifi_red_led);
        break;
    }
}
void led_valve_fail(void)
{
    led_set_mode(beep, 3,"200,200,");
    led_start(beep);
    led_set_mode(off_red, 3,"200,200,");
    led_start(off_red);
}
void led_notice_once(void)
{
    led_set_mode(beep, 1,"200,0,");
    led_start(beep);
    led_set_mode(off_red, 1,"200,0,");
    led_start(off_red);
}
void led_factory_start(void)
{
    led_set_mode(beep, 5,"200,200,");
    led_start(beep);
    led_set_mode(off_red, 5,"200,200,");
    led_start(off_red);
}
void led_learn_start(void)
{
    led_set_mode(beep, 5,"200,200,");
    led_start(beep);
    led_set_mode(off_red, 75,"200,200,");
    led_start(off_red);
}
void led_slave_low_start(void)
{
    led_set_mode(beep, LOOP_PERMANENT,"200,5000,");
    led_start(beep);
    led_set_mode(off_red, LOOP_PERMANENT,"200,5000,");
    led_start(off_red);
}
void led_moto_fail_start(void)
{
    led_set_mode(beep, LOOP_PERMANENT,"200,200,200,200,200,200,200,200,200,200,200,10000,");
    led_start(beep);
    led_set_mode(off_red, LOOP_PERMANENT,"200,200,200,200,200,200,200,200,200,200,200,10000,");
    led_start(off_red);
}
void led_offline_start(void)
{
    led_set_mode(beep, LOOP_PERMANENT,"200,200,200,200,200,200,200,5000,");
    led_start(beep);
    led_set_mode(off_red, LOOP_PERMANENT,"200,200,200,200,200,200,200,5000,");
    led_start(off_red);
}
void led_master_lost_start(void)
{
    led_set_mode(loss_beep, LOOP_PERMANENT,"0,5000,200,200,200,5000,");
    led_start(loss_beep);
    led_set_mode(loss_red, LOOP_PERMANENT,"0,5000,200,200,200,5000,");
    led_start(loss_red);
}
void led_water_alarm_start(void)
{
    led_set_mode(beep, LOOP_PERMANENT,"200,200,200,200,200,5000,");
    led_start(beep);
    led_set_mode(off_red, LOOP_PERMANENT,"200,200,200,200,200,5000,");
    led_start(off_red);
}
void beep_stop(void)
{
    led_stop(loss_beep);
    led_stop(beep);
}
void key_down(void)
{
    led_start(key_beep);
    led_start(key_led);
}
void beep_once(void)
{
    led_start(key_beep);
}
void learn_fail_ring(void)
{
    led_start(learn_beep);
}
void led_relearn(void)
{
    led_set_mode(off_red, 75,"200,200,");
    led_start(off_red);
}
void led_ntc_alarm(void)
{
    led_set_mode(beep, LOOP_PERMANENT,"50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50\
                            ,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,2000,");
    led_set_mode(off_red, LOOP_PERMANENT,"50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50\
                            ,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,2000,");
    led_set_mode(on_green, LOOP_PERMANENT,"50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50\
                            ,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,2000,");
    led_start(off_red);
    led_start(on_green);
    led_start(beep);
}
void led_factory_warn(void)
{
    led_stop(on_green);
    led_set_mode(beep, 1,"200,0,");
    led_set_mode(off_red, 1,"200,0,");
    led_start(off_red);
    led_start(beep);
}
void led_factory_normal(void)
{
    led_stop(off_red);
    led_set_mode(beep, 1,"200,0,");
    led_set_mode(on_green, 1,"200,0,");
    led_set_mode(wifi_green_led, 1,"200,0,");
    led_set_mode(wifi_red_led, 1,"200,0,");
    led_start(beep);
    led_start(on_green);
    led_start(wifi_green_led);
    led_start(wifi_red_led);
}
void led_valve_on(void)
{
    led_stop(off_red);
    led_set_mode(on_green,LOOP_PERMANENT,"200,0,");
    led_start(on_green);
    led_start(key_beep);
}
void led_valve_off(void)
{
    led_stop(on_green);
    led_start(key_beep);
}
void led_warn_off(void)
{
    led_stop(off_red);
    led_stop(loss_red);
}
int led_Init(void)
{
    led_mem_opreation.malloc_fn = (void* (*)(size_t))rt_malloc;
    led_mem_opreation.free_fn = rt_free;
    led_set_mem_operation(&led_mem_opreation);

    //初始化信号灯对象
    off_red = led_create(gw_green_on, gw_green_off, NULL);
    on_green = led_create(off_red_on, off_red_off, NULL);
    beep = led_create(beep_on, beep_close, NULL);

    off_red = led_create(off_red_on, off_red_off, NULL);
    led_set_mode(off_red, LOOP_PERMANENT, "200,200,");
    loss_red = led_create(off_red_on, off_red_off, NULL);
    led_set_mode(loss_red, LOOP_PERMANENT, "200,200,");
    key_led = led_create(off_red_on, off_red_off, NULL);
    led_set_mode(key_led, 1, "200,1,");
    off_red_led_three = led_create(off_red_on, off_red_off, NULL);
    led_set_mode(off_red_led_three, 3, "200,200,");

    on_green = led_create(on_green_on, on_green_off, NULL);
    led_set_mode(on_green, 3, "200,200,");

    beep = led_create(beep_on, beep_close, NULL);
    led_set_mode(beep, LOOP_PERMANENT, "200,200,");
    loss_beep = led_create(beep_on, beep_close, NULL);
    led_set_mode(loss_beep, LOOP_PERMANENT, "200,200,");
    key_beep = led_create(beep_on, beep_close, NULL);
    led_set_mode(key_beep, 1, "200,1,");
    beep_three = led_create(beep_on, beep_close, NULL);
    led_set_mode(beep_three, 3, "200,200,");
    learn_beep = led_create(beep_on, beep_close, NULL);
    led_set_mode(learn_beep, 3, "50,50,200,200,");

    wifi_green_led = led_create(gw_green_on, gw_green_off, NULL);
    led_set_mode(wifi_green_led, LOOP_PERMANENT, "200,200,");

    wifi_red_led = led_create(gw_red_on, gw_red_off, NULL);
    led_set_mode(wifi_red_led, LOOP_PERMANENT, "200,200,");

    wifi_blue_led = led_create(gw_blue_on, gw_blue_off, NULL);
    led_set_mode(wifi_blue_led, 1, "50,0,");

    rt_thread_t tid = RT_NULL;
    tid = rt_thread_create("signal_led",
                            led_run,
                            RT_NULL,
                            512,
                            RT_THREAD_PRIORITY_MAX/2,
                            100);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return RT_EOK;
}
