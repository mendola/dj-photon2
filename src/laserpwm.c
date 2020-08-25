#include <ch.h>
#include <hal.h>

#include "laserpwm.h"

#define LASER_CHANNEL_R 0
#define LASER_CHANNEL_G 1
#define LASER_CHANNEL_B 2

static void lasers_on(PWMDriver* driver) {
    (void)driver;
    palSetPad(GPIOB, 3);
    palSetPad(GPIOB, 6);
    palSetPad(GPIOB, 9);
}

static void red_laser_off(PWMDriver* driver) {
    (void)driver;
    palClearPad(GPIOB, 3);
}

static void green_laser_off(PWMDriver* driver) {
    (void)driver;
    palClearPad(GPIOB, 6);
}

static void blue_laser_off(PWMDriver* driver) {
    (void)driver;
    palClearPad(GPIOB, 9);
}


static PWMConfig pwm_config = {
  PWM_FREQUENCY,                                    /* 10kHz PWM clock frequency.   */
  PWM_PERIOD,                                    /* Initial PWM period 1S.       */
  lasers_on,
  {
   {PWM_OUTPUT_ACTIVE_LOW, red_laser_off}, // LASER_CHANNEL_R
   {PWM_OUTPUT_ACTIVE_LOW, green_laser_off}, // LASER_CHANNEL_G
   {PWM_OUTPUT_ACTIVE_LOW, blue_laser_off},  // LASER_CHANNEL_B
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0,
#if STM32_PWM_USE_ADVANCED
  0
#endif
};


void StartLaserPwm(void) {
    pwmStart(&PWMD1, &pwm_config);
    pwmEnablePeriodicNotification(&PWMD1);
    pwmEnableChannel(&PWMD1, LASER_CHANNEL_R, 0);
    pwmEnableChannel(&PWMD1, LASER_CHANNEL_G, 0);
    pwmEnableChannel(&PWMD1, LASER_CHANNEL_B, 0); // PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 7500));

    pwmEnableChannelNotification(&PWMD1, LASER_CHANNEL_R);
    pwmEnableChannelNotification(&PWMD1, LASER_CHANNEL_B);
    pwmEnableChannelNotification(&PWMD1, LASER_CHANNEL_B);
}

void SetLaserPwmLevels(uint16_t r, uint16_t g, uint16_t b) {
    r = r > PWM_PERIOD ? PWM_PERIOD : r;
    g = g > PWM_PERIOD ? PWM_PERIOD : g;
    b = b > PWM_PERIOD ? PWM_PERIOD : b;

    pwmEnableChannel(&PWMD1, LASER_CHANNEL_R, (uint32_t)r);
    pwmEnableChannel(&PWMD1, LASER_CHANNEL_G, (uint32_t)g);
    pwmEnableChannel(&PWMD1, LASER_CHANNEL_B, (uint32_t)b); // PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 7500));
}