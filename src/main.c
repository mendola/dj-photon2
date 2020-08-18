/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
#include <math.h>

#include "ch.h"
#include "hal.h"

#include "dac_mcp4822.h"
#define ADC_GRP1_NUM_CHANNELS   1
#define ADC_GRP1_BUF_DEPTH      8

#define ADC_GRP2_NUM_CHANNELS   8
#define ADC_GRP2_BUF_DEPTH      16

#define ADC_GROUP_NUM_CHANNELS   5
#define ADC_GROUP_BUF_DEPTH      1

// static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
// static adcsample_t samples2[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
static adcsample_t g_adc_samples_buf[ADC_GROUP_NUM_CHANNELS * ADC_GROUP_BUF_DEPTH];


/*
 * ADC conversion group.
 * Mode:        Continuous, 1 sample of 6 channels, SW triggered.
 * Channels:    IN0 - IN5
 */
static const ADCConversionGroup g_adc_grp_config = {
  FALSE,                                 /* circular */
  ADC_GROUP_NUM_CHANNELS,              /* num_channels */
  NULL,                                /* end_cb   */
  NULL,                                /* error_cb */
  0, 0,                                /* CR1, CR2  */
  0,                                   /* SMPR1 (ch 10-17) */
  ADC_SMPR2_SMP_AN3(ADC_SAMPLE_41P5)   /* SMPR2*/
    | ADC_SMPR2_SMP_AN4(ADC_SAMPLE_41P5)
    | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_41P5)
    | ADC_SMPR2_SMP_AN6(ADC_SAMPLE_41P5)
    | ADC_SMPR2_SMP_AN7(ADC_SAMPLE_41P5),
  ADC_SQR1_NUM_CH(ADC_GROUP_NUM_CHANNELS), /* sqr1 sequence steps 13-16, seq len */
  0,                                       /* sqr2 sequence steps 7-12 */
  ADC_SQR3_SQ1_N(ADC_CHANNEL_IN3)         /* sqr3 sequence steps 1-6  */
    | ADC_SQR3_SQ2_N(ADC_CHANNEL_IN4)
    | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN5)
    | ADC_SQR3_SQ4_N(ADC_CHANNEL_IN6)
    | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN7)
};

/*
 * Red LEDs blinker thread, times are in milliseconds.
 */
// static THD_WORKING_AREA(waThread1, 128);
// static THD_FUNCTION(Thread1, arg) {

//   (void)arg;

//   chRegSetThreadName("blinker");
//   while (true) {
//     palClearPad(GPIOC, GPIOC_LED);
//     chThdSleepMilliseconds(500);
//     palSetPad(GPIOC, GPIOC_LED);
//     chThdSleepMilliseconds(1000);
//   }
// }

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();
  InitDac();
  palSetPadMode(GPIOA, 3, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 5, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);

  palSetPadMode(GPIOB, 3, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 6, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 9, PAL_MODE_OUTPUT_PUSHPULL);

  adcStart(&ADCD1, NULL);

  /*
   * Setting up analog inputs used by the demo.
   */
  // palSetGroupMode(GPIOC, PAL_PORT_BIT(0) | PAL_PORT_BIT(1),
  //                 0, PAL_MODE_INPUT_ANALOG);

  /*
   * Creates the blinker thread.
   */
  //chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Activates the ADC1 driver and the temperature sensor.
   */

  /*
   * Linear conversion.
   */
  //adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
  //chThdSleepMilliseconds(1000);

  /*
   * Starts an ADC continuous conversion.
   */
  adcConvert(&ADCD1, &g_adc_grp_config, g_adc_samples_buf, ADC_GROUP_BUF_DEPTH);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  int16_t ch1_val = 0;
  int16_t ch2_val = 0;
  chThdSleepMilliseconds(2000);
  palSetPad(GPIOB, 3);
  palSetPad(GPIOB, 6);
  palSetPad(GPIOB, 9);

  while (true) {
    adcConvert(&ADCD1, &g_adc_grp_config, g_adc_samples_buf, ADC_GROUP_BUF_DEPTH);
    uint16_t left = g_adc_samples_buf[3];
    uint16_t right = g_adc_samples_buf[4];
    TransmitSamples(left, right);
}
  // double x = 0.0;
  // #define PI (3.14159265)
  // #define PI_OVER_2  (PI / 2.0)
  // #define OFFSET  (DAC_OUT_MAX / 2)
  // double dx = PI / 50;

  // double amplitude = 0;
  // double d_amplitude = 0.001;
  // while (true) {
  //   amplitude += d_amplitude;
  //   if (amplitude > 1.0) {
  //     amplitude = 0;
  //   }
  //   x += dx;
  //   ch1_val = (int16_t)(sin(x) * OFFSET * amplitude) + OFFSET;
  //   ch2_val = (int16_t)(sin(x + PI_OVER_2) * OFFSET) + OFFSET;
  //   TransmitSamples((uint16_t)ch1_val, (uint16_t)ch2_val);
  // }
  return 0;
}
