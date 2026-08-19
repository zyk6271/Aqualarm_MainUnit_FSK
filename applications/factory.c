/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-25     Rick       the first version
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "pin_config.h"
#include "factory.h"
#include "moto.h"
#include "radio_encoder.h"

#define DBG_TAG "Factory"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

static uint8_t factory_test = 0;
rt_timer_t factory_rf_cycle_timer = RT_NULL;

uint8_t factory_get_flag(void)
{
    return factory_test;
}

void factory_rf_cycle_callback(void *parameter)
{
    RadioEnqueue(98989898,1,9,0);
}

void factory_rf_start(void)
{
    if(factory_rf_cycle_timer != RT_NULL)
    {
        rt_timer_start(factory_rf_cycle_timer);
        LOG_I("Factory RF cycle started\r\n");
    }
}

void factory_rf_stop(void)
{
    if(factory_rf_cycle_timer != RT_NULL)
    {
        rt_timer_stop(factory_rf_cycle_timer);
        LOG_I("Factory RF cycle stopped\r\n");
    }
}

void factory_detect(void)
{
    rt_pin_mode(FACTORY_DETECT_PIN, PIN_MODE_INPUT_PULLUP);
    if(rt_pin_read(FACTORY_DETECT_PIN) == 0)
    {
        rt_thread_mdelay(200);
        if(rt_pin_read(FACTORY_DETECT_PIN) == 0)
        {
            factory_test = 1;
            factory_rf_cycle_timer = rt_timer_create("factory_rf", factory_rf_cycle_callback, RT_NULL, 2000, RT_TIMER_FLAG_PERIODIC | RT_TIMER_FLAG_SOFT_TIMER);
            factory_rf_start();
        }
    }
}

void factory_valve_test(void)
{
    factory_rf_stop();
    valve_factory_check_reset();
    Moto_Detect();
}
