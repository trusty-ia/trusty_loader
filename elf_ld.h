/****************************************************************************
* Copyright (c) 2018 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0

* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
****************************************************************************/

#ifndef _ELF_LD_H_
#define _ELF_LD_H_

#include "trusty_loader_base.h"

#define EI_NIDENT       16      /* Size of e_ident array. */

/* Indexes into the e_ident array.  Keep synced with
 * http://www.sco.com/developers/gabi/latest/ch4.eheader.html */
#define EI_MAG0         0       /* Magic number, byte 0. */
#define EI_MAG1         1       /* Magic number, byte 1. */
#define EI_MAG2         2       /* Magic number, byte 2. */
#define EI_MAG3         3       /* Magic number, byte 3. */
#define EI_CLASS        4       /* Class of machine. */
#define EI_DATA         5       /* Data format. */
#define EI_VERSION      6       /* ELF format version. */
#define EI_OSABI        7       /* Operating system / ABI identification */
#define EI_ABIVERSION   8       /* ABI version */
#define OLD_EI_BRAND    8       /* Start of architecture identification. */
#define EI_PAD          9       /* Start of padding (per SVR4 ABI). */
#define EI_NIDENT       16      /* Size of e_ident array. */

/* Values for the magic number bytes. */
#define ELFMAG0         0x7f
#define ELFMAG1         'E'
#define ELFMAG2         'L'
#define ELFMAG3         'F'
#define ELFMAG          "\177ELF"       /* magic string */
#define SELFMAG         4               /* magic string size */

/* Values for e_ident[EI_CLASS]. */
#define ELFCLASSNONE    0       /* Unknown class. */
#define ELFCLASS32      1       /* 32-bit architecture. */
#define ELFCLASS64      2       /* 64-bit architecture. */

/* Values for e_ident[EI_DATA]. */
#define ELFDATANONE     0       /* Unknown data format. */
#define ELFDATA2LSB     1       /* 2's complement little-endian. */
#define ELFDATA2MSB     2       /* 2's complement big-endian. */

/* Values for e_ident[EI_VERSION] and e_version. */
#define EV_NONE         0
#define EV_CURRENT      1

/* Values for e_type. */
#define ET_NONE         0       /* Unknown type. */
#define ET_REL          1       /* Relocatable. */
#define ET_EXEC         2       /* Executable. */
#define ET_DYN          3       /* Shared object. */
#define ET_CORE         4       /* Core file. */
#define ET_LOOS         0xfe00  /* First operating system specific. */
#define ET_HIOS         0xfeff  /* Last operating system-specific. */
#define ET_LOPROC       0xff00  /* First processor-specific. */
#define ET_HIPROC       0xffff  /* Last processor-specific. */

/* Values for p_type. */
#define PT_NULL         0               /* Unused entry. */
#define PT_LOAD         1               /* Loadable segment. */
#define PT_DYNAMIC      2               /* Dynamic linking information segment. */
#define PT_INTERP       3               /* Pathname of interpreter. */
#define PT_NOTE         4               /* Auxiliary information. */
#define PT_SHLIB        5               /* reserved (not used). */
#define PT_PHDR         6               /* Location of program header itself. */

/* Values for d_tag. */
#define DT_NULL         0       /* Terminating entry. */
#define DT_NEEDED       1       /* String table offset of a needed shared library. */
#define DT_PLTRELSZ     2       /* Total size in bytes of PLT relocations. */
#define DT_PLTGOT       3       /* Processor-dependent address. */
#define DT_HASH         4       /* Address of symbol hash table. */
#define DT_STRTAB       5       /* Address of string table. */
#define DT_SYMTAB       6       /* Address of symbol table. */
#define DT_RELA         7       /* Address of ElfNN_Rela relocations. */
#define DT_RELASZ       8       /* Total size of ElfNN_Rela relocations. */
#define DT_RELAENT      9       /* Size of each ElfNN_Rela relocation entry. */
#define DT_STRSZ        10      /* Size of string table. */
#define DT_SYMENT       11      /* Size of each symbol table entry. */
#define DT_INIT         12      /* Address of initialization function. */
#define DT_FINI         13      /* Address of finalization function. */
#define DT_SONAME       14      /* String table offset of shared object name. */
#define DT_RPATH        15      /* String table offset of library path. [sup] */
#define DT_SYMBOLIC     16      /* Indicates "symbolic" linking. [sup] */
#define DT_REL          17      /* Address of ElfNN_Rel relocations. */
#define DT_RELSZ        18      /* Total size of ElfNN_Rel relocations. */
#define DT_RELENT       19      /* Size of each ElfNN_Rel relocation. */
#define DT_PLTREL       20      /* Type of relocation used for PLT. */
#define DT_DEBUG        21      /* reserved (not used). */
#define DT_TEXTREL      22      /* Indicates there may be relocations in non-writable segments. [sup] */
#define DT_JMPREL       23      /* Address of PLT relocations. */
#define DT_BIND_NOW     24      /* [sup] */
#define DT_INIT_ARRAY   25      /* Address of the array of pointers to initialization functions */
#define DT_FINI_ARRAY   26      /* Address of the array of pointers to termination functions */
#define DT_INIT_ARRAYSZ 27      /* Size in bytes of the array of initialization functions. */
#define DT_FINI_ARRAYSZ 28      /* Size in bytes of the array of termination functions. */
#define DT_RUNPATH      29      /* String table offset of a null-terminated library search path string. */
#define DT_FLAGS        30      /* Object specific flag values. */
#define DT_ENCODING     32      /* Values greater than or equal to DT_ENCODING */

/*
 * 64-bit relocations
 */

#define R_X86_64_NONE           0       /* No relocation. */
#define R_X86_64_64             1       /* Add 64 bit symbol value. */
#define R_X86_64_PC32           2       /* PC-relative 32 bit signed sym value. */
#define R_X86_64_GOT32          3       /* PC-relative 32 bit GOT offset. */
#define R_X86_64_PLT32          4       /* PC-relative 32 bit PLT offset. */
#define R_X86_64_COPY           5       /* Copy data from shared object. */
#define R_X86_64_GLOB_DAT       6       /* Set GOT entry to data address. */
#define R_X86_64_JMP_SLOT       7       /* Set GOT entry to code address. */
#define R_X86_64_RELATIVE       8       /* Add load address of shared object. */
#define R_X86_64_GOTPCREL       9       /* Add 32 bit signed pcrel offset to GOT. */
#define R_X86_64_32             10      /* Add 32 bit zero extended symbol value */
#define R_X86_64_32S            11      /* Add 32 bit sign extended symbol value */
#define R_X86_64_16             12      /* Add 16 bit zero extended symbol value */
#define R_X86_64_PC16           13      /* Add 16 bit signed extended pc relative symbol value */
#define R_X86_64_8              14      /* Add 8 bit zero extended symbol value */
#define R_X86_64_PC8            15      /* Add 8 bit signed extended pc relative symbol value */
#define R_X86_64_DTPMOD64       16      /* ID of module containing symbol */
#define R_X86_64_DTPOFF64       17      /* Offset in TLS block */
#define R_X86_64_TPOFF64        18      /* Offset in static TLS block */
#define R_X86_64_TLSGD          19      /* PC relative offset to GD GOT entry */
#define R_X86_64_TLSLD          20      /* PC relative offset to LD GOT entry */
#define R_X86_64_DTPOFF32       21      /* Offset in TLS block */
#define R_X86_64_GOTTPOFF       22      /* PC relative offset to IE GOT entry */
#define R_X86_64_TPOFF32        23      /* Offset in static TLS block */
#define R_X86_64_IRELATIVE      37

/* Legal values for machine_type below */

#define EM_386       3                  /* Intel 80386 */
#define EM_X86_64   62                  /* AMD x86-64 architecture */

/* Macros to facilitate locations of program segment headers and section
 * headers */
#define GET_PHDR(__ehdr, __phdrtab, __i) \
	((__phdrtab) + (__i) * (__ehdr)->e_phentsize)
#define GET_SHDR(__ehdr, __shdrtab, __i) \
	((__shdrtab) + (__i) * (__ehdr)->e_shentsize)

#define elf_header_is_valid(ehdr) (\
		ELFMAG0 == (ehdr)->e_ident[EI_MAG0] && \
		ELFMAG1 == (ehdr)->e_ident[EI_MAG1] && \
		ELFMAG2 == (ehdr)->e_ident[EI_MAG2] && \
		ELFMAG3 == (ehdr)->e_ident[EI_MAG3] && \
		ELFDATA2LSB == (ehdr)->e_ident[EI_DATA] && \
		EV_CURRENT == (ehdr)->e_version && \
		(ET_DYN == (ehdr)->e_type || ET_EXEC == (ehdr)->e_type))

#define is_elf64(ehdr) (\
		ELFCLASS64 == (ehdr)->e_ident[EI_CLASS] && \
		EM_X86_64 == (ehdr)->e_machine)

#define TRUSTY_RUNTIME_TOTAL_SIZE   16 MEGABYTE

/*
 * ELF header.
 */

typedef struct {
	unsigned char	e_ident[EI_NIDENT];     /* File identification. */
	uint16_t	e_type;                 /* File type. */
	uint16_t	e_machine;              /* Machine architecture. */
	uint32_t	e_version;              /* ELF format version. */
	uint64_t	e_entry;                /* Entry point. */
	uint64_t	e_phoff;                /* Program header file offset. */
	uint64_t	e_shoff;                /* Section header file offset. */
	uint32_t	e_flags;                /* Architecture-specific flags. */
	uint16_t	e_ehsize;               /* Size of ELF header in bytes. */
	uint16_t	e_phentsize;            /* Size of program header entry. */
	uint16_t	e_phnum;                /* Number of program header entries. */
	uint16_t	e_shentsize;            /* Size of section header entry. */
	uint16_t	e_shnum;                /* Number of section header entries. */
	uint16_t	e_shstrndx;             /* Section name strings section. */
} elf64_ehdr_t;

/*
 * Program header.
 */

typedef struct {
	uint32_t	p_type;                 /* Entry type. */
	uint32_t	p_flags;                /* Access permission flags. */
	uint64_t	p_offset;               /* File offset of contents. */
	uint64_t	p_vaddr;                /* Virtual address in memory image. */
	uint64_t	p_paddr;                /* Physical address (not used). */
	uint64_t	p_filesz;               /* Size of contents in file. */
	uint64_t	p_memsz;                /* Size of contents in memory. */
	uint64_t	p_align;                /* Alignment in memory and file. */
} elf64_phdr_t;

/*
 * Dynamic structure.  The ".dynamic" section contains an array of them.
 */

typedef struct {
	uint64_t d_tag;                 /* Entry type. */
	union {
		uint64_t	d_val;          /* Integer value. */
		uint64_t	d_ptr;          /* Address value. */
	} d_un;
} elf64_dyn_t;

/* Relocations that need an addend field. */
typedef struct {
	uint64_t	r_offset;               /* Location to be relocated. */
	uint64_t	r_info;                 /* Relocation type and symbol index. */
	uint64_t	r_addend;               /* Addend. */
} elf64_rela_t;

/*
 * Symbol table entries.
 */

typedef struct {
	uint32_t	st_name;        /* String table index of name. */
	unsigned char	st_info;        /* Type and binding information. */
	unsigned char	st_other;       /* reserved (not used). */
	uint16_t	st_shndx;       /* Section index of symbol. */
	uint64_t	st_value;       /* Symbol value. */
	uint64_t	st_size;        /* Size of associated object. */
} elf64_sym_t;

boolean_t relocate_elf_image (uint64_t loadtime_addr, uint64_t runtime_addr,
        uint64_t *run_entry);

#endif
