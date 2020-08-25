#include <ch.h> // For NULL
#include "engine.h"
#include "math.h"
#include "laserpwm.h"

#define PI (3.14159265)
#define PI_OVER_2  (PI / 2.0)

typedef void (*modeFunctor)(engine_inputs_t* inputs, engine_outputs_t* outputs);

typedef enum generatormode{
    MODE_AUDIO_STEREO = 0,
    MODE_AUDIO_MONO_WAVEFORM,
    MODE_SPINNING_COIN,
    MODE_MESSED_UP_SPIRAL,
    MODE_RECTANGLE,
    // MODE_SPIRAL
    // MODE_FULL_SCAN,

    NUM_MODES
} GeneratorModeEnum;

const float REGION_SIZE = 1.0 /  (float)NUM_MODES;


typedef struct modemixt {
    GeneratorModeEnum mode_a;
    GeneratorModeEnum mode_b;
    float mix_ratio;
} mode_mix_t;

void SetModeMix(mode_mix_t* mode_mix, float selection_point) {
    if (selection_point < REGION_SIZE) {
        mode_mix->mode_a = MODE_AUDIO_STEREO;
        mode_mix->mode_b = MODE_AUDIO_STEREO;
        mode_mix->mix_ratio = 0.0;
    } else if (selection_point < REGION_SIZE * 2.0) {
        mode_mix->mode_a = MODE_AUDIO_STEREO;
        mode_mix->mode_b = MODE_AUDIO_MONO_WAVEFORM;
        mode_mix->mix_ratio = 1 - (selection_point - REGION_SIZE) / REGION_SIZE;
    } else if (selection_point < REGION_SIZE * 3) {
        mode_mix->mode_a = MODE_AUDIO_MONO_WAVEFORM;
        mode_mix->mode_b = MODE_AUDIO_MONO_WAVEFORM;
        mode_mix->mix_ratio = 0;
    } else if (selection_point < REGION_SIZE * 4) {
        mode_mix->mode_a = MODE_AUDIO_MONO_WAVEFORM;
        mode_mix->mode_b = MODE_SPINNING_COIN;
        mode_mix->mix_ratio = 1 - (selection_point - REGION_SIZE*3) / REGION_SIZE;
    } else if (selection_point < REGION_SIZE * 5){//if (selection_point < REGION_SIZE * 5) {
        mode_mix->mode_a = MODE_SPINNING_COIN;
        mode_mix->mode_b = MODE_SPINNING_COIN;
        mode_mix->mix_ratio = 0;
    } else if (selection_point < REGION_SIZE * 6) {
        mode_mix->mode_a = MODE_SPINNING_COIN;
        mode_mix->mode_b = MODE_MESSED_UP_SPIRAL;
        mode_mix->mix_ratio = 1 - (selection_point - REGION_SIZE*5) / REGION_SIZE;
    } else if (selection_point < REGION_SIZE * 7) {
        mode_mix->mode_a = MODE_MESSED_UP_SPIRAL;
        mode_mix->mode_b = MODE_MESSED_UP_SPIRAL;
        mode_mix->mix_ratio = 0;
    } else if (selection_point < REGION_SIZE * 8) {
        mode_mix->mode_a = MODE_MESSED_UP_SPIRAL;
        mode_mix->mode_b = MODE_RECTANGLE;
        mode_mix->mix_ratio = 1 - (selection_point - REGION_SIZE*7) / REGION_SIZE;
    } else {//if (selection_point < REGION_SIZE * 7) {
        mode_mix->mode_a = MODE_RECTANGLE;
        mode_mix->mode_b = MODE_RECTANGLE;
        mode_mix->mix_ratio = 0;
    }
//     } else if (selection_point < REGION_SIZE * 8) {
//         mode_mix->mode_a = MODE_RECTANGLE;
//         mode_mix->mode_b = MODE_FULL_SCAN;
//         mode_mix->mix_ratio = (selection_point - REGION_SIZE*8) / REGION_SIZE;
//     } else {
//         mode_mix->mode_a = MODE_FULL_SCAN;
//         mode_mix->mode_b = MODE_FULL_SCAN;
//         mode_mix->mix_ratio = 0;   
//     }
}

//MODE_AUDIO_STEREO
void operator_mode_audio_stereo(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    outputs->position_output_x = inputs->audio_in_left;
    outputs->position_output_y = inputs->audio_in_right;
    outputs->laser_pwm_output_r = 1;
    outputs->laser_pwm_output_g = 1;
    outputs->laser_pwm_output_b = 1;
}

// MODE_AUDIO_MONO_WAVEFORM
void operator_mode_audio_mono(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    static int16_t x_value = 0;
    int16_t x_rate = inputs->cv_in_left;
    x_value += x_rate;
    x_value = x_value % LASER_POS_MAX;
    outputs->position_output_x = x_value;
    outputs->position_output_y = inputs->audio_in_right;
    outputs->laser_pwm_output_r = 1;
    outputs->laser_pwm_output_g = 1;
    outputs->laser_pwm_output_b = 1;
}

// MODE_SPINNING_COIN
void operator_mode_messed_up_spiral(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    static int16_t x_val = 0;
    static int16_t y_val = 0;
    static float t = 0;
    static float amplitude = 0;

    float dt = (float)inputs->cv_in_left / 10000;
    float d_amplitude = (float)inputs->cv_in_right / 100000; // Arbitrary denom

    amplitude += d_amplitude;
    if (amplitude > 1.0) {
        amplitude = 0.0;
    }
    t += dt;
    x_val = (int16_t)(sin(t) * LASER_POS_MAX * amplitude) + LASER_POS_MAX;
    y_val = (int16_t)(sin(t + PI_OVER_2) * LASER_POS_MAX) + LASER_POS_MAX;

    outputs->position_output_x = x_val;
    outputs->position_output_y = y_val;
    outputs->laser_pwm_output_r = 1;
    outputs->laser_pwm_output_g = 1;
    outputs->laser_pwm_output_b = 1;
}

void operator_mode_spiral(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    static int16_t x_val = 0;
    static int16_t y_val = 0;
    static float t = 0;
    static float amplitude = 0;

    float dt = (float)inputs->cv_in_left / 10000;
    static float sign = 1.0;
    float d_amplitude = (float)inputs->cv_in_right / 100000; // Arbitrary denom

    amplitude += d_amplitude * sign;
    if (amplitude > 1.0) {
        sign = -1.0;
    } else if (amplitude < 0.0) {
        sign = 1.0;
    }
    t += dt;
    x_val = (int16_t)(sin(t) * (float)ADC_IN_MIDPOINT * amplitude) + ADC_IN_MIDPOINT;
    y_val = (int16_t)(cos(t) * (float)ADC_IN_MIDPOINT) + ADC_IN_MIDPOINT;

    outputs->position_output_x = x_val;
    outputs->position_output_y = y_val;
    outputs->laser_pwm_output_r = 1;
    outputs->laser_pwm_output_g = 1;
    outputs->laser_pwm_output_b = 1;
}


void operator_mode_rectangle(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    int16_t x_out = 0;
    int16_t y_out = 0;

    static int16_t t = 0;
    int16_t dt = inputs->cv_in_right/10;
    int16_t width = inputs->cv_in_left;
    const int16_t halfwidth = width/2;

    t += dt;
    if (t >= 4*width) {
        t = 0;
    }
    if (t < width) {
        x_out = LASER_MIDPOINT + t - halfwidth; // MID - half, MID + half
        y_out = LASER_MIDPOINT - halfwidth; // MID - half
    } else if (t < 2*width) {
        x_out = LASER_MIDPOINT + halfwidth; // MID + half
        y_out = LASER_MIDPOINT + t - width - halfwidth; // MID - half, MID + half
    } else if (t < 3*width) {
        x_out = LASER_MIDPOINT - (t - 2*width - halfwidth); // MID
        y_out = LASER_MIDPOINT + halfwidth;
    } else {//(t < 4LASER_POS_MAX) {
        x_out = LASER_MIDPOINT - halfwidth;
        y_out = LASER_MIDPOINT - (t - 3*width - halfwidth);
    }

    static uint8_t color = 0;
    color = (color + 1) % PWM_PERIOD;

    outputs->position_output_x = x_out;
    outputs->position_output_y = y_out;
    outputs->laser_pwm_output_r = color;
    outputs->laser_pwm_output_g = color;
    outputs->laser_pwm_output_b = color;
}

modeFunctor g_mode_functors[NUM_MODES] = {
    &operator_mode_audio_stereo,
    &operator_mode_audio_mono,
    &operator_mode_spiral,
    &operator_mode_messed_up_spiral,
    &operator_mode_rectangle
};

normalized_inputs_t NormalizeInputs(engine_inputs_t* inputs) {
    normalized_inputs_t result;
    result.audio_in_left = (float)(inputs->audio_in_left - ADC_IN_MIDPOINT) / (float)ADC_IN_MAX;
    result.audio_in_right = (float)(inputs->audio_in_right - ADC_IN_MIDPOINT) / (float)ADC_IN_MAX;
    result.cv_in_left = (float)inputs->cv_in_left / (float)ADC_IN_MAX;
    result.cv_in_middle = (float)inputs->cv_in_middle / (float)ADC_IN_MAX;
    result.cv_in_right = (float)inputs->cv_in_right / (float)ADC_IN_MAX;
    return result;
}

engine_outputs_t DenormalizeOutputs(normalized_outputs_t* outputs) {
    engine_outputs_t result;
    result.position_output_x = (int16_t)(outputs->position_output_x *  (float)ADC_IN_MAX) + ADC_IN_MIDPOINT;
    result.position_output_x = (int16_t)(outputs->position_output_x *  (float)ADC_IN_MAX) + ADC_IN_MIDPOINT;
    result.laser_pwm_output_r = (int16_t)(outputs->laser_pwm_output_r * (float)ADC_IN_MAX);
    result.laser_pwm_output_g = (int16_t)(outputs->laser_pwm_output_g * (float)ADC_IN_MAX);
    result.laser_pwm_output_b = (int16_t)(outputs->laser_pwm_output_b * (float)ADC_IN_MAX);
    return result;
}

// engine_inputs_t DeormalizeOutputs(normalized_inputs_t* inputs) {
//     engine_inputs_t result;
//     result.audio_in_left = (int16_t)(inputs->audio_in_left *  (float)ADC_IN_MAX) + ADC_IN_MIDPOINT;
//     result.audio_in_right = (int16_t)(inputs->audio_in_right *  (float)ADC_IN_MAX) + ADC_IN_MIDPOINT;
//     result.cv_in_left = (uint16_t)(inputs->cv_in_left * (float)ADC_IN_MAX);
//     result.cv_in_middle = (uint16_t)(inputs->cv_in_middle * (float)ADC_IN_MAX);
//     result.cv_in_right = (uint16_t)(inputs->cv_in_right * (float)ADC_IN_MAX);
//     return result;
// }

normalized_outputs_t MixNormedOutputs(normalized_outputs_t* out_a, normalized_outputs_t* out_b, float ratio) {
    normalized_outputs_t result;
    const float ratio_complement = 1.0 - ratio;
    result.position_output_x = out_a->position_output_x*ratio + out_b->position_output_x*ratio_complement;
    result.position_output_y = out_a->position_output_y*ratio + out_b->position_output_y*ratio_complement;
    result.laser_pwm_output_r = out_a->laser_pwm_output_r*ratio + out_b->laser_pwm_output_r*ratio_complement;
    result.laser_pwm_output_g = out_a->laser_pwm_output_g*ratio + out_b->laser_pwm_output_g*ratio_complement;
    result.laser_pwm_output_b = out_a->laser_pwm_output_b*ratio + out_b->laser_pwm_output_b*ratio_complement;
    return result;
}

engine_outputs_t MixEngineOutputs(engine_outputs_t* out_a, engine_outputs_t* out_b, float ratio) {
    engine_outputs_t result;
    const float ratio_complement = 1.0 - ratio;
    result.position_output_x = (int16_t)((float)out_a->position_output_x*ratio + (float)out_b->position_output_x*ratio_complement);
    result.position_output_y = (int16_t)((float)out_a->position_output_y*ratio + (float)out_b->position_output_y*ratio_complement);
    result.laser_pwm_output_r = (int16_t)((float)out_a->laser_pwm_output_r*ratio + (float)out_b->laser_pwm_output_r*ratio_complement);
    result.laser_pwm_output_g = (int16_t)((float)out_a->laser_pwm_output_g*ratio + (float)out_b->laser_pwm_output_g*ratio_complement);
    result.laser_pwm_output_b = (int16_t)((float)out_a->laser_pwm_output_b*ratio + (float)out_b->laser_pwm_output_b*ratio_complement);
    return result;
}

void RunEngine(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    const float generator_selection_point = (float)inputs->cv_in_middle / (float)2048;

    mode_mix_t mix;
    // normalized_inputs_t norm_inputs = NormalizeInputs(inputs);
    // normalized_outputs_t norm_outputs_a;
    // normalized_outputs_t norm_outputs_b;
    engine_outputs_t outputs_a;
    engine_outputs_t outputs_b;
    SetModeMix(&mix, generator_selection_point);
    modeFunctor functor_a = g_mode_functors[(uint8_t)mix.mode_a];
    modeFunctor functor_b = g_mode_functors[(uint8_t)mix.mode_b];
    
    if (functor_a != NULL && functor_b != NULL) {
        functor_a(inputs, &outputs_a);
        functor_b(inputs, &outputs_b);
    }

    *outputs = MixEngineOutputs(&outputs_a, &outputs_b, mix.mix_ratio);
}