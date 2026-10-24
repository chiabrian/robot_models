/*
 * port.h
 *
 *  Created on: Jul 1, 2026
 *      Author: Brian
 */

#ifndef INC_PORT_H_
#define INC_PORT_H_

#include "stm32c0xx_ll_gpio.h"

void led_toggle(GPIO_TypeDef * PORT, uint32_t PIN);
void led_set(GPIO_TypeDef * PORT, uint32_t PIN);
void led_clear(GPIO_TypeDef * PORT, uint32_t PIN);

#endif /* INC_PORT_H_ */
