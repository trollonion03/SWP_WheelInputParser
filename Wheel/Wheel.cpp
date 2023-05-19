#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <chrono>
#include "hidapi.h"

using namespace std;

void parseInput(unsigned char* dat);

int main() {
	int res;
	unsigned char buf[256];
	unsigned char read[64];
#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device* handle;
	int i;


	struct hid_device_info* devs, * cur_dev;

	if (hid_init())
		return -1;

	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	bool flag = false;
#if 1
	while (cur_dev) {
		if (cur_dev->vendor_id == 0x044f && cur_dev->product_id == 0xb697) {
			printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
			printf("\n");
			printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
			printf("  Product:      %ls\n", cur_dev->product_string);
			printf("  Release:      %hx\n", cur_dev->release_number);
			printf("  Interface:    %d\n", cur_dev->interface_number);
			printf("\n");
			flag = true;
		}
		cur_dev = cur_dev->next;
	}

	if (!flag) {
		printf("Device not Found!\n");
		return 0;
	}

#endif;
	hid_free_enumeration(devs);

	// Set up the command buffer.
	memset(buf, 0x00, sizeof(buf));
	buf[0] = 0x01;
	buf[1] = 0x81;


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open(0x044f, 0xb697, NULL);
	if (!handle) {
		printf("unable to open device\n");
		return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);

	// Read the Serial Number String
	wstr[0] = 0x0000;
	res = hid_get_serial_number_string(handle, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read serial number string\n");
	printf("Serial Number String: (%d) %ls", wstr[0], wstr);
	printf("\n");

	// Read Indexed String 1
	wstr[0] = 0x0000;
	res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
	if (res < 0)
		printf("Unable to read indexed string 1\n");
	printf("Indexed String 1: %ls\n", wstr);

	// Set the hid_read() function to be non-blocking.
	hid_set_nonblocking(handle, 1);

	// Try to read from the device. There shoud be no
	// data here, but execution should not block.
	res = hid_read(handle, buf, 17);

	// Send a Feature Report to the device
	buf[0] = 0x2;
	buf[1] = 0xa0;
	buf[2] = 0x0a;
	buf[3] = 0x00;
	buf[4] = 0x00;
	res = hid_send_feature_report(handle, buf, 17);
	if (res < 0) {
		printf("Unable to send a feature report.\n");
	}

	memset(buf, 0, sizeof(buf));

	// Read a Feature Report from the device
	buf[0] = 0x2;
	res = hid_get_feature_report(handle, buf, sizeof(buf));
	if (res < 0) {
		printf("Unable to get a feature report.\n");
		printf("%ls", hid_error(handle));
	}
	else {
		// Print out the returned buffer.
		printf("Feature Report\n   ");
		for (i = 0; i < res; i++)
			printf("%02hhx ", buf[i]);
		printf("\n");
	}

	memset(buf, 0, sizeof(buf));
	while (1) {
		res = 0;
		int temp;
		while (res == 0) {
			res = hid_read(handle, read, sizeof(read));
#if 0
			for (i = 0; i < sizeof(read)/sizeof(unsigned char); i++) {
				printf("%02hhx ", read[i]);
			}
#endif
			parseInput(read);
			//system("cls");
			printf("\n");
			if (res == 0)
				printf("waiting...\n");
			if (res < 0)
				printf("Unable to read()\n");
			//Sleep(1);
		}
		if (_kbhit())
			break;
	}

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef WIN32
	system("pause");
#endif

	return 0;
}

void parseInput(unsigned char* dat)
{
	//system("cls");
	int temp;
	printf("\n");
	//temp = (dat[2] << 8) | dat[1];
	temp = dat[2];
	printf("\nWheel: %03d ", (int)(temp));

	//temp = (dat[3] << 8) | dat[4];
	temp = dat[4];
	printf("break: %03d ", temp);

	//temp = (dat[5] << 8) | dat[6];
	temp = dat[6];
	printf("accelerator: %03d ", temp);
}
