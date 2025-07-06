#include "elfloader.h"
#include "../../config.h"
#include "../../fs/file.h"
#include "../../kernel/kernel.h"
#include "../../memory/heap/kheap.h"
#include "../../memory/memory.h"
#include "../../memory/paging/paging.h"
#include "../../status.h"
#include "../../string/string.h"
#include "elf.h"
#include <stdbool.h>
#include <stdint.h>

const char elf_signature[] = {0x7f, 'E', 'L', 'F'};

static bool elf_valid_signature(void *buffer) {
  // check for the elf signature match
  return memcmp(buffer, (void *)elf_signature, sizeof(elf_signature)) == 0;
}

static bool elf_valid_class(struct elf_header *header) {
  // We only support 32 bit binaries.
  return header->e_ident[EI_CLASS] == ELFCLASSNONE ||
         header->e_ident[EI_CLASS] == ELFCLASS32;
}

static bool elf_valid_encoding(struct elf_header *header) {
  // No support for MSB
  return header->e_ident[EI_DATA] == ELFDATANONE ||
         header->e_ident[EI_DATA] == ELFDATA2LSB;
}

static bool elf_is_executable(struct elf_header *header) {
  // only support for executable elf files as of now
  return header->e_type == ET_EXEC &&
         header->e_entry >= LIEBE_OS_PROGRAM_VIRTUAL_ADDRESS;
}

static bool elf_has_program_header(struct elf_header *header) {
  return header->e_phoff != 0;
}

// some getter methods
void *elf_memory(struct elf_file *file) { return file->elf_memory; }

struct elf_header *elf_header(struct elf_file *file) {
  return file->elf_memory;
}

struct elf32_shdr *elf_sheader(struct elf_header *header) {
  return (struct elf32_shdr *)((uintptr_t)header + header->e_shoff);
}

struct elf32_phdr *elf_pheader(struct elf_header *header) {
  if (header->e_phoff == 0) {
    return 0;
  }

  return (struct elf32_phdr *)((uintptr_t)header + header->e_phoff);
}

struct elf32_phdr *elf_program_header(struct elf_header *header, int index) {
  return &elf_pheader(header)[index];
}

struct elf32_shdr *elf_section(struct elf_header *header, int index) {
  return &elf_sheader(header)[index];
}

char *elf_str_table(struct elf_header *header) {
  return (char *)header + elf_section(header, header->e_shstrndx)->sh_offset;
}

void *elf_virtual_base(struct elf_file *file) {
  return file->virtual_base_address;
}

void *elf_virtual_end(struct elf_file *file) {
  return file->virtual_end_address;
}

void *elf_phys_base(struct elf_file *file) {
  return file->physical_base_address;
}

void *elf_phys_end(struct elf_file *file) { return file->physical_end_address; }

int elf_validate_loaded(struct elf_header *header) {
  return (elf_valid_signature(header) && elf_valid_class(header) &&
          elf_valid_encoding(header) && elf_has_program_header(header))
             ? OK
             : -EINFORMAT;
}

int elf_process_phdr_pt_load(struct elf_file *elf_file,
                             struct elf32_phdr *phdr) {
  if (elf_file->virtual_base_address >= (void *)(uintptr_t)phdr->p_vaddr ||
      elf_file->virtual_base_address == 0x00) {
    elf_file->virtual_base_address = (void *)(uintptr_t)phdr->p_vaddr;
    elf_file->physical_base_address = elf_memory(elf_file) + phdr->p_offset;
  }

  unsigned int end_virtual_address = phdr->p_vaddr + phdr->p_filesz;
  if (elf_file->virtual_end_address <=
          (void *)(uintptr_t)(end_virtual_address) ||
      elf_file->virtual_end_address == 0x00) {
    elf_file->virtual_end_address = (void *)(uintptr_t)end_virtual_address;
    elf_file->physical_end_address =
        elf_memory(elf_file) + phdr->p_offset + phdr->p_filesz;
  }
  return 0;
}

int elf_process_pheader(struct elf_file *elf_file_obj,
                        struct elf32_phdr *phdr) {
  int res = 0;
  switch (phdr->p_type) {
  case PT_LOAD:
    // only load type is supported now
    res = elf_process_phdr_pt_load(elf_file_obj, phdr);
    break;
  }
  return res;
}

int elf_process_pheaders(struct elf_file *elf_file_obj) {
  int res = 0;
  struct elf_header *header = elf_header(elf_file_obj);
  for (int i = 0; i < header->e_phnum; i++) {
    struct elf32_phdr *phdr = elf_program_header(header, i);
    res = elf_process_pheader(elf_file_obj, phdr);
    if (res < 0) {
      break;
    }
  }
  return res;
}

int elf_process_loaded(struct elf_file *elf_file_obj) {
  int res = 0;
  struct elf_header *header = elf_header(elf_file_obj);
  res = elf_validate_loaded(header);
  if (res < 0) {
    // invalid elf file
    goto exit_fn;
  }

  res = elf_process_pheaders(elf_file_obj);
  if (res < 0) {
    goto exit_fn;
  }

exit_fn:
  return res;
}

int elf_load(const char *filename, struct elf_file **file_out) {
  struct elf_file *elf_file_obj = kzalloc(sizeof(struct elf_file));

  // read the file
  int fd = 0;
  int res = fopen(filename, "r");

  if (res <= 0) {
    goto exit_fn;
  }

  fd = res;

  // read the stats for the size and meta
  struct file_stat stat;
  res = fstat(fd, &stat);

  if (res < 0) {
    goto exit_fn;
  }

  elf_file_obj->elf_memory = kzalloc(stat.filesize);
  res = fread(elf_file_obj->elf_memory, stat.filesize, 1, fd);
  if (res < 0) {
    goto exit_fn;
  }

  res = elf_process_loaded(elf_file_obj);

  if (res < 0) {
    goto exit_fn;
  }

  *file_out = elf_file_obj;

exit_fn:
  fclose(fd);
  return res;
}

void elf_close(struct elf_file *file) {
  if (!file)
    return;

  kfree(file->elf_memory);
  kfree(file);
}

void *elf_phdr_phys_address(struct elf_file *file, struct elf32_phdr *phdr) {
  return elf_memory(file) + phdr->p_offset;
}
