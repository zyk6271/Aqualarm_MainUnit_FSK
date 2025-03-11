/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-27     Rick       the first version
 */
#include "rtthread.h"
#include "rtdevice.h"
#include "pin_config.h"
#include "led.h"
#include "key.h"
#include "moto.h"
#include "flashwork.h"
#include "status.h"
#include "gateway.h"
#include "radio_encoder.h"
#include "device.h"
#include "work.h"

#define DBG_TAG "valve"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>

#define VALVE_STATUS_CLOSE   0
#define VALVE_STATUS_OPEN    1

uint8_t valve_left_low_cnt,valve_right_low_cnt = 0;
uint8_t valve_left_low_check_start,valve_right_low_check_start = 0;
uint8_t valve_left_detect_low_success,valve_right_detect_low_success = 0;
uint8_t valve_left_warning_result,valve_right_warning_result = 0;    //0:normal,1:warning

rt_timer_t valve_check_detect_low_timer;
rt_timer_t valve_check_detect_timeout_timer;
rt_timer_t valve_check_left_final_timer;
rt_timer_t valve_check_right_final_timer;
rt_timer_t valve_detect_once_timer = RT_NULL;

extern uint8_t ValveStatus;
extern enum Device_Status Now_Status;
extern Device_Info Global_Device;

void Moto_InitOpen(uint8_t ActFlag)
{
    LOG_D("Moto Open Init Now is is %d , act is %d\r\n",Global_Device.LastFlag,ActFlag);
    if((Global_Device.LastFlag == OtherOff && ActFlag == OtherOpen)||(Global_Device.LastFlag != OtherOff))
    {
        Now_Status = Open;
        led_valve_on();
        ValveStatus = 1;
        Global_Device.LastFlag = ActFlag;
        Flash_Moto_Change(ActFlag);
        rt_pin_write(Turn1,1);
        rt_pin_write(Turn2,1);
    }
    else if(Global_Device.LastFlag == OtherOff && ActFlag == NormalOpen)
    {
        led_valve_fail();
        LOG_D("No permissions to Open\r\n");
    }
    else
    {
        led_notice_once();
        LOG_D("No permissions to Open\r\n");
    }
}
void Moto_Open(uint8_t ActFlag)
{
    if((Global_Device.LastFlag == OtherOff && ActFlag == OtherOpen)||(Global_Device.LastFlag != OtherOff))
    {
        Now_Status = Open;
        led_valve_on();
        ValveStatus=1;
        Global_Device.LastFlag = ActFlag;
        Flash_Moto_Change(ActFlag);
        rt_pin_write(Turn1,1);
        rt_pin_write(Turn2,1);
        rt_timer_stop(valve_check_detect_low_timer);
        rt_timer_stop(valve_check_detect_timeout_timer);
        rt_timer_stop(valve_check_left_final_timer);
        rt_timer_stop(valve_check_right_final_timer);
        if(ActFlag==NormalOpen)
        {
            ControlUpload_GW(1,0,1,1);
            rt_timer_start(valve_detect_once_timer);
        }
        Delay_Timer_Stop();
    }
    else if(Global_Device.LastFlag == OtherOff && ActFlag == NormalOpen)
    {
        led_valve_fail();
        LOG_D("No permissions to Open\r\n");
    }
}
void Moto_Close(uint8_t ActFlag)
{
    if(Global_Device.LastFlag != OtherOff )
    {
        Now_Status = Close;
        led_valve_off();
        ValveStatus=0;
        Global_Device.LastFlag = ActFlag;
        Flash_Moto_Change(ActFlag);
        if(ActFlag==NormalOff)
        {
            ControlUpload_GW(1,0,1,0);
        }
        rt_pin_write(Turn1,0);
        rt_pin_write(Turn2,0);
        Delay_Timer_Stop();
        rt_timer_stop(valve_check_detect_low_timer);
        rt_timer_stop(valve_check_detect_timeout_timer);
        rt_timer_stop(valve_check_left_final_timer);
        rt_timer_stop(valve_check_right_final_timer);
        rt_timer_stop(valve_detect_once_timer);
        Key_IO_Init();
        WaterScan_IO_Init();
    }
    else if(Global_Device.LastFlag == OtherOff && ActFlag == OtherOff)
    {
        Now_Status = Close;
        ValveStatus=0;
        beep_once();
        Delay_Timer_Stop();
        LOG_D("Moto is alreay otheroff\r\n");
    }
    else
    {
        led_valve_fail();
        LOG_D("No permissions to Off\r\n");
    }
}

uint8_t Get_Moto1_Fail_FLag(void)
{
    return valve_left_warning_result;
}

uint8_t Get_Moto2_Fail_FLag(void)
{
    return valve_right_warning_result;
}

uint8_t valve_check_detect_low_timer_callback(void *parameter)
{
    if(valve_left_low_check_start == 1 && valve_left_detect_low_success == 0)
    {
        if(rt_pin_read(Senor1) == 0)
        {
            valve_left_low_cnt++;
        }
        else
        {
            valve_left_low_cnt = 0;
        }

        if(valve_left_low_cnt > 3)
        {
            valve_left_low_check_start = 0;
            valve_left_detect_low_success = 1;
            rt_pin_write(Turn1,PIN_HIGH);
            rt_timer_start(valve_check_left_final_timer);
        }
        rt_kprintf("valve_left_low_cnt %d\r\n",valve_left_low_cnt);
    }

    if(valve_right_low_check_start == 1 && valve_right_detect_low_success == 0)
    {
        if(rt_pin_read(Senor2) == 0)
        {
            valve_right_low_cnt++;
        }
        else
        {
            valve_right_low_cnt = 0;
        }

        if(valve_right_low_cnt > 3)
        {
            valve_right_low_check_start = 0;
            valve_right_detect_low_success = 1;
            rt_pin_write(Turn2,PIN_HIGH);
            rt_timer_start(valve_check_right_final_timer);
        }
        rt_kprintf("valve_right_low_cnt %d\r\n",valve_right_low_cnt);
    }

    if(valve_left_low_check_start == 0 && valve_right_low_check_start == 0)
    {
        rt_timer_stop(valve_check_detect_low_timer);
    }
}

uint8_t valve_check_detect_timeout_timer_callback(void *parameter)
{
    if(valve_left_low_check_start == 1 && valve_left_detect_low_success == 0)
    {
        valve_left_warning_result = 1;
        valve_left_low_check_start = 0;
        Warning_Enable_Num(6);
        rt_pin_write(Turn1,PIN_HIGH);
        rt_timer_stop(valve_check_detect_low_timer);
        rt_kprintf("valve_left_turn_check fail\r\n");
    }

    if(valve_right_low_check_start == 1 && valve_right_detect_low_success == 0)
    {
        valve_right_warning_result = 1;
        valve_right_low_check_start = 0;
        Warning_Enable_Num(9);
        rt_pin_write(Turn2,PIN_HIGH);
        rt_timer_stop(valve_check_detect_low_timer);
        rt_kprintf("valve_right_turn_check fail\r\n");
    }
}

uint8_t valve_check_left_final_timer_callback(void *parameter)
{
    if(rt_pin_read(Senor1) == 1)
    {
        valve_left_warning_result = 0;
        if(valve_right_warning_result == 0)
        {
            WarUpload_GW(1,0,2,0);//MOTO1解除报警
        }
        rt_kprintf("valve_left_check success\r\n");
    }
    else
    {
        valve_left_warning_result = 1;
        Warning_Enable_Num(6);
        rt_kprintf("valve_left_check fail\r\n");
    }
}

uint8_t valve_check_right_final_timer_callback(void *parameter)
{
    if(rt_pin_read(Senor2) == 1)
    {
        valve_right_warning_result = 0;
        if(valve_left_warning_result == 0)
        {
            WarUpload_GW(1,0,2,1);//MOTO2解除报警
        }
        rt_kprintf("valve_right_check success\r\n");
    }
    else
    {
        valve_right_warning_result = 1;
        Warning_Enable_Num(9);
        rt_kprintf("valve_right_check fail\r\n");
    }
}

void Moto_Detect(void)
{
    if(ValveStatus == VALVE_STATUS_CLOSE)
    {
        return;
    }

    valve_left_low_cnt = 0;
    valve_right_low_cnt = 0;
    valve_left_low_check_start = 0;
    valve_right_low_check_start = 0;
    valve_left_detect_low_success = 0;
    valve_right_detect_low_success = 0;

    rt_timer_stop(valve_check_detect_low_timer);
    rt_timer_stop(valve_check_detect_timeout_timer);
    rt_timer_stop(valve_check_left_final_timer);
    rt_timer_stop(valve_check_right_final_timer);
    rt_timer_stop(valve_detect_once_timer);

    if(rt_pin_read(Senor1))
    {
        valve_left_low_check_start = 1;
        rt_pin_write(Turn1,PIN_LOW);
        rt_timer_start(valve_check_detect_low_timer);
        rt_timer_start(valve_check_detect_timeout_timer);
        rt_kprintf("valve_left_turn_check start\r\n");
    }
    if(rt_pin_read(Senor2))
    {
        valve_right_low_check_start = 1;
        rt_pin_write(Turn2,PIN_LOW);
        rt_timer_start(valve_check_detect_low_timer);
        rt_timer_start(valve_check_detect_timeout_timer);
        rt_kprintf("valve_right_turn_check start\r\n");
    }
}

void valve_detect_once_timer_callback(void *parameter)
{
    Moto_Detect();
}

void Moto_Init(void)
{
    rt_pin_mode(Turn1,PIN_MODE_OUTPUT);
    rt_pin_mode(Turn2,PIN_MODE_OUTPUT);
    rt_pin_mode(Senor1,PIN_MODE_INPUT);
    rt_pin_mode(Senor2,PIN_MODE_INPUT);

    valve_detect_once_timer  = rt_timer_create("valve_detect", valve_detect_once_timer_callback, RT_NULL, 60*1000*5, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    valve_check_detect_low_timer = rt_timer_create("valve_check_detect_low", valve_check_detect_low_timer_callback, RT_NULL, 500, RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER);
    valve_check_detect_timeout_timer = rt_timer_create("valve_check_detect_timeout", valve_check_detect_timeout_timer_callback, RT_NULL, 10000, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    valve_check_left_final_timer = rt_timer_create("valve_check_left_final", valve_check_left_final_timer_callback, RT_NULL, 10000, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);
    valve_check_right_final_timer = rt_timer_create("valve_check_right_final", valve_check_right_final_timer_callback, RT_NULL, 10100, RT_TIMER_FLAG_ONE_SHOT|RT_TIMER_FLAG_SOFT_TIMER);

    if(Flash_Get_SlaveAlarmFlag())
    {
        Warning_Enable_Num(2);
        LOG_W("Moto is Init Fail,Last is Slaver Alarm\r\n");
        return;
    }
    if(Global_Device.LastFlag != OtherOff)
    {
        Moto_InitOpen(NormalOpen);
    }
}
