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
#include "engine.h"

#define ADC_GRP1_NUM_CHANNELS   1
#define ADC_GRP1_BUF_DEPTH      8

#define ADC_GRP2_NUM_CHANNELS   8
#define ADC_GRP2_BUF_DEPTH      16

#define ADC_GROUP_NUM_CHANNELS   5
#define ADC_GROUP_BUF_DEPTH      1

typedef enum adcindext {
  BUF_IDX_CV_INPUT_L = 0,
  BUF_IDX_CV_INPUT_C = 1,
  BUF_IDX_CV_INPUT_R = 2,
  BUF_IDX_AUDIO_INPUT_L = 3,
  BUF_IDX_AUDIO_INPUT_R = 4
} adc_index_t;

// static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
// static adcsample_t samples2[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];
static adcsample_t g_adc_samples_buf[ADC_GROUP_NUM_CHANNELS * ADC_GROUP_BUF_DEPTH];

void GetSamples(engine_inputs_t* samples_in, adcsample_t* sample_buf) {
  samples_in->audio_in_left = sample_buf[BUF_IDX_AUDIO_INPUT_L];
  samples_in->audio_in_right = sample_buf[BUF_IDX_AUDIO_INPUT_R];
  samples_in->cv_in_left = sample_buf[BUF_IDX_CV_INPUT_L];
  samples_in->cv_in_middle = sample_buf[BUF_IDX_CV_INPUT_C];
  samples_in->cv_in_right = sample_buf[BUF_IDX_CV_INPUT_R];
}

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

void SetupPins(void) {
 // Sets up DAC pins
  InitDac();

  // Inputs
  palSetPadMode(GPIOA, 3, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 4, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 5, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 6, PAL_MODE_INPUT_ANALOG);
  palSetPadMode(GPIOA, 7, PAL_MODE_INPUT_ANALOG);

  // Outputs
  palSetPadMode(GPIOB, 3, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 6, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOB, 9, PAL_MODE_OUTPUT_PUSHPULL);
}


void SetLaserPwm(const int16_t pwm_output_r, const int16_t pwm_output_g, const int16_t pwm_output_b) {
  // if (pwm_output_r > 0) {
  //   palSetPad(GPIOB, 3);
  // } else {
  //   palClearPad(GPIOB, 3);
  // }

  // if (pwm_output_g > 0) {
  //   palSetPad(GPIOB, 6);
  // } else {
  //   palClearPad(GPIOB, 6);
  // }

  // if (pwm_output_b > 0) {
  //   palSetPad(GPIOB, 9);
  // } else {
  //   palClearPad(GPIOB, 9);
  // }
    palSetPad(GPIOB, 3);
    palSetPad(GPIOB, 6);
    palSetPad(GPIOB, 9);

}


void SetLaserOutputs(engine_outputs_t* engine_outputs) {
  TransmitSamples(engine_outputs->position_output_x, engine_outputs->position_output_y);
  SetLaserPwm(engine_outputs->laser_pwm_output_r, engine_outputs->laser_pwm_output_g, engine_outputs->laser_pwm_output_b);
}

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

  SetupPins();


  adcStart(&ADCD1, NULL);

  /*
   * Starts an ADC continuous conversion.
   */
  adcConvert(&ADCD1, &g_adc_grp_config, g_adc_samples_buf, ADC_GROUP_BUF_DEPTH);

  /*
   * Sleep 2s before disabling Serial JTAG (so there's a window to program it)
   */
  chThdSleepMilliseconds(2000);

  // GPIO PB3 is by default used for SWD output. Turn it off so we can use PB3 as GPIO.
  AFIO->MAPR &= ~AFIO_MAPR_SWJ_CFG_Msk;
  AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_DISABLE;

  palSetPad(GPIOB, 3);
  palSetPad(GPIOB, 6);
  palSetPad(GPIOB, 9);

  while (true) {
    engine_inputs_t inputs;
    engine_outputs_t outputs;
    adcConvert(&ADCD1, &g_adc_grp_config, g_adc_samples_buf, ADC_GROUP_BUF_DEPTH);
    GetSamples(&inputs, g_adc_samples_buf);
    RunEngine(&inputs, &outputs);
    SetLaserOutputs(&outputs);    
  }
  return 0;
}
