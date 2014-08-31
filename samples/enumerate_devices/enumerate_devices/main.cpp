#include <stdio.h>
#include <interception.h>


int main()
{
	InterceptionContext context;
	InterceptionDevice devices[INTERCEPTION_MAX_DEVICE];
	int actual_devices_count = 0;

	context = interception_create_context();
	actual_devices_count = interception_enumerate_devices(context, devices, INTERCEPTION_MAX_DEVICE, INTERCEPTION_DEVICE_KEYBOARD | INTERCEPTION_DEVICE_MOUSE);
	printf("number of devices:  %d\n", actual_devices_count);
	for (int i = 0; i < actual_devices_count; ++i) {
		printf("%d\n", devices[i]);
	}
	printf("\n");

	actual_devices_count = interception_enumerate_keyboards(context, devices, INTERCEPTION_MAX_DEVICE);
	printf("number of keyboards: %d\n", actual_devices_count);
	for (int i = 0; i < actual_devices_count; ++i) {
		printf("%d\n", devices[i]);
	}
	printf("\n");

	actual_devices_count = interception_enumerate_mouses(context, devices, INTERCEPTION_MAX_DEVICE);
	printf("number of mouses:    %d\n", actual_devices_count);
	for (int i = 0; i < actual_devices_count; ++i) {
		printf("%d\n", devices[i]);
	}
	printf("\n");

	return 0;
}