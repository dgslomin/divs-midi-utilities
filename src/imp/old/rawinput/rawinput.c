
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

int main(int argc, char **argv)
{
	unsigned int number_of_raw_input_devices;
	RAWINPUTDEVICELIST *raw_input_device_list;
	unsigned int raw_input_device_number;

	GetRawInputDeviceList(NULL, &number_of_raw_input_devices, sizeof(RAWINPUTDEVICELIST));
	raw_input_device_list = (RAWINPUTDEVICELIST *)(malloc(sizeof(RAWINPUTDEVICELIST) * number_of_raw_input_devices));
	GetRawInputDeviceList(raw_input_device_list, &number_of_raw_input_devices, sizeof(RAWINPUTDEVICELIST));

	for (raw_input_device_number = 0; raw_input_device_number < number_of_raw_input_devices; raw_input_device_number++)
	{
		int raw_input_device_name_size;
		char *raw_input_device_name;
		RID_DEVICE_INFO raw_input_device_info;

		GetRawInputDeviceInfo(raw_input_device_list[raw_input_device_number].hDevice, RIDI_DEVICENAME, NULL, &raw_input_device_name_size);
		raw_input_device_name = malloc(raw_input_device_name_size);
		GetRawInputDeviceInfo(raw_input_device_list[raw_input_device_number].hDevice, RIDI_DEVICENAME, raw_input_device_name, &raw_input_device_name_size);
		printf("%2d.  \"%s\"\n", raw_input_device_number + 1, raw_input_device_name);
		free(raw_input_device_name);

		raw_input_device_info.cbSize = sizeof(RID_DEVICE_INFO);
		GetRawInputDeviceInfo(raw_input_device_list[raw_input_device_number].hDevice, RIDI_DEVICEINFO, &raw_input_device_info, &(raw_input_device_info.cbSize));

		switch (raw_input_device_info.dwType)
		{
			case RIM_TYPEHID:
			{
				printf("     type: human interface device other than keyboard or mouse\n");
				break;
			}
			case RIM_TYPEKEYBOARD:
			{
				printf("     type: keyboard (%d/%d)\n", raw_input_device_info.keyboard.dwType, raw_input_device_info.keyboard.dwSubType);
				printf("     scan code mode: %d\n", raw_input_device_info.keyboard.dwKeyboardMode);
				printf("     number of function keys: %d\n", raw_input_device_info.keyboard.dwNumberOfFunctionKeys);
				printf("     number of indicators: %d\n", raw_input_device_info.keyboard.dwNumberOfIndicators);
				printf("     total number of keys: %d\n", raw_input_device_info.keyboard.dwNumberOfKeysTotal);
				break;
			}
			case RIM_TYPEMOUSE:
			{
				printf("     type: mouse\n");
				break;
			}
			default:
			{
				printf("     type: other raw input device\n");
				break;
			}
		}

		printf("\n");
	}

	free(raw_input_device_list);
	return 0;
}

