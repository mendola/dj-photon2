#include <ch.h> // For NULL
#include "engine.h"
#include "math.h"

#define PI (3.14159265)
#define PI_OVER_2  (PI / 2.0)

#define COLORLINE_MAX  800
#define NUM_COLORS 8
#define COLORLINE_BIN_SIZE (COLORLINE_MAX / NUM_COLORS)

typedef void (*modeFunctor)(engine_inputs_t* inputs, engine_outputs_t* outputs);

typedef enum colorchannel {
    RED = 0,
    GREEN = 1,
    BLUE = 2
} colorchannel_t;

typedef enum generatormode{
    MODE_AUDIO_STEREO = 0,
    MODE_AUDIO_MONO_WAVEFORM,
    MODE_SPINNING_COIN,
    MODE_SPIRAL,
    MODE_MESSED_UP_SPIRAL,
    MODE_RECTANGLE,
    MODE_FULL_SCAN,

    NUM_MODES
} GeneratorModeEnum;

const int16_t REGION_SIZE = ADC_IN_MAX / NUM_MODES;


typedef struct modemixt {
    GeneratorModeEnum mode_a;
    GeneratorModeEnum mode_b;
    float mix_ratio;
} mode_mix_t;

void SetMonoColor(const colorchannel_t color, const int16_t value, engine_outputs_t* outputs) {
    switch (color) {
        case RED:
            outputs->laser_pwm_output_r = value;
            break;
        case GREEN:
            outputs->laser_pwm_output_g = value;
            break;
        case BLUE:
        default:
            outputs->laser_pwm_output_b = value;
            break;        
    }
}

void IntToColors(int16_t value, engine_outputs_t* outputs) {
    value = value < 0 ? 0 :
                        value > COLORLINE_MAX ? COLORLINE_MAX : value;
    int16_t bin = value / COLORLINE_BIN_SIZE + 1;

    switch (bin) {
        case 1:
            SetMonoColor(RED, 0, outputs);
            SetMonoColor(GREEN, 0, outputs);
            SetMonoColor(BLUE, 0, outputs);
            break;
        case 2:
            SetMonoColor(RED, 1, outputs);
            SetMonoColor(GREEN, 0, outputs);
            SetMonoColor(BLUE, 0, outputs);
            break;
        case 3:
            SetMonoColor(RED, 0, outputs);
            SetMonoColor(GREEN, 1, outputs);
            SetMonoColor(BLUE, 0, outputs);
            break;
        case 4:
            SetMonoColor(RED, 0, outputs);
            SetMonoColor(GREEN, 0, outputs);
            SetMonoColor(BLUE, 1, outputs);
            break;
        case 5:
            SetMonoColor(RED, 0, outputs);
            SetMonoColor(GREEN, 1, outputs);
            SetMonoColor(BLUE, 1, outputs);
            break;
        case 6:
            SetMonoColor(RED, 1, outputs);
            SetMonoColor(GREEN, 1, outputs);
            SetMonoColor(BLUE, 0, outputs);
            break;
        case 7:
            SetMonoColor(RED, 1, outputs);
            SetMonoColor(GREEN, 0, outputs);
            SetMonoColor(BLUE, 1, outputs);
            break;
        case 8:
        default:
            SetMonoColor(RED, 1, outputs);
            SetMonoColor(GREEN, 1, outputs);
            SetMonoColor(BLUE, 1, outputs);
            break;
    }
}

GeneratorModeEnum GetMode(int16_t selection_point_adc_val) {
    int16_t int_selection_point = selection_point_adc_val / (REGION_SIZE + 1);

    if (int_selection_point < 0 || int_selection_point >= NUM_MODES) {
        while (1) {};
    }
    return (GeneratorModeEnum)int_selection_point;

}

//MODE_AUDIO_STEREO
void operator_mode_audio_stereo(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    outputs->position_output_x = inputs->audio_in_left;
    outputs->position_output_y = inputs->audio_in_right;
    int16_t color = inputs->cv_in_right/5;
    IntToColors(color, outputs);
}

// MODE_AUDIO_MONO_WAVEFORM
void operator_mode_audio_mono(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    static int16_t x_value = 0;
    int16_t x_rate = inputs->cv_in_left;
    x_value += x_rate;
    x_value = x_value % LASER_POS_MAX;
    outputs->position_output_x = x_value;
    outputs->position_output_y = inputs->audio_in_right;

    int16_t color = inputs->cv_in_right/5;
    IntToColors(color, outputs);

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


    static int16_t color = 0;
    int16_t dcolor = inputs->cv_in_right/10;
    color += dcolor;
    color = color > COLORLINE_MAX ? 0 : color;
    IntToColors(color, outputs);

    outputs->position_output_x = x_val;
    outputs->position_output_y = y_val;
}

void operator_mode_spinning_coin(engine_inputs_t* inputs, engine_outputs_t* outputs) {
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


    static int16_t color = 0;
    int16_t dcolor = inputs->cv_in_right/10;
    color += dcolor;
    color = color > COLORLINE_MAX ? 0 : color;
    IntToColors(color, outputs);

    outputs->position_output_x = x_val;
    outputs->position_output_y = y_val;
}

void operator_mode_spiral(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    static int16_t x_val = 0;
    static int16_t y_val = 0;
    static float t = 0;
    static float amplitude = 0;
    const int16_t range_start = ADC_IN_MAX / NUM_MODES * (int16_t)MODE_RECTANGLE;

    const int dt = (float)(inputs->cv_in_middle - range_start) / 100;

    static float sign = 1.0;
    float d_amplitude = (float)inputs->cv_in_left / 100000; // Arbitrary denom

    amplitude += d_amplitude * sign;
    if (amplitude > 1.0) {
        sign = -1.0;
    } else if (amplitude < 0.0) {
        sign = 1.0;
    }
    t += dt;
    x_val = (int16_t)(sin(t) * (float)ADC_IN_MIDPOINT * amplitude) + ADC_IN_MIDPOINT;
    y_val = (int16_t)(cos(t) * (float)ADC_IN_MIDPOINT * amplitude) + ADC_IN_MIDPOINT;


    int16_t color = inputs->cv_in_right/5;

    IntToColors(color, outputs);

    outputs->position_output_x = x_val;
    outputs->position_output_y = y_val;
}

void operator_mode_rectangle(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    int16_t x_out = 0;
    int16_t y_out = 0;

    const int16_t range_start = ADC_IN_MAX / NUM_MODES * (int16_t)MODE_RECTANGLE;
    //const int16_t range_end = ADC_IN_MAX / NUM_COLORS * ((int16_t)MODE_RECTANGLE + 1);
    static int16_t t = 0;

    const int dt = (inputs->cv_in_middle - range_start);
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

    static int16_t color = 0;
    int16_t dcolor = inputs->cv_in_right/10;
    color += dcolor;
    color = color > COLORLINE_MAX ? 0 : color;
    IntToColors(color, outputs);

    outputs->position_output_x = x_out;
    outputs->position_output_y = y_out;
}

void operator_mode_full_scan(engine_inputs_t* inputs, engine_outputs_t* outputs) {
    static int16_t x_out = 0;
    static int16_t y_out = 0;

    static uint8_t downscale = 0;

    if (++downscale > 2) {
        downscale = 0;
        if (x_out == 0) {
            x_out = LASER_POS_MAX;
        } else {
            x_out = 0;
            y_out += 100;
            if (y_out >= LASER_POS_MAX) {
                y_out = 0;
            }
        }
    }

    static int16_t color = 0;
    int16_t dcolor = inputs->cv_in_right/10;
    color += dcolor;
    color = color > COLORLINE_MAX ? 0 : color;
    IntToColors(color, outputs);

    outputs->position_output_x = x_out;
    outputs->position_output_y = y_out;
}

modeFunctor g_mode_functors[NUM_MODES] = {
    &operator_mode_audio_stereo,
    &operator_mode_audio_mono,
    &operator_mode_spinning_coin,
    &operator_mode_spiral,
    &operator_mode_messed_up_spiral,
    &operator_mode_rectangle,
    &operator_mode_full_scan
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

    const GeneratorModeEnum mode = GetMode(inputs->cv_in_middle);

    modeFunctor functor = g_mode_functors[(uint8_t)mode];
    
    if (functor != NULL) {
        functor(inputs, outputs);
    }
}