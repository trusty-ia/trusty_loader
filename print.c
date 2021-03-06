/*******************************************************************************
* Copyright (c) 2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/
#include "print.h"
#include "serial.h"
#include "string.h"

#define PRINTF_BUFFER_SIZE 256

static uint64_t serial_base;

void print_init(void)
{
	/* TODO: hard code here, will get from PCI driver */
	serial_base = 0xfc000000;
}

/*caller must make sure this function is NOT
called simultaneously in different cpus*/
void printf(const char *format, ...)
{
	uint32_t printed_size;
	/* use static buffer to save stack space */
	va_list args;
	char buffer[PRINTF_BUFFER_SIZE];

	va_start(args, format);
	printed_size = vmm_vsprintf_s(buffer, PRINTF_BUFFER_SIZE, format, args);
	va_end(args);
	if (printed_size > 0)
		serial_puts(buffer, serial_base);
}

