/* 
 * File:   heater.h
 * Author: xlink
 *
 * Created on February 28, 2024, 8:40 PM
 */

#ifndef HEATER_H
#define	HEATER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define PID_CONTROL_RATE    (256-156)    // about 100 Hz : 16M/1024/156

void heater_pwm_init(int pwm_hz);
void heater_stop(void);
void heater_restart(void);
void heater_set(uint8_t percent);
void heater_clear_pid_values(void);
void heater_control(int16_t target_temperature, int16_t cur_temperature, 
                    int16_t pre_temperature, uint32_t gap_ms);
void heater_pid_control(int16_t target_temperature, int16_t cur_temperature, 
                        int16_t pre_temperature, uint32_t gap_ms);

#ifdef	__cplusplus
}
#endif

#endif	/* HEATER_H */

