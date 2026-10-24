/*
 * port_stm32c0.c
 *
 *  Created on: Jun 29, 2026
 *      Author: Brian
 */

#include "stdint.h"
#include "port.h"

void led_toggle(GPIO_TypeDef * PORT, uint32_t PIN)
{
	if( (PORT->ODR & (1<<PIN)) == 0)
	{
		PORT->BSRR = 1 << (PIN);
	}
	else
	{
		PORT->BRR = 1 << (PIN);
	}
}

void led_set(GPIO_TypeDef * PORT, uint32_t PIN)
{
	PORT->BSRR = 1 << (PIN);
}

void led_clear(GPIO_TypeDef * PORT, uint32_t PIN)
{
	PORT->BRR = 1 << (PIN);
}
