##############################################################################
# Copyright (c) 2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

.file   "trusty_loder_entry.s"

#include "trusty_loader_asm.h"

.text

.extern trusty_loader_main

.globl start
.org 0 #make below code in beginning of final binary.
start:
.align 8

/* Multiboot header. */
multiboot_header:
	/* magic */
	.long   MULTIBOOT_HEADER_MAGIC
	/* flags */
	.long   MULTIBOOT_HEADER_FLAGS
	/* checksum */
	.long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
	/* header_addr */
	.long   LOAD_ADDR + multiboot_header - start
	/* load_addr */
	.long   LOAD_ADDR
	/* load_end_addr */
	.long   0
	/* bss_end_addr */
	.long   0
	/* entry_addr */
	.long   LOAD_ADDR + start_x64 - start

/* trusty image start offset, hard code for now.
 * TODO: should be updated by image stich script. 
 */
trusty_file_info:
	.long   TRUSTY_LOAD_ADDR_MAGIC

/* 64bit entry point */
.code64
start_x64:
	/* reset rflags */
	pushq $0
	popfq

	/* setup loader's own stack, don't use the old rsp(it might be above 4G) */
	leaq bspstack(%rip), %rax
	movq %rax, %rsp

    /* prepare for trusty_loader_main's parameters */
	movq %rbx, %rdi
    leaq start(%rip), %rsi

	call trusty_loader_main
	/* it should never return */

/* stack for trusty loader */
.align 4
bspstack_start:
	.fill TRUSTY_LOADER_STACK_SIZE, 1, 0xCC
bspstack:

