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
#ifndef _TRUSTY_LOADER_BASE_H_
#define _TRUSTY_LOADER_BASE_H_

#define UNUSED  __attribute__((unused))
#define PACKED  __attribute((packed))
/* PACKED should be applied to below cases
 * 1.the size of the struct is not 32bit aligned. e.g. gdtr64_t.
 * 2.the member of the struct is not aligned with it's size.
 *   e.g. tss64_t (rsp is 64bit but it's offset is 32bit, not 64bit aligned).
 * 3.the struct is not aligned with max size of the member. e.g. acpi_table_rsdp_t.
 */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t boolean_t;
typedef struct {
	uint64_t uint64[2];
} uint128_t;

#define FALSE 0
#define TRUE 1

#define NULL ((void *)0)

#define ALIGN_B(value, align) \
	((uint64_t)(value) & (~((uint64_t)(align) - 1ULL)))
#define ALIGN_F(value, align) \
	ALIGN_B(value + align - 1, align)

#define KILOBYTE            *1024ULL
#define MEGABYTE            *1024ULL KILOBYTE
#define GIGABYTE            *1024ULL MEGABYTE

#define PAGE_4K_SHIFT 12
#define PAGE_4K_SIZE 		(4 KILOBYTE)
#define PAGE_4K_MASK 		(PAGE_4K_SIZE - 1)
#define PAGE_ALIGN_4K(x)    ALIGN_F(x, PAGE_4K_SIZE)
/* Returns number of pages (4KB) required to accomdate x bytes */
#define PAGE_4K_ROUNDUP(x)  (((x) + PAGE_4K_SIZE - 1) >> PAGE_4K_SHIFT)

#ifndef MAX
#define MAX(a, b)   ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)   ((b) > (a) ? (a) : (b))
#endif

#define __STOP_HERE__ \
	while (1) { \
	}

#endif
