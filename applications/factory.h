/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2021-08-25     Rick       the first version
 */
#ifndef APPLICATIONS_FACTORY_H_
#define APPLICATIONS_FACTORY_H_

#include <stdint.h>

uint8_t factory_get_flag(void);
void factory_rf_start(void);
void factory_rf_stop(void);
void factory_detect(void);
void factory_valve_test(void);

#endif /* APPLICATIONS_FACTORY_H_ */
