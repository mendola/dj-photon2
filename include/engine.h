#ifndef ENGINE_H_
#define ENGINE_H_

#include <stdint.h>

#define LASER_POS_MAX 4095
#define LASER_MIDPOINT (LASER_POS_MAX/2)
#define ADC_IN_MAX 4095
#define ADC_IN_MIDPOINT  (ADC_IN_MAX / 2)
#define AUDIO_IN_LEFT_MAX

typedef struct engineinputs {
    int16_t audio_in_left;
    int16_t audio_in_right;
    int16_t cv_in_left;
    int16_t cv_in_middle;
    int16_t cv_in_right;
} engine_inputs_t;

typedef struct engineoutputs {
    int16_t position_output_x;
    int16_t position_output_y;
    int16_t laser_pwm_output_r;
    int16_t laser_pwm_output_g;
    int16_t laser_pwm_output_b;
} engine_outputs_t;

typedef struct normalized_inputs {
    float audio_in_left;
    float audio_in_right;
    float cv_in_left;
    float cv_in_middle;
    float cv_in_right;
} normalized_inputs_t;

typedef struct normalized_outputs {
    float position_output_x;
    float position_output_y;
    float laser_pwm_output_r;
    float laser_pwm_output_g;
    float laser_pwm_output_b;
} normalized_outputs_t;

void RunEngine(engine_inputs_t* inputs, engine_outputs_t* outputs);

#endif // ENGINE_H_
