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
#include "util.h"
#include "elf_ld.h"

static boolean_t elf64_update_rela_section(uint16_t e_type, uint64_t relocation_offset,
        elf64_dyn_t *dyn_section, uint64_t dyn_section_sz)
{
    elf64_rela_t *rela = NULL;
    uint64_t rela_sz = 0;
    uint64_t rela_entsz = 0;
    elf64_sym_t *symtab = NULL;
    uint64_t symtab_entsz = 0;
    uint64_t i;
    uint64_t d_tag = 0;

    if (!dyn_section){
        printf("trusty loader: invalid dynamic section parameter\n");
        return FALSE;
    }

    /* locate rela address, size, entry size */
    for (i = 0; i < dyn_section_sz / sizeof(elf64_dyn_t); ++i) {
        d_tag = dyn_section[i].d_tag;

        if(DT_RELA == d_tag) {
            rela = (elf64_rela_t *)(uint64_t)(dyn_section[i].d_un.d_ptr +
                    relocation_offset);
        } else if((DT_RELASZ == d_tag) || (DT_RELSZ == d_tag)) {
            rela_sz = dyn_section[i].d_un.d_val;
        } else if(DT_RELAENT == d_tag) {
            rela_entsz = dyn_section[i].d_un.d_val;
        } else if(DT_SYMTAB == d_tag) {
            symtab = (elf64_sym_t *)(uint64_t)(dyn_section[i].d_un.d_ptr +
                    relocation_offset);
        } else if(DT_SYMENT == d_tag) {
            symtab_entsz = dyn_section[i].d_un.d_val;
        } else { 
            continue; 
        }
    }

    if (NULL == rela || 0 == rela_sz || NULL == symtab
            || sizeof(elf64_rela_t) != rela_entsz
            || sizeof(elf64_sym_t) != symtab_entsz) {

        if (e_type == ET_DYN) {
            printf("trusty loader: DYN type relocation section is optional\n");
            return TRUE;
        } else {
            printf("trusty loader: EXEC type missed\n");
            return FALSE;
        }
    }

    for (i = 0; i < rela_sz / rela_entsz; ++i) {
        uint64_t *target_addr = (uint64_t *)(uint64_t)(rela[i].r_offset +
                relocation_offset);
        uint32_t symtab_idx;

        switch (rela[i].r_info & 0xFF) {
            /* Formula for R_x86_64_32 and R_X86_64_64 are same: S + A  */
            case R_X86_64_32:
            case R_X86_64_64:
                *target_addr = rela[i].r_addend + relocation_offset;
                symtab_idx = (uint32_t)(rela[i].r_info >> 32);
                *target_addr += symtab[symtab_idx].st_value;
                break;
            case R_X86_64_RELATIVE:
                *target_addr = rela[i].r_addend + relocation_offset;
                break;
            case 0:        /* do nothing */
                break;
            default:
                printf("trusty loader: Unsupported Relocation %#x\n",
                        rela[i].r_info & 0xFF);
                return FALSE;
        }
    }

    return TRUE;
}

static void elf64_update_segment_table(uint64_t runtime_addr, 
        uint64_t relocation_offset)
{
    elf64_ehdr_t *ehdr;
    uint8_t      *phdrtab;
    uint32_t     i;

    ehdr = (elf64_ehdr_t *)runtime_addr;
    phdrtab = (uint8_t *)(uint64_t)(runtime_addr+ ehdr->e_phoff);

    for (i = 0; i < (uint16_t)ehdr->e_phnum; ++i) {
        elf64_phdr_t *phdr = (elf64_phdr_t *)GET_PHDR(ehdr, phdrtab, i);

        if (0 != phdr->p_memsz) {
            phdr->p_paddr += relocation_offset;
            phdr->p_vaddr += relocation_offset;
        }
    }
}

static boolean_t elf64_load_executable(uint64_t loadtime_addr, uint64_t runtime_addr,
        uint64_t *runtime_entry)
{
    elf64_ehdr_t  *ehdr;
    elf64_phdr_t  *phdr;
    elf64_phdr_t  *phdr_dyn = NULL;
    uint8_t       *phdrtab;
    elf64_dyn_t   *dyn_section;
    uint64_t      low_addr = (uint64_t) ~0;
    uint64_t      max_addr = 0;
    uint64_t      addr;
    uint64_t      memsz;
    uint64_t      filesz;
    uint64_t      relocation_offset;
    uint64_t      offset_0_addr = (uint64_t)~0;
    uint16_t      cnt;
    uint64_t      runtime_size;

    /* map ELF header to Ehdr */
    ehdr = (elf64_ehdr_t *)loadtime_addr;

    /* map Program Segment header Table to Phdrtab */
    phdrtab = (uint8_t *)((uint64_t)loadtime_addr + (uint64_t)ehdr->e_phoff);

    /* Calculate amount of memory required. First calculate size of all
     * loadable segments 
     */
    for (cnt = 0; cnt < (uint16_t)ehdr->e_phnum; ++cnt) {
        phdr = (elf64_phdr_t *)GET_PHDR(ehdr, phdrtab, cnt);

        addr = phdr->p_paddr;
        memsz = phdr->p_memsz;

        if (PT_LOAD != phdr->p_type || 0 == phdr->p_memsz) {
            continue;
        }

        if (addr < low_addr) {
            low_addr = addr;
        }

        if (addr + memsz > max_addr) {
            max_addr = addr + memsz;
        }
    }

    /* check the memory size */
    if (0 != (low_addr & PAGE_4K_MASK)) {
        printf("trusty loader: low address page not aligned:%#p\n", low_addr);
        return FALSE;
    }

    runtime_size = PAGE_ALIGN_4K(max_addr - low_addr);

    if (TRUSTY_RUNTIME_TOTAL_SIZE < runtime_size || 0 == runtime_size) {
        printf("trusty loader: memory is smaller than required or it is zero\n");
        return FALSE;
    }

    relocation_offset = runtime_addr - low_addr;

    /* now actually copy image to its target destination */
    for (cnt = 0; cnt < (uint16_t)ehdr->e_phnum; ++cnt) {
        phdr = (elf64_phdr_t *)GET_PHDR(ehdr, phdrtab, cnt);

        if (PT_DYNAMIC == phdr->p_type) {
            phdr_dyn = phdr;
            continue;
        }

        if (PT_LOAD != phdr->p_type || 0 == phdr->p_memsz) {
            continue;
        }

        if (0 == phdr->p_offset)
            offset_0_addr = phdr->p_paddr;

        filesz = phdr->p_filesz;
        addr = phdr->p_paddr;
        memsz = phdr->p_memsz;

        /* make sure we only load what we're supposed to! */
        if (filesz > memsz) {
            filesz = memsz;
        }

        memcpy((void *)(uint64_t)(addr + relocation_offset),
                (void *)(uint64_t)(loadtime_addr + phdr->p_offset),
                (uint64_t)filesz);

        if (filesz < memsz) {
            memset((void *)(uint64_t)(addr + filesz + relocation_offset), 0,
                    (uint64_t)(memsz - filesz));
        }
    }

    /* if there's a segment whose P_Offset is 0, elf header and
     * segment headers are in this segment and will be relocated
     * to target location with this segment. if such segment exists,
     * OffsetZeroAddr will be updated to hold the P_Paddr. usually
     * this P_Paddr is the minimal Address (=LowAddr).
     * add a check here to detect violation.
     */
    if (offset_0_addr != (uint64_t)~0) {
        if (offset_0_addr != low_addr) {
            printf("trusty loader: elf header is relocated to wrong place!\n");
            return FALSE;
        }
        elf64_update_segment_table(runtime_addr, relocation_offset);
    }

    if (NULL != phdr_dyn) {
        dyn_section = (elf64_dyn_t *)(loadtime_addr + phdr_dyn->p_offset);
        if (!elf64_update_rela_section(ehdr->e_type, relocation_offset, dyn_section,
                phdr_dyn->p_filesz))
            printf("trusty loader: failed to update rela section!\n");
        return FALSE;
    }

    /* get the relocation entry addr */
    *runtime_entry = ehdr->e_entry + relocation_offset;

    return TRUE;
}

// relocate elf image accroding to header.
boolean_t relocate_elf_image (uint64_t loadtime_addr,
        uint64_t runtime_addr, uint64_t *run_entry)
{
    // check header
    if (!elf_header_is_valid((elf64_ehdr_t *)loadtime_addr)) {
        printf("trusty loader: elf header invalid\n");
        return FALSE;
    }

    // check ELF type, 64 bit supports only
    if (!is_elf64((elf64_ehdr_t *)loadtime_addr)) {
        printf("trusty loader: elf type unsupported!\n");
        return FALSE;
    }

    // load elf image to reserved memory region
    if (!elf64_load_executable(loadtime_addr, runtime_addr, run_entry)) {
        printf("trusty loader: faile to load elf image!\n");
        return FALSE;
    }

    return TRUE;
}

