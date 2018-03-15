#ifndef PROJECT_H__
#define PROJECT_H__

//#include "arm_math.h"
#include "nrf_delay.h"
#include "nrf_drv_pwm.h"
#include "nrf_drv_gpiote.h"
#include "nrf_gpiote.h"
#include "app_pwm.h"
#include "LedPatterns.h"
#include "ws2812b_drive.h"
#include "i2s_ws2812b_drive.h"
#include "nrf_drv_timer.h"
#include "drv_Buzzer.h"
#include "SongPatterns.h"

#define NUM_LEDS                    (3)     //  Count of LEDs
#define I2S_STDO_PIN    25

#endif // PROJECT_H
