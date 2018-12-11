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
#include "util.h"

/* with tests we found that, using "stosb" to set 1 page is
 * a little bit quicker than "stosq" in most cases
 */
void memset(void *dest, uint8_t val, uint64_t count)
{
	__asm__ __volatile__ (
		"cld        \n\t"
		"rep stosb  \n\t"
		:: "D" (dest), "a" (val), "c" (count)
		);
	return;
}

/* with tests we found that, using "movsb" to copy 1 page is
 * a little bit quicker than "movsq" in most cases
 */
void memcpy(void *dest, const void *src, uint64_t count)
{
	if (dest < src) {
		__asm__ __volatile__ (
			"cld        \n\t"
			"rep movsb  \n\t"
			:: "D" (dest), "S" (src), "c" (count)
			);
	}else {
		__asm__ __volatile__ (
			"std        \n\t"
			"rep movsb  \n\t"
			"cld        \n\t"
			:: "D" ((uint64_t)dest + count - 1), "S" ((uint64_t)src + count - 1), "c" (count)
			);
	}
	return;
}

