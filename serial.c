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
#include "serial.h"

#define UART_REG_THR 0     /* WO Transmit Holding Register */
#define UART_REG_RBR 0     /* RO Receive Buffer Register */
#define UART_REG_DLL 0     /* R/W Divisor Latch LSB */
#define UART_REG_DLM 1     /* R/W Divisor Latch MSB */
#define UART_REG_IER 1     /* R/W Interrupt Enable Register */
#define UART_REG_IIR 2     /* RO Interrupt Identification Register */
#define UART_REG_FCR 2     /* WO FIFO Cotrol Register */
#define UART_REG_LCR 3     /* R/W Line Control Register */
#define UART_REG_MCR 4     /* R/W Modem Control Register */
#define UART_REG_LSR 5     /* R/W Line Status Register */
#define UART_REG_MSR 6     /* R/W Modem Status Register */
#define UART_REG_SCR 7     /* R/W Scratch Pad Register */

#define UART_LSR_THRE_MASK (1 << 5)

#ifdef SERIAL_MMIO
static inline uint8_t serial_get_reg(uint64_t base_addr, uint32_t reg)
{
	return *(volatile uint8_t *)(base_addr + (uint64_t)reg * 4);
}

static inline void serial_set_reg(uint64_t base_addr, uint32_t reg, uint8_t val)
{
	*(volatile uint8_t *)(base_addr + (uint64_t)reg * 4) = val;
}
#else
static inline uint8_t asm_in8(uint16_t port)
{
	uint8_t val8;

	__asm__ __volatile__ (
	"inb %1, %0"
	: "=a" (val8)
	: "d" (port));
	return val8;
}

static inline void asm_out8(uint16_t port, uint8_t val8)
{
	__asm__ __volatile__ (
	"outb %1, %0"
	:
	: "d" (port), "a" (val8));

}

static inline uint8_t serial_get_reg(uint64_t base_addr, uint32_t reg)
{
	return asm_in8((uint16_t)base_addr + (uint16_t)reg);
}

static inline void serial_set_reg(uint64_t base_addr, uint32_t reg, uint8_t val)
{
	asm_out8((uint16_t)base_addr + (uint16_t)reg, val);
}
#endif

static void serial_putc(char c, uint64_t serial_base)
{
	uint8_t data;

	while (1)
	{
		data = serial_get_reg(serial_base, UART_REG_LSR);
		if (data & UART_LSR_THRE_MASK)
			break;
	}
	serial_set_reg(serial_base, UART_REG_THR, c);
}

void serial_puts(const char *str, uint64_t serial_base)
{
	uint32_t i;
	for (i = 0; str[i] != 0; i++)
		serial_putc(str[i], serial_base);
}
