#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "heater.h"
#include "sensor.h"

#define HEATER_OFF()        (PORTB &= ~_BV(PB1))
#define T1_PRESCALER_256    (_BV(CS12))
#define T1_PRESCALER_1024   (_BV(CS12) | _BV(CS10))

#define CH_A_DISCONNECT      0
#define CH_A_INVERT_MODE     (_BV(COM1A1) | _BV(COM1A0))
#define CH_A_NON_INVERT_MODE (_BV(COM1A1))

static uint8_t heater_run_flag;

void heater_stop(void)
{
    TCCR1A = CH_A_DISCONNECT;
    HEATER_OFF();
    heater_run_flag = 0;
}

void heater_restart(void)
{
    TCCR1A = CH_A_NON_INVERT_MODE | _BV(WGM11);
    heater_run_flag = 1;
}

void heater_set(uint8_t percent)
{
    static uint8_t pre_percent;
    uint16_t val;
    
    if(percent > 100 || percent < 0)
    {
        printf("Heater control value ERROR : %d\r\n", percent);
        percent = 0;
    }
    
    val = ICR1 / 100 * percent;
    
    printf("Heater set : %d %%, %d\r\n", percent, val);
    
    if(pre_percent == 0 && percent != 0)
    {
        heater_restart();
    }
    else if(pre_percent != 0 && percent == 0)
    {
        heater_stop();
    }
    
    OCR1A = val;
    
    pre_percent = percent;
}

uint32_t timestamp_10ms;
// interrupt 100 Hz
ISR (TIMER2_OVF_vect)
{
    TCNT2 = PID_CONTROL_RATE;   // 100 Hz interrupt
    
    timestamp_10ms++;
}

static void timer2_init(void)
{
	/* Timer clock = I/O clock / 1024 */
	TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
	/* Clear overflow flag */
	TIFR2  = _BV(TOV2);
	/* Enable Overflow Interrupt */
	TIMSK2 = _BV(TOIE2);
    
    TCNT2 = PID_CONTROL_RATE;   // 100 Hz interrupt
}

void heater_pwm_init(int pwm_hz) 
{
    // PWM OCR1A (PB1)
    DDRB |= _BV(PB1);

    // Fast PWM
    TCCR1A = CH_A_INVERT_MODE | _BV(WGM11);                 
    TCCR1B = _BV(WGM12) | _BV(WGM13) | T1_PRESCALER_256;      // 16 Bit, Prescaler 256Clock
    
    // Frequency = 1/16Mhz * Prescale * ICR1
    // 16M / 256 / 62500 = 1 Hz
    ICR1 = 62500 / pwm_hz;                            
    
    // Duty rate
    // Invert OCR1A(High) : ICR1-OCR1A(Low))
    // Not invert OCR1A(Low) : ICR1-OCR1A(High)
    
    heater_stop();
    timer2_init();
}

void heater_control(int16_t target_temperature, int16_t cur_temperature, 
                    int16_t pre_temperature, uint32_t gap_ms)
{
    int16_t temperature_diff = target_temperature - cur_temperature;
    
    if(gap_ms > 4200)
    {
        printf("PID gap time OVERFLOW : %ld", gap_ms);
        return;
    }
    
    printf("Temperature diff : %d\r\n", temperature_diff);
    if(temperature_diff > 0)
    {
        heater_set(100);
    }
    else if(temperature_diff < 0)
    {
        heater_set(0);
    }
    else
    {
        if(target_temperature < 250) heater_set(0);
        else if(target_temperature > 400) heater_set(60);
        else heater_set(((target_temperature / 10) - 25) * 4);
    }
}

#if 0
//PID constants
//////////////////////////////////////////////////////////
static float Kp = 70;   float Ki = 0.9;   float Kd = 50;
//////////////////////////////////////////////////////////

static float PID_p = 0;    float PID_i = 0;    float PID_d = 0;

void heater_clear_pid_values(void)
{
    PID_p = 0;
    PID_i = 0;
    PID_d = 0;
}

void heater_pid_control(int16_t target_temperature, int16_t cur_temperature, 
                        int16_t pre_temperature, uint32_t gap_ms)
{
    static uint8_t d_cal_count;
    static double d_sum_error, d_pre_error;
    static double previous_error;
    double elapsedTime = (double)gap_ms / 1000;   // Change second unit
    double PID_error, PID_value;
    
    if(gap_ms > 4200)
    {
        printf("PID gap time OVERFLOW : %ld", gap_ms);
        return;
    }
    
    PID_error = (double)(target_temperature - cur_temperature);
    
    PID_p = (Kp * PID_error) * elapsedTime;
    
    if(PID_error > 1 || PID_error < -1)
        PID_i = 0;
    else PID_i = PID_i + (Ki * PID_error) * elapsedTime;
    
    if(PID_i > 80) PID_i = 80;  // Limit Max.
    if(PID_i < -80) PID_i = -80;  // Limit Min.
    
//    PID_d = Kd * (PID_error - previous_error) / elapsedTime;
    d_sum_error += PID_error;
    if(++d_cal_count >= 3)
    {
        PID_d = Kd * (d_sum_error - d_pre_error) / elapsedTime;
        d_cal_count = 0;
        d_pre_error = d_sum_error;
        d_sum_error = 0;
    }

    PID_value = PID_p + PID_i + PID_d;
    
    printf("PID_p : %d, PID_i : %d, PID_d : %d, gap : %ld\r\n", (int)PID_p, (int)PID_i, (int)PID_d, gap_ms);

    if(PID_value < 0)
    {
        printf("Org PID_value : %d, set 0\r\n", (int)PID_value);
        PID_value = 0;
    }
    if(PID_value > 100)
    {
        printf("Org PID_value : %d, set 100\r\n", (int)PID_value);
        PID_value = 100;
    }
    
    heater_set((uint8_t)PID_value);
    
    previous_error = PID_error;
}
#endif

/*
void heater_task(void)
{
	float elapsedTime;
	float cur_temperature, PID_error, PID_value;

	if(run_heater_flag)
	{
		if((get_now_ms() - loop_ref_time) < HEATER_PID_LOOP_TIME) return;
		
		cur_temperature = ntc_get_temperature();
		INFO_MSG("cur_temperature : %3.1f", cur_temperature);

		// ?? second ??? ??
		loop_ref_time = get_now_ms();
		nowTime = loop_ref_time;
		elapsedTime = (float)(nowTime - timePrev) / 1000; 
		
		PID_error = target_temperature - cur_temperature;// + 1;

		PID_p = (Kp * PID_error) * elapsedTime;

		//Kwon TaeYoung 2022/02/20 : 1? ?? ???? ???? ???? ??
		if(PID_error > 1 || PID_error < -1) PID_i = 0;
		else PID_i = PID_i + (Ki * PID_error) * elapsedTime;
		
		PID_d = Kd * (PID_error - previous_error) / elapsedTime;

		PID_value = PID_p + PID_i + PID_d;

		INFO_MSG("PID_p : %f, PID_i : %f, PID_d : %f", PID_p, PID_i, PID_d);

		if(PID_value < 0)
		{
			INFO_MSG("Org PID_value : %f, set 0", PID_value);
			PID_value = 0;
		}
		if(PID_value > 100)
		{
			INFO_MSG("Org PID_value : %f, set 100", PID_value);
			PID_value = 100;
		}

		previous_error = PID_error;
		timePrev = nowTime;
		heater_set_power((int)PID_value);
	}
}
*/
