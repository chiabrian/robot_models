/*
 * Copyright (c) 2021, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ti_msp_dl_config.h"
#include "stdlib.h"

#define TICKS_TO_US(x)  ((x - 992) * 5 / 8 + 1500)
#define US_TO_TICKS(x)  ((x - 1500) * 8 / 5 + 992)

typedef struct
{
    uint32_t channel_01: 11;
    uint32_t channel_02: 11;
    uint32_t channel_03: 11;
    uint32_t channel_04: 11;
    uint32_t channel_05: 11;
    uint32_t channel_06: 11;
    uint32_t channel_07: 11;
    uint32_t channel_08: 11;
    uint32_t channel_09: 11;
    uint32_t channel_10: 11;
    uint32_t channel_11: 11;
    uint32_t channel_12: 11;
    uint32_t channel_13: 11;
    uint32_t channel_14: 11;
    uint32_t channel_15: 11;
    uint32_t channel_16: 11;
}rc_ch_t;

typedef union {
        uint8_t bytes[61];
        rc_ch_t rc_ch;
}payload_t;

typedef struct
{
    uint8_t spare;
    uint8_t sync;
    uint8_t len;
    uint8_t type;
    payload_t payload;
}erls_t;

typedef struct
{
    int32_t channel_01;
    int32_t channel_02;
    int32_t channel_03;
    int32_t channel_04;
    int32_t channel_05;
    int32_t ch1_percent;
    int32_t ch2_percent;
    int32_t pwm1a;
    int32_t pwm1b;
    int32_t pwm2a;
    int32_t pwm2b;
    int32_t pwm3a;
    int32_t pwm3b;
    
}controls_t;

volatile int32_t g_sys_tick = 0;
volatile uint8_t rx_buf[256];
volatile erls_t frame;
volatile uint8_t * rx_ptr = &frame.sync;
volatile uint8_t rx_cnt = 63;
volatile rc_ch_t * rc_ch_ptr = (rc_ch_t*)&frame.payload.bytes;
volatile uint8_t frame_rdy = 0;
volatile uint8_t erls_time_out = 0;
volatile controls_t controls;

static const uint8_t crc_table[256] = {
    0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54, 0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
    0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06, 0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
    0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0, 0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
    0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2, 0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
    0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9, 0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
    0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B, 0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
    0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D, 0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
    0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F, 0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
    0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB, 0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
    0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9, 0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
    0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F, 0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
    0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D, 0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
    0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26, 0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
    0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74, 0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
    0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82, 0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
    0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0, 0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9,
};

uint8_t calc_crc8(uint8_t*data, uint8_t len)
{
    uint8_t crc = 0x00;
    for(uint32_t i=0;i<len;i++)
    {
        crc = crc ^ data[i];
        crc = crc_table[crc];
    }
    return crc;
}

void parse_rc_ch(void)
{
    controls.channel_01 = TICKS_TO_US(rc_ch_ptr->channel_01);
    controls.channel_02 = TICKS_TO_US(rc_ch_ptr->channel_02);
    controls.channel_03 = TICKS_TO_US(rc_ch_ptr->channel_03);
    controls.channel_04 = TICKS_TO_US(rc_ch_ptr->channel_04);
    controls.channel_05 = TICKS_TO_US(rc_ch_ptr->channel_05);

    // Out of range set to 0
    // Std range 988-2012us
    // Extended Range 885-2115us
    // Half Range 494-1006us
    if( (controls.channel_01 > 2050) || (controls.channel_01 < 950) )
    {
    	controls.channel_01 = 1500;
    }
	if( (controls.channel_02 > 2050) || (controls.channel_02 < 950) )
	{
		controls.channel_02 = 1500;
	}
	if( (controls.channel_03 > 2050) || (controls.channel_03 < 950) )
	{
		controls.channel_03 = 1500;
	}
	if( (controls.channel_04 > 2050) || (controls.channel_04 < 950) )
	{
		controls.channel_04 = 1500;
	}
	if( (controls.channel_05 > 2050) || (controls.channel_05 < 950) )
	{
		controls.channel_05 = 1500;
	}
}


void convert_crsf(void)
{
    // Scale to +/- 500
    controls.ch1_percent = controls.channel_01;
    if( controls.ch1_percent > 2000 ) {
        controls.ch1_percent = 2000;
    }
    if( controls.ch1_percent < 1000 ) {
        controls.ch1_percent = 1000;
    }
    controls.ch1_percent -= 1500;

    controls.ch2_percent = controls.channel_02; 
    if( controls.ch2_percent > 2000 ) {
        controls.ch2_percent = 2000;
    }
    if( controls.ch2_percent < 1000 ) {
        controls.ch2_percent = 1000;
    }
    controls.ch2_percent -= 1500;
}

void set_drive_pwm(void)
{    
    int32_t left_wheel;
    int32_t right_wheel;
    int32_t max_mag;
    // Differential Drive, Scale 500 by 20 to 10000
    // U5 Left Wheel PWM1
    left_wheel = controls.ch2_percent + controls.ch1_percent;
    left_wheel *= 20;
    // U6 Right Wheel PWM2
    right_wheel = controls.ch2_percent - controls.ch1_percent;
    right_wheel *= 20;

    if(abs(left_wheel) >= abs(right_wheel)) {
        max_mag = abs(left_wheel);
    }
    else {
        max_mag = abs(right_wheel);
    }
    if( max_mag > 10000)
    {
        left_wheel *= 10000;
        left_wheel /= max_mag;
        right_wheel *= 10000;
        right_wheel /= max_mag;
    }

    if( left_wheel >= 0)
    {
        controls.pwm1a = abs(left_wheel);
        controls.pwm1b = 0;
    }
    else {
        controls.pwm1a = 0;
        controls.pwm1b = abs(left_wheel);
    }
    
    if( right_wheel >= 0)
    {
        controls.pwm2a = 0;
        controls.pwm2b = abs(right_wheel);
    }
    else {
        controls.pwm2a = abs(right_wheel);
        controls.pwm2b = 0;
    }

    

    // // Drive Priority
    // if( abs(controls.ch2_percent) >= abs(controls.ch1_percent)) {
    //     // Forward
    //     if( controls.ch2_percent >= 0)
    //     {
    //         controls.pwm1a = abs(controls.ch2_percent * 20);
    //         controls.pwm1b = 0;
    //         controls.pwm2a = abs(controls.ch2_percent * 20);
    //         controls.pwm2b = 0;
    //     }        
    //     // Reverse
    //     else
    //     {
    //         controls.pwm1a = 0;
    //         controls.pwm1b = abs(controls.ch2_percent * 20);
    //         controls.pwm2a = 0;
    //         controls.pwm2b = abs(controls.ch2_percent * 20);
    //     }
    // }
    // // Turn Priority
    // else {
    //     // Turn Left
    //     if( controls.ch1_percent >= 0)
    //     {
    //         controls.pwm1a = abs(controls.ch1_percent * 20);
    //         controls.pwm1b = 0;
    //         controls.pwm2a = 0;
    //         controls.pwm2b = abs(controls.ch1_percent * 20);
    //     }
    //     // Turn Right
    //     else
    //     {
    //         controls.pwm1a = 0;
    //         controls.pwm1b = abs(controls.ch1_percent * 20);
    //         controls.pwm2a = abs(controls.ch1_percent * 20);
    //         controls.pwm2b = 0;
    //     }   
    // }

    if( controls.pwm1a != DL_TimerG_getCaptureCompareValue(PWM_U5_LEFT_INST,DL_TIMER_CC_0_INDEX)) {
        DL_TimerG_setCaptureCompareValue(PWM_U5_LEFT_INST, controls.pwm1a, DL_TIMER_CC_0_INDEX);
    }
    if( controls.pwm1b != DL_TimerG_getCaptureCompareValue(PWM_U5_LEFT_INST,DL_TIMER_CC_1_INDEX)) {
        DL_TimerG_setCaptureCompareValue(PWM_U5_LEFT_INST, controls.pwm1b, DL_TIMER_CC_1_INDEX);
    }
    if( controls.pwm2a != DL_TimerG_getCaptureCompareValue(PWM_U6_RIGHT_INST,DL_TIMER_CC_0_INDEX)) {
        DL_TimerG_setCaptureCompareValue(PWM_U6_RIGHT_INST, controls.pwm2a, DL_TIMER_CC_0_INDEX);
    }
    if( controls.pwm2b != DL_TimerG_getCaptureCompareValue(PWM_U6_RIGHT_INST,DL_TIMER_CC_1_INDEX)) {
        DL_TimerG_setCaptureCompareValue(PWM_U6_RIGHT_INST, controls.pwm2b, DL_TIMER_CC_1_INDEX);
    }

    // Weapon Control
    if(controls.channel_04 > 1500) {
        controls.pwm3a = 5000;
        controls.pwm3b = 0;
    }
    else {
        controls.pwm3a = 0;
        controls.pwm3b = 0;
    }
    if( controls.pwm3a != DL_TimerG_getCaptureCompareValue(PWM_U7_WEAPON_INST,DL_TIMER_CC_0_INDEX)) {
        DL_TimerG_setCaptureCompareValue(PWM_U7_WEAPON_INST, controls.pwm3a, DL_TIMER_CC_0_INDEX);
    }
}

int main(void)
{
    SYSCFG_DL_init();

    DL_GPIO_setPins(LED_PORT, LED_Red_PIN);
    while( g_sys_tick < 50){}
    DL_GPIO_clearPins(LED_PORT, LED_Red_PIN);

    NVIC_ClearPendingIRQ(UART_0_INST_INT_IRQN);
    NVIC_EnableIRQ(UART_0_INST_INT_IRQN);

    DL_TimerG_setCaptureCompareValue(PWM_U7_WEAPON_INST, 0, DL_TIMER_CC_0_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_U5_LEFT_INST, 0, DL_TIMER_CC_0_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_U6_RIGHT_INST, 0, DL_TIMER_CC_0_INDEX);

    DL_TimerG_setCaptureCompareValue(PWM_U7_WEAPON_INST, 0, DL_TIMER_CC_1_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_U5_LEFT_INST, 0, DL_TIMER_CC_1_INDEX);
    DL_TimerG_setCaptureCompareValue(PWM_U6_RIGHT_INST, 0, DL_TIMER_CC_1_INDEX);
    while (1) {
        if(g_sys_tick == 100)
        {
            g_sys_tick = 0;
            DL_GPIO_togglePins(LED_PORT, LED_Green_PIN);
        }
        if( frame_rdy )
        {
            rx_cnt = 63;
            frame_rdy = 0;
            if( calc_crc8((uint8_t*)&frame.type,frame.len) == 0)
            {
                parse_rc_ch();
                erls_time_out = 0;
            }
        }
        if(erls_time_out < 5)
        {
            if( !DL_TimerG_isRunning(PWM_U7_WEAPON_INST) )
            {
                DL_TimerG_startCounter(PWM_U7_WEAPON_INST);
            }
            if( !DL_TimerG_isRunning(PWM_U5_LEFT_INST) )
            {
                DL_TimerG_startCounter(PWM_U5_LEFT_INST);
            }
            if( !DL_TimerG_isRunning(PWM_U6_RIGHT_INST) )
            {
                DL_TimerG_startCounter(PWM_U6_RIGHT_INST);
            }
            
            convert_crsf();
            set_drive_pwm();   
        //     DL_TimerG_setCaptureCompareValue(PWM_U5_LEFT_INST, controls.channel_01, DL_TIMER_CC_0_INDEX);
        //     DL_TimerG_setCaptureCompareValue(PWM_U6_RIGHT_INST, controls.channel_02, DL_TIMER_CC_0_INDEX);
        //     DL_TimerG_setCaptureCompareValue(PWM_U7_WEAPON_INST, controls.channel_04, DL_TIMER_CC_0_INDEX);
        }
        else
        {
            erls_time_out = 10;
            DL_TimerG_stopCounter(PWM_U7_WEAPON_INST);
            DL_TimerG_stopCounter(PWM_U5_LEFT_INST);
            DL_TimerG_stopCounter(PWM_U6_RIGHT_INST);
            DL_GPIO_clearPins(LED_PORT, LED_Red_PIN);
        }
    }
}

// 5ms Ticks
void SysTick_Handler(void)
{
    g_sys_tick++;
    erls_time_out++;
}

void UART_0_INST_IRQHandler(void)
{
    uint8_t rx_data;
    switch (DL_UART_Main_getPendingInterrupt(UART_0_INST)) {
        case DL_UART_MAIN_IIDX_RX:
            rx_data = DL_UART_Main_receiveDataBlocking(UART_0_INST);
            if(rx_data == 0xC8)
            {
                rx_ptr = &frame.sync;
            }
            if(rx_ptr == &frame.len)
            {
                rx_cnt = *rx_ptr;
            }
            *rx_ptr = rx_data;
            rx_ptr++;
            rx_cnt--;
            if(rx_cnt == 0)
            {
                frame_rdy = 1;        
                DL_GPIO_togglePins(LED_PORT, LED_Red_PIN);
            }
            break;
        default:
            break;
    }
}

// void UART0_IRQHandler(void)
// {
//     uint8_t rx_data;
//     rx_data = UART_0_INST->RXDATA;
//     if(rx_data == 0xC8)
//     {
//         rx_ptr = &frame.sync;
//     }
//     if(rx_ptr == &frame.len)
//     {
//         rx_cnt = *rx_ptr;
//     }
//     *rx_ptr = rx_data;
//     rx_ptr++;
//     rx_cnt--;
//     if(rx_cnt == 0)
//     {
//         frame_rdy = 1;        
//         DL_GPIO_togglePins(LED_PORT, LED_Red_PIN);
//     }
// }