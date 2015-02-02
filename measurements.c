
#include "measurements.h"
#include "communication.h"
#include "firmware.h"
#include "analog.h"
#include "time.h"

#define N_SAMPLES_BY_PERIOD 20
#define I_SAMPLES_BUFF_SIZE (N_SAMPLES_BY_PERIOD / 1)
#define N_PERIODS 1
#define N_SAMPLES (N_PERIODS * N_SAMPLES_BY_PERIOD)


volatile float I_samples_buffer[I_SAMPLES_BUFF_SIZE];
volatile uint16_t I_samples_count, V_samples_count;

volatile float I_rms_acc, V_rms_acc, power_acc;

volatile uint8_t measuring;


/* internal functions prototypes */
void measure_V_sample(void);
void measure_I_sample(void);
/***/

void measure_V_sample(void)
{
	//debug_to_pi("v_sample");
	float V_sample = analog_get_V_sample();
	uint16_t idx = V_samples_count % I_SAMPLES_BUFF_SIZE;

	V_rms_acc += V_sample * V_sample;
	power_acc += V_sample * I_samples_buffer[idx];

	debug_to_pi_uint16(V_samples_count);
	if (!V_samples_count)
	{
		time_set_V_period();
	}

	if (++V_samples_count == N_SAMPLES)
	{
		time_stop_V_timer();
		measuring = false;
	}
}

void measure_I_sample(void)
{
	//debug_to_pi("i_sample");
	uint16_t idx = I_samples_count % I_SAMPLES_BUFF_SIZE;

	I_samples_buffer[idx] = analog_get_I_sample();
	I_rms_acc += I_samples_buffer[idx] * I_samples_buffer[idx];

	if (++I_samples_count == N_SAMPLES)
	{
		time_stop_I_timer();
	}
}

void measure(circuit_t *circuit)
{
	measurement_packet_t packet;

	I_samples_count = 0; V_samples_count = 0;
	I_rms_acc = 0; V_rms_acc = 0; power_acc = 0;


	analog_config(circuit);
	debug_to_pi("config");

	measure_I_sample();
	debug_to_pi("1 sample");
	time_start_timers(circuit->delay);
	time_set_V_callback(measure_V_sample);
	time_set_I_callback(measure_I_sample);

	measuring = true;
	debug_to_pi("measuring");

	/* wait until all the samples are taken */
	while (measuring);

	debug_to_pi("end measuring");

	packet.circuit_id = circuit->circuit_id;

    //FIXME: Decide to use uint16 or float for this values.
	packet.real_power = power_acc / N_SAMPLES;
	packet.irms = I_rms_acc / N_SAMPLES;
	packet.vrms = V_rms_acc / N_SAMPLES;

	send_to_pi(&packet);
}
