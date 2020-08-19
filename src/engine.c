#include <ch.h> // For NULL
#include "engine.h"

typedef void (*modeFunctor)(normalized_inputs_t* inputs, normalized_outputs_t* outputs);

typedef enum generatormode{
    MODE_AUDIO_STEREO = 0,
    MODE_AUDIO_MONO_WAVEFORM,
    MODE_SPIRAL,
    MODE_RECTANGLE,
    MODE_FULL_SCAN,

    NUM_MODES
} GeneratorModeEnum;


const float REGION_SIZE = 1.0 /  (float)NUM_MODES;


typedef struct modemixt {
    GeneratorModeEnum mode_a;
    GeneratorModeEnum mode_b;
    float mix_ratio;
} mode_mix_t;

SetModeMix(mode_mix_t* mode_mix, float selection_point) {
    if (selection_point < REGION_SIZE) {
        mode_mix->mode_a = MODE_AUDIO_STEREO;
        mode_mix->mode_b = MODE_AUDIO_STEREO;
        mode_mix->mix_ratio = 0.0;
    } else if (selection_point < REGION_SIZE * 2.0) {
        mode_mix->mode_a = MODE_AUDIO_STEREO;
        mode_mix->mode_b = MODE_AUDIO_MONO_WAVEFORM;
        mode_mix->mix_ratio = (selection_point - REGION_SIZE) / REGION_SIZE;
    } else if (selection_point < REGION_SIZE * 3) {
        mode_mix->mode_a = MODE_AUDIO_MONO_WAVEFORM;
        mode_mix->mode_b = MODE_AUDIO_MONO_WAVEFORM;
        mode_mix->mix_ratio = 0;
    } else if (selection_point < REGION_SIZE * 4) {
        mode_mix->mode_a = MODE_AUDIO_MONO_WAVEFORM;
        mode_mix->mode_b = MODE_SPIRAL;
        mode_mix->mix_ratio = (selection_point - REGION_SIZE*4) / REGION_SIZE;
    } else if (selection_point < REGION_SIZE * 5) {
        mode_mix->mode_a = MODE_SPIRAL;
        mode_mix->mode_b = MODE_SPIRAL;
        mode_mix->mix_ratio = 0;
    } else if (selection_point < REGION_SIZE * 6) {
        mode_mix->mode_a = MODE_SPIRAL;
        mode_mix->mode_b = MODE_RECTANGLE;
        mode_mix->mix_ratio = (selection_point - REGION_SIZE*6) / REGION_SIZE;
    } else if (selection_point < REGION_SIZE * 7) {
        mode_mix->mode_a = MODE_RECTANGLE;
        mode_mix->mode_b = MODE_RECTANGLE;
        mode_mix->mix_ratio = 0;
    } else if (selection_point < REGION_SIZE * 8) {
        mode_mix->mode_a = MODE_RECTANGLE;
        mode_mix->mode_b = MODE_FULL_SCAN;
        mode_mix->mix_ratio = (selection_point - REGION_SIZE*8) / REGION_SIZE;
    } else {
        mode_mix->mode_a = MODE_FULL_SCAN;
        mode_mix->mode_b = MODE_FULL_SCAN;
        mode_mix->mix_ratio = 0;   
    }
}

//MODE_AUDIO_STEREO
void operator_mode_audio_stereo(normalized_inputs_t* inputs, normalized_outputs_t* outputs) {
    outputs->position_output_x = inputs->audio_in_left;
    outputs->position_output_y = inputs->audio_in_right;
    outputs->laser_pwm_output_r = inputs->cv_in_left;
    outputs->laser_pwm_output_g = 0;
    outputs->laser_pwm_output_b = inputs->cv_in_right;
}

// MODE_AUDIO_MONO_WAVEFORM
void operator_mode_audio_mono(normalized_inputs_t* inputs, normalized_outputs_t* outputs) {
    static float x_value = 0;
    outputs->position_output_x = inputs->audio_in_left;
    outputs->position_output_y = inputs->audio_in_right;
    outputs->laser_pwm_output_r = inputs->cv_in_left;
    outputs->laser_pwm_output_g = 0;
    outputs->laser_pwm_output_b = inputs->cv_in_right;
}

modeFunctor g_mode_functors[NUM_MODES] = {
    &operator_mode_audio_stereo
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
    result.laser_pwm_output_r = (uint16_t)(outputs->laser_pwm_output_r * (float)ADC_IN_MAX);
    result.laser_pwm_output_g = (uint16_t)(outputs->laser_pwm_output_g * (float)ADC_IN_MAX);
    result.laser_pwm_output_b = (uint16_t)(outputs->laser_pwm_output_b * (float)ADC_IN_MAX);
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


void RunEngine(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    const float generator_selection_point = (float)inputs->cv_in_middle / (float)ADC_IN_MAX;

    mode_mix_t mix;
    normalized_inputs_t norm_inputs = NormalizeInputs(inputs);
    normalized_outputs_t norm_outputs_a;
    normalized_outputs_t norm_outputs_b;

    SetModeMix(&mix, generator_selection_point);
    modeFunctor functor_a = g_mode_functors[(uint8_t)mix.mode_a];
    modeFunctor functor_b = g_mode_functors[(uint8_t)mix.mode_b];
    
    if (functor_a != NULL && functor_b != NULL) {
        functor_a(&norm_inputs, &norm_outputs_a);
        functor_b(&norm_inputs, &norm_outputs_b);
    }

    normalized_outputs_t mixed_outputs = MixNormedOutputs(&norm_outputs_a, &norm_outputs_b, mix.mix_ratio);

    *outputs = DenormalizeOutputs(&mixed_outputs);

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
}