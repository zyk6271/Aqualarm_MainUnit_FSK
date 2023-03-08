/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-13     Rick       the first version
 */
#include "rtthread.h"
#include "main.h"
#include "radio.h"
#include "radio_app.h"
#include "radio_encoder.h"

#define DBG_TAG "RADIO_APP"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

CRC_HandleTypeDef crchandle;
RadioEvents_t RadioEvents;

uint8_t rx_convert_buf[256];
uint8_t tx_convert_buf[256];

static const unsigned char BitReverseTable256[] =
{
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};
static void OnTxDone(void)
{
    LOG_D("OnTxDone\r\n");

    Radio.SetMaxPayloadLength(MODEM_FSK, Frame_Size);
    Radio.Rx(0);
}
void RF_Send(char *payload,int size)
{
    rt_memset(tx_convert_buf, 0, sizeof(tx_convert_buf));
    uint16_t pkt_size = size + 1;//add len
    uint16_t send_size = size + 3;//add len,crc_H,crc_L
    rt_memcpy(&tx_convert_buf[1],payload,size);
    tx_convert_buf[0] = pkt_size;
    uint32_t calc_crc = HAL_CRC_Calculate(&crchandle, (uint32_t *)tx_convert_buf, pkt_size) ^ 0xffff;
    tx_convert_buf[pkt_size+1] = (calc_crc&0xff00)>>8;
    tx_convert_buf[pkt_size] = calc_crc & 0xff;
    for(uint16_t i = 0 ; i < send_size ; i++)
    {
        tx_convert_buf[i] = BitReverseTable256[tx_convert_buf[i]];
    }
    Radio.Send(tx_convert_buf, send_size);
}
static void OnRxDone(uint8_t *src_payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
    uint16_t real_size = BitReverseTable256[src_payload[0]];
    if(real_size > size)
    {
        return;
    }
    rt_memset(rx_convert_buf, 0, sizeof(rx_convert_buf));
    for(uint16_t i=0;i<size;i++)
    {
        rx_convert_buf[i] = BitReverseTable256[src_payload[i]];
    }
    uint32_t src_crc = rx_convert_buf[real_size] | rx_convert_buf[real_size + 1]<<8;
    uint32_t calc_crc = HAL_CRC_Calculate(&crchandle, (uint32_t *)rx_convert_buf, real_size) ^ 0xffff;
    if(calc_crc == src_crc)
    {
        LOG_D("RSSI %d,Recv Size %d,Recv Payload is %s\r\n",rssi,real_size,rx_convert_buf);
        Radio_Parse(rssi,rx_convert_buf,real_size);
    }
    else
    {
        LOG_E("RSSI %d,Calc_CRC is %04X,src_CRC is %04X\r\n",rssi,calc_crc,src_crc);
    }
}
static void OnTxTimeout(void)
{
    LOG_W("OnTxTimeout\r\n");
}
static void OnRxTimeout(void)
{
    LOG_W("OnRxTimeout\r\n");
}
static void OnRxError(void)
{
    LOG_W("OnRxError\r\n");
}

void RF_Init(void)
{
    RadioQueue_Init();
    /* CRC initialization */
    crchandle.Instance = CRC;
    crchandle.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    crchandle.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    crchandle.Init.GeneratingPolynomial = 0x8005;
    crchandle.Init.CRCLength = CRC_POLYLENGTH_16B;
    crchandle.Init.InitValue = 0xFFFF;
    crchandle.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
    crchandle.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    crchandle.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    if (HAL_CRC_Init(&crchandle) != HAL_OK)
    {
        Error_Handler();
    }
    /* Radio initialization */

    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init(&RadioEvents);

    Radio.SetChannel(RF_FREQUENCY);

    Radio.SetTxConfig(MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                    FSK_DATARATE, 0,
                    FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                    false ,0, 0, 0, TX_TIMEOUT_VALUE);

    Radio.SetRxConfig(MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                    0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                    0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, false,
                    0, 0, false, true);

    Radio.SetMaxPayloadLength(MODEM_FSK, Frame_Size);

    Radio.Rx(0);
}
