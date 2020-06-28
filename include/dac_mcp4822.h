#ifndef DAC_MCP4822_H_
#define DAC_MCP4822_H_

#define DAC_OUT_MAX ((int16_t)0xFFF)

extern void DacThreadFn(void*);

void InitDac(void);

void TransmitSamples(const uint16_t ch1_out, const uint16_t ch2_out);

#endif  // DAC_MCP4822_H_