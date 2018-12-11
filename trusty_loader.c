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
#include "elf_ld.h"
#include "string.h"
#include "util.h"

#define MULTIBOOT_HEADER_SIZE         32

//Macro must align to definition in acorn
#define _HC_ID(x, y) (((x)<<24)|(y))

#define HC_ID 0x80UL

#define HC_ID_TRUSTY_BASE           0x70UL
#define HC_INITIALIZE_TRUSTY        _HC_ID(HC_ID, HC_ID_TRUSTY_BASE + 0x00UL)

#define TRUSTY_RUNTIME_PAGES        16*1024
#define TRUSTY_RUNTIME_BASE         0x7FC0000000
#define TRUSTY_RSVD_SIZE            0x1000
#define TRUSTY_64BIT_ENTRY_OFFSET   0x400

#define CHECK_FLAG(flag,bit)	((flag) & (1 << (bit)))

/*
 * Trusty boot params, used for HC_INITIALIZE_TRUSTY.
 */

typedef struct {
    uint32_t size_of_struct;    /* sizeof this structure */
    uint32_t version;           /* version of this structure */
    uint32_t base_addr;         /* trusty runtime memory base address */
    uint32_t entry_point;       /* trusty entry point */
    uint32_t mem_size;          /* trusty runtime memory size */
    uint32_t padding;           /* padding */
    uint32_t base_addr_high;    /* trusty runtime memory base address (high 32bit) */
    uint32_t entry_point_high;  /* trusty entry point (high 32bit) */
    uint8_t  rpmb_key[64];      /* rpmb key */
} trusty_boot_param_t;

/* arguments parsed from cmdline */
typedef struct {
	uint32_t size_of_struct;
    uint32_t version;
    uint64_t seedlist_info_addr;
    uint64_t platform_info_addr;
    uint64_t vmm_boot_param_addr;
} image_boot_param_t;

/* Linux boot cpu sate */
typedef struct {
  uint32_t eip;
  uint32_t eax;
  uint32_t ebx;
  uint32_t esi;
  uint32_t edi;
  uint32_t ecx;
} cpu_boot_state_t;

/* Linux boot params, used for linux launch */
typedef struct {
  uint32_t size_of_struct;
  uint32_t version;
  cpu_boot_state_t cpu_state;
} linux_boot_param_t;


/* a.out kernel image */
typedef struct {
	uint32_t tabsize;
	uint32_t strsize;
	uint32_t addr;
	uint32_t reserved;
} aout_t;

/* elf kernel */
typedef struct {
	uint32_t num;
	uint32_t size;
	uint32_t addr;
	uint32_t shndx;
} elf_t;

/* only used partial of the standard multiboot_info_t */
typedef struct {
	uint32_t flags;

	/* valid if flags[0] (MBI_MEMLIMITS) set */
	uint32_t mem_lower;
	uint32_t mem_upper;

	/* valid if flags[1] set */
	uint32_t boot_device;

	/* valid if flags[2] (MBI_CMDLINE) set */
	uint32_t cmdline;

	/* valid if flags[3] (MBI_MODS) set */
	uint32_t mods_count;
	uint32_t mods_addr;

	/* valid if flags[4] or flags[5] set */
	union {
		aout_t aout_image;
		elf_t elf_image;
	} syms;

	/* valid if flags[6] (MBI_MEMMAP) set */
	uint32_t mmap_length;
	uint32_t mmap_addr;
} multiboot_info_t;

static boolean_t cmdline_parse(multiboot_info_t *mbi, uint64_t *boot_param_addr)
{
    const char *cmdline;
	const char *arg;
	const char *param;
	uint32_t addr;

	if (!mbi || !boot_param_addr)
		return FALSE;

	if (!CHECK_FLAG(mbi->flags, 2)) {
		printf("trusty loader: multiboot info does not contain cmdline field!\n");
		return FALSE;
	}

	cmdline = (const char *)(uint64_t)mbi->cmdline;

    /* Parse ImageBootParamsAddr */
	printf("cmdline from vSBL: %s\n", cmdline);
	arg = strstr_s(cmdline, MAX_STR_LEN, "ImageBootParamsAddr=",
            sizeof("ImageBootParamsAddr=")-1);

	if (!arg) {
		printf("trusty loader: ImageBootParamsAddr not found in cmdline!\n");
		return FALSE;
	}

	param = arg + sizeof("ImageBootParamsAddr=") - 1;

	addr = str2uint(param, 18, NULL, 16);
	if ((addr == (uint32_t)-1) ||
		(addr == 0)) {
		printf("trusty loader: failed to parse ImageBootParamsAddr!\n");
		return FALSE;
	}
	*boot_param_addr = (uint64_t)addr;

    return TRUE;
}

static int launch_trusty(trusty_boot_param_t *param)
{
    int ret = TRUE;

    register uint64_t hypercall_id __asm__("r8") = HC_INITIALIZE_TRUSTY;

    if (!param)
        return FALSE;

    __asm__ __volatile__ (
        "vmcall;"
        : "=a" (ret)
        : "r" (hypercall_id), "D" ((uint64_t)param)
        : "memory");

    return ret;
}

static void launch_linux(uint64_t boot_param_addr)
{
    image_boot_param_t *image_boot_params = (image_boot_param_t *)boot_param_addr;
    linux_boot_param_t *linux_boot_params = (linux_boot_param_t *)
        (image_boot_params->vmm_boot_param_addr);
    cpu_boot_state_t *cpu_state = &(linux_boot_params->cpu_state);
    uint64_t rip = cpu_state->eip;
    uint64_t rax = cpu_state->eax;
    uint64_t rbx = cpu_state->ebx;
    uint64_t rsi = cpu_state->esi;
    uint64_t rdi = cpu_state->edi;
    uint64_t rcx = cpu_state->ecx;

    printf("trusty loader linux entry point is 0x%lx\n", rip);

    __asm__ __volatile__ ("cli\n\t" 
                          "movq %1, %%rax\n\t"
                          "movq %2, %%rbx\n\t"
                          "movq %3, %%rsi\n\t"
                          "movq %4, %%rdi\n\t"
                          "movq %5, %%rcx\n\t"
                          "jmp *%0\n\t"
						  : 
                          : "m" (rip),
                          "m" (rax),
                          "m" (rbx),
                          "m" (rsi),
                          "m" (rdi),
                          "m" (rcx));
}

void trusty_loader_main(uint64_t *multiboot_info, uint64_t trusty_loader_base)
{
    trusty_boot_param_t param;
    multiboot_info_t *mbi = (multiboot_info_t *)multiboot_info;
    uint64_t trusty_loadtime_addr = *((uint32_t *)(trusty_loader_base +
                MULTIBOOT_HEADER_SIZE)) * 512;
    uint64_t trusty_runtime_addr = TRUSTY_RUNTIME_BASE + TRUSTY_RSVD_SIZE;
    uint64_t trusty_run_entry;
    uint64_t boot_param_addr;

    print_init();

    printf("trusty loader start\n");

    memset((void *)&param, 0, sizeof(trusty_boot_param_t));

    if (!cmdline_parse(mbi, &boot_param_addr)) {
        printf("trusty loader: cmdline parse failed");
        goto fail;
    }

    if (!relocate_elf_image(trusty_loadtime_addr, trusty_runtime_addr,
                &trusty_run_entry)) {
		printf("trusty loader: relocate trusty failed\n");
		goto fail;
	}

    // Fill in parameters
    param.size_of_struct   = sizeof(trusty_boot_param_t);
    param.mem_size         = 16 MEGABYTE;
    param.version          = 2;
    param.base_addr        = (uint32_t)((TRUSTY_RUNTIME_BASE) & 0xFFFFFFFF);
    param.base_addr_high   = (uint32_t)((TRUSTY_RUNTIME_BASE >> 32) & 0xFFFFFFFF);
    param.entry_point      = (uint32_t)((trusty_run_entry +
                TRUSTY_64BIT_ENTRY_OFFSET) & 0xFFFFFFFF);
    param.entry_point_high = (uint32_t)(((trusty_run_entry +
                TRUSTY_64BIT_ENTRY_OFFSET) >> 32) & 0xFFFFFFFF);

    launch_trusty(&param);

    launch_linux(boot_param_addr);

fail:
	printf("trusty loader: deadloop!\n");
	__STOP_HERE__;
}
