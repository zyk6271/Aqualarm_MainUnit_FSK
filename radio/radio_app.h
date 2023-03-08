/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-07-15     Rick       the first version
 */
#ifndef RADIO_RADIO_APP_H_
#define RADIO_RADIO_APP_H_

#define USE_MODEM_LORA  0
#define USE_MODEM_FSK   1
/*
 * Packet Config
 */
#define Frame_Size                                  47
/*
 * FSK Config
 */
#define FSK_FDEV                                    1660
#define FSK_DATARATE                                4800
#define FSK_BANDWIDTH                               7200
#define FSK_PREAMBLE_LENGTH                         4
#define FSK_FIX_LENGTH_PAYLOAD_ON                   1
#define FSK_AFC_BANDWIDTH                           870
/*
 * Radio Config
 */
#define RF_FREQUENCY                                434700000
#define TX_OUTPUT_POWER                             10
#define RX_TIMEOUT_VALUE                            0
#define TX_TIMEOUT_VALUE                            3000

void RF_Init(void);
void RF_Send(char *payload,int size);

#endif /* RADIO_RADIO_APP_H_ */
