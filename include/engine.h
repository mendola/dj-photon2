#ifndef ENGINE_H_
#define ENGINE_H_

#include <stdint.h>

#define ADC_IN_MAX 4096
#define ADC_IN_MIDPOINT  (ADC_IN_MAX / 2)
#define AUDIO_IN_LEFT_MAX

typedef struct engineinputs {
    int16_t audio_in_left;
    int16_t audio_in_right;
    uint16_t cv_in_left;
    uint16_t cv_in_middle;
    uint16_t cv_in_right;
} engine_inputs_t;

typedef struct engineoutputs {
    uint16_t position_output_x;
    uint16_t position_output_y;
    uint16_t laser_pwm_output_r;
    uint16_t laser_pwm_output_g;
    uint16_t laser_pwm_output_b;
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
