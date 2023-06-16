/*
 * Copyright (c) 2006-2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-01     RT-Thread    first version
 */

#include <rtthread.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define MCU_VER     "0.0.4"

int main(void)
{
    LOG_I("System Version is %s\r\n",MCU_VER);
    Flash_Init();
    ADC_Init();
    led_Init();
    Key_Reponse();
    WarningInit();
    RTC_Init();
    RF_Init();
    Moto_Init();
    Button_Init();
    WaterScan_Init();
    DetectFactory();
    Gateway_Init();
    while (1)
    {
        rt_thread_mdelay(1000);
    }

    return RT_EOK;
}
