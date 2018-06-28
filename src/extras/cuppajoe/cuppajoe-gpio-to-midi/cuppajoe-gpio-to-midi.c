
/*
 * This is the lowest software layer of Cuppajoe, responsible for converting
 * from Raspberry Pi GPIO to MIDI.
 *
 * Matrix logic adapted from public domain example code by James "Bald
 * Engineer" Lewis.
 *
 * Debounce logic adapted from public domain example code by Kenneth A. Kuhn.
 *
 * MCP3008 ADC SPI logic adapted from public domain example code by Limor
 * "Ladyada" Fried.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <wiringPi.h>

#define NUMBER_OF_BUTTON_ROWS 4
#define NUMBER_OF_BUTTON_COLUMNS 4
int button_row_gpio_pin_numbers[] = {0, 1, 2, 3};
int button_column_gpio_pin_numbers[] = {4, 5, 6, 7};
#define NUMBER_OF_ANALOG_INPUTS 4
#define SPI_MOSI_GPIO_PIN_NUMBER 21
#define SPI_MISO_GPIO_PIN_NUMBER 22
#define SPI_CLK_GPIO_PIN_NUMBER 23
#define SPI_CS_GPIO_PIN_NUMBER 24
#define SAMPLE_FREQUENCY_HZ 10
#define BUTTON_DEBOUNCE_TIME_SECS 0.3
#define MIDI_CLIENT_NAME "Cuppajoe GPIO to MIDI"
#define MIDI_PORT_NAME "out"
#define MIDI_CHANNEL 0
#define MIDI_BASE_NOTE 64
#define MIDI_BASE_CONTROLLER_NUMBER 16

#define NUMBER_OF_BUTTONS (NUMBER_OF_BUTTON_ROWS * NUMBER_OF_BUTTON_COLUMNS)
#define SAMPLE_FREQUENCY_MSECS (1000 / SAMPLE_FREQUENCY_HZ)
#define BUTTON_DEBOUNCE_INTEGRATOR_MAX (SAMPLE_FREQUENCY_HZ * BUTTON_DEBOUNCE_TIME_SECS)
int button_debounce_integrator[NUMBER_OF_BUTTONS];
int current_button_state[NUMBER_OF_BUTTONS];
int current_analog_input_values[NUMBER_OF_ANALOG_INPUTS];
snd_seq_t *midi_client;
int midi_port;

void handle_analog_input_change(int analog_input_number, int value)
{
	snd_seq_event_t midi_event;
	snd_seq_ev_clear(&midi_event);
	snd_seq_ev_set_source(&midi_event, midi_port);
	snd_seq_ev_set_subs(&midi_event);
	snd_seq_ev_set_controller(&midi_event, MIDI_CHANNEL, MIDI_BASE_CONTROLLER_NUMBER + analog_input_number, value);
	snd_seq_event_output_direct(midi_client, &midi_event);
}

void handle_raw_analog_input_sample(int analog_input_number, int ten_bit_value)
{
	int seven_bit_value = ten_bit_value >> 3; /* reduce from ADC resolution to MIDI controller resolution; also helps reduce jitter */

	if (current_analog_input_values[analog_input_number] != seven_bit_value)
	{
		current_analog_input_values[analog_input_number] = seven_bit_value;
		handle_analog_input_change(analog_input_number, seven_bit_value);
	}
}

void sample_analog_input(int analog_input_number)
{
	int request;
	int response;

	digitalWrite(SPI_CS_GPIO_PIN_NUMBER, HIGH);
	digitalWrite(SPI_CLK_GPIO_PIN_NUMBER, LOW);
	digitalWrite(SPI_CS_GPIO_PIN_NUMBER, LOW);

	request = analog_input_number;
	request |= 0x18; /* start bit + single-ended bit */
	request <<= 3; /* we only need to send 5 bits here */

	for (int request_bit_number = 0; request_bit_number < 5; request_bit_number++)
	{
		if (request & 0x80)
		{
			digitalWrite(SPI_MOSI_GPIO_PIN_NUMBER, HIGH);
		}
		else
		{
			digitalWrite(SPI_MOSI_GPIO_PIN_NUMBER, LOW);
		}

		request <<= 1;

		digitalWrite(SPI_CLK_GPIO_PIN_NUMBER, HIGH);
		digitalWrite(SPI_CLK_GPIO_PIN_NUMBER, LOW);
	}

	/* read in one empty bit, one null bit, and 10 ADC bits */

	response = 0;

	for (int response_bit_number = 0; response_bit_number < 12; response_bit_number++)
	{
		digitalWrite(SPI_CLK_GPIO_PIN_NUMBER, HIGH);
		digitalWrite(SPI_CLK_GPIO_PIN_NUMBER, LOW);

		response <<= 1;

		if (digitalRead(SPI_MISO_GPIO_PIN_NUMBER))
		{
			response |= 1;
		}
	}

	digitalWrite(SPI_CS_GPIO_PIN_NUMBER, HIGH);

	response >>= 1; /* first bit is "null" so drop it */
	handle_raw_analog_input_sample(analog_input_number, response);
}

void sample_analog_inputs()
{
	for (int analog_input_number = 0; analog_input_number < NUMBER_OF_ANALOG_INPUTS; analog_input_number++)
	{
		sample_analog_input(analog_input_number);
	}
}

void handle_button_state_change(int button_number, int pressed)
{
	snd_seq_event_t midi_event;
	snd_seq_ev_clear(&midi_event);
	snd_seq_ev_set_source(&midi_event, midi_port);
	snd_seq_ev_set_subs(&midi_event);

	if (pressed)
	{
		snd_seq_ev_set_noteon(&midi_event, MIDI_CHANNEL, MIDI_BASE_NOTE + button_number, 127);
	}
	else
	{
		snd_seq_ev_set_noteoff(&midi_event, MIDI_CHANNEL, MIDI_BASE_NOTE + button_number, 0);
	}

	snd_seq_event_output_direct(midi_client, &midi_event);
}

void handle_debounced_button_sample(int button_number, int pressed)
{
	if (current_button_state[button_number] != pressed)
	{
		current_button_state[button_number] = pressed;
		handle_button_state_change(button_number, pressed);
	}
}

void handle_raw_button_sample(int button_number, int pressed)
{
	if (pressed)
	{
		if (button_debounce_integrator[button_number] < BUTTON_DEBOUNCE_INTEGRATOR_MAX)
		{
			button_debounce_integrator[button_number]++;
		}
	}
	else
	{
		if (button_debounce_integrator[button_number] > 0)
		{
			button_debounce_integrator[button_number]--;
		}
	}

	if (button_debounce_integrator[button_number] == BUTTON_DEBOUNCE_INTEGRATOR_MAX)
	{
		handle_debounced_button_sample(button_number, 1);
	}
	else if (button_debounce_integrator[button_number] == 0)
	{
		handle_debounced_button_sample(button_number, 0);
	}
}

void sample_button_matrix(void)
{
	for (int button_column_number = 0; button_column_number < NUMBER_OF_BUTTON_COLUMNS; button_column_number++)
	{
		pinMode(button_column_gpio_pin_numbers[button_column_number], OUTPUT);
		digitalWrite(button_column_gpio_pin_numbers[button_column_number], LOW);

		for (int button_row_number = 0; button_row_number < NUMBER_OF_BUTTON_ROWS; button_row_number++)
		{
			pullUpDnControl(button_row_gpio_pin_numbers[button_row_number], PUD_UP);
			handle_raw_button_sample((button_row_number * NUMBER_OF_BUTTON_COLUMNS) + button_column_number, 1 - digitalRead(button_row_gpio_pin_numbers[button_row_number]));
			pullUpDnControl(button_row_gpio_pin_numbers[button_row_number], PUD_OFF);
		}

		pinMode(button_column_gpio_pin_numbers[button_column_number], INPUT);
	}
}

void sampling_loop(void)
{
	while (1)
	{
		unsigned int start_time = millis();
		unsigned int time_spent;

		sample_button_matrix();
		sample_analog_inputs();
		time_spent = millis() - start_time;

		if (time_spent < SAMPLE_FREQUENCY_MSECS)
		{
			delay(SAMPLE_FREQUENCY_MSECS - time_spent);
		}
	}
}

void setup_midi(void)
{
	snd_seq_open(&midi_client, "default", SND_SEQ_OPEN_OUTPUT, 0);
	snd_seq_set_client_name(midi_client, MIDI_CLIENT_NAME);
	midi_port = snd_seq_create_simple_port(midi_client, MIDI_PORT_NAME, SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
}

void setup_gpio(void)
{
	for (int button_number = 0; button_number < NUMBER_OF_BUTTONS; button_number++)
	{
		button_debounce_integrator[button_number] = 0;
		current_button_state[button_number] = 0;
	}

	for (int analog_input_number = 0; analog_input_number < NUMBER_OF_ANALOG_INPUTS; analog_input_number++)
	{
		current_analog_input_values[analog_input_number] = 0;
	}

	wiringPiSetup();

	for (int button_row_number = 0; button_row_number < NUMBER_OF_BUTTON_ROWS; button_row_number++)
	{
		pinMode(button_row_gpio_pin_numbers[button_row_number], INPUT);
	}

	for (int button_column_number = 0; button_column_number < NUMBER_OF_BUTTON_COLUMNS; button_column_number++)
	{
		pinMode(button_column_gpio_pin_numbers[button_column_number], INPUT);
	}

	pinMode(SPI_MOSI_GPIO_PIN_NUMBER, OUTPUT);
	pinMode(SPI_MISO_GPIO_PIN_NUMBER, INPUT);
	pinMode(SPI_CLK_GPIO_PIN_NUMBER, OUTPUT);
	pinMode(SPI_CS_GPIO_PIN_NUMBER, OUTPUT);
}

int main(int argc, char **argv)
{
	setup_gpio();
	setup_midi();
	sampling_loop();
	return 0;
}

