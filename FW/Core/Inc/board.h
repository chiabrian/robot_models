/*
 * board.h
 *
 *  Created on: Jun 29, 2026
 *      Author: Brian
 */

#ifndef INC_BOARD_H_
#define INC_BOARD_H_

#include "main.h"

#define LEFT_WHEEL_TIM	TIM3
#define LEFT_WHEEL_FWD	CCR1
#define LEFT_WHEEL_REV	CCR4

#define RIGHT_WHEEL_TIM TIM3
#define RIGHT_WHEEL_FWD CCR2
#define RIGHT_WHEEL_REV	CCR3

#define WEAPON_TIM	TIM1
#define WEAPON_DC	CCR1
#define GPIO_MODE_OUTPUT	0x01
#define GPIO_MODE_AF	0x10

#define LED_GREEN_PORT	GPIOA
#define LED_GREEN_PIN	0
#define LED_RED_PORT	GPIOA
#define LED_RED_PIN		1

#define TIM_CH_LEFT_FWD		LL_TIM_CHANNEL_CH1
#define TIM_CH_LEFT_REV		LL_TIM_CHANNEL_CH4
#define TIM_CH_RIGHT_FWD	LL_TIM_CHANNEL_CH2
#define TIM_CH_RIGHT_REV	LL_TIM_CHANNEL_CH3


#endif /* INC_BOARD_H_ */
