#ifndef LASER_PWM_H_
#define LASER_PWM_H_

#define PWM_FREQUENCY 100000
#define PWM_PERIOD 1000

void StartLaserPwm(void);
void SetLaserPwmLevels(uint16_t r, uint16_t g, uint16_t b);


#endif  // LASER_PWM_H_