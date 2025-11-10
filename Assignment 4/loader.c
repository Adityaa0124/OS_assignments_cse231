#include "loader.h"

#define PAGE_SIZE 4096

int pgfault_cnt = 0;
int pgalloc_cnt = 0;
size_t internal_frag = 0;

Elf32_Ehdr *ehdr = NULL;
Elf32_Phdr *phdr = NULL;
int fd = -1;
int seg_num = 0;

void segv_handler(int sig, siginfo_t *si, void *context)
{
  void *fault_addr = si->si_addr;
  pgfault_cnt++;

  for (int i = 0; i < seg_num; i++)
  {
    if (phdr[i].p_type == PT_LOAD &&
        (uintptr_t)fault_addr >= phdr[i].p_vaddr &&
        (uintptr_t)fault_addr < phdr[i].p_vaddr + phdr[i].p_memsz)
    {
      uintptr_t seg_start = phdr[i].p_vaddr;
      uintptr_t seg_end = seg_start + phdr[i].p_memsz;
      uintptr_t seg_file_end = seg_start + phdr[i].p_filesz;

      uintptr_t pg_start = (uintptr_t)fault_addr & ~(PAGE_SIZE - 1);
      uintptr_t pg_end = pg_start + PAGE_SIZE;

      uintptr_t off_in_seg = (uintptr_t)fault_addr - seg_start;
      uintptr_t pg_off_in_seg = off_in_seg & ~(PAGE_SIZE - 1);

      size_t off_in_file = phdr[i].p_offset + pg_off_in_seg;

      int prot = PROT_READ | PROT_WRITE | PROT_EXEC;

      munmap((void *)pg_start, PAGE_SIZE);

      void *mapped = mmap((void *)pg_start, PAGE_SIZE,
                          prot,
                          MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

      if (mapped == MAP_FAILED)
      {
        perror("segv_handler mmap failed.");
        exit(1);
      }
      pgalloc_cnt++;

      size_t bytes_to_read = 0;
      if (pg_start < seg_file_end)
      {
        if (pg_end <= seg_file_end)
        {
          bytes_to_read = PAGE_SIZE;
        }
        else
        {
          bytes_to_read = seg_file_end - pg_start;
        }
      }
      if (bytes_to_read > 0)
      {
        if (lseek(fd, off_in_file, SEEK_SET) < 0)
        {
          perror("segv_handler lseek failed.");
          exit(1);
        }
        ssize_t read_bytes = read(fd, mapped, bytes_to_read);
        if (read_bytes < 0)
        {
          perror("segv_handler read failed.");
          exit(1);
        }
        if ((size_t)read_bytes != bytes_to_read)
        {
          fprintf(stderr, "Warning: incomplete read in segv_handler\n");
        }
      }
      if (bytes_to_read < PAGE_SIZE)
      {
        memset((char *)mapped + bytes_to_read, 0, PAGE_SIZE - bytes_to_read);
      }
      size_t bytes_used_in_pg = PAGE_SIZE;
      if (pg_end > seg_end)
      {
        bytes_used_in_pg = seg_end - pg_start;
      }
      internal_frag += (PAGE_SIZE - bytes_used_in_pg);
      return;
    }
  }
  fprintf(stderr, "Segmentation fault at %p (not in any loadable segment)\n", fault_addr);
  exit(1);
}

void loader_cleanup()
{
  if (ehdr)
    free(ehdr);
  if (phdr)
    free(phdr);
  if (fd != -1)
    close(fd);
}

void load_and_run(char **exe)
{
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = segv_handler;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);

  if (sigaction(SIGSEGV, &sa, NULL) < 0)
  {
    perror("sigaction failed");
    exit(1);
  }
  fd = open(*exe, O_RDONLY);
  if (fd < 0)
  {
    perror("failed to open ELF file");
    exit(1);
  }
  ehdr = malloc(sizeof(Elf32_Ehdr));
  if (!ehdr)
  {
    perror("malloc failed for ELF header");
    exit(1);
  }

  if (lseek(fd, 0, SEEK_SET) < 0)
  {
    perror("lseek failed");
    exit(1);
  }

  if (read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr))
  {
    perror("failed to read ELF header");
    exit(1);
  }
  if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
      ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
      ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
      ehdr->e_ident[EI_MAG3] != ELFMAG3)
  {
    fprintf(stderr, "not a valid ELF file\n");
    exit(1);
  }

  phdr = malloc(ehdr->e_phnum * sizeof(Elf32_Phdr));
  if (!phdr)
  {
    perror("malloc failed for program headers");
    exit(1);
  }

  if (lseek(fd, ehdr->e_phoff, SEEK_SET) < 0)
  {
    perror("lseek failed for program headers");
    exit(1);
  }

  size_t phdr_size = ehdr->e_phnum * sizeof(Elf32_Phdr);
  if (read(fd, phdr, phdr_size) != (ssize_t)phdr_size)
  {
    perror("failed to read program headers");
    exit(1);
  }

  seg_num = ehdr->e_phnum;

  void *entry_ptr = (void *)(uintptr_t)ehdr->e_entry;

  if (!entry_ptr)
  {
    fprintf(stderr, "invalid entry point address\n");
    exit(1);
  }

  int (*_start)() = (int (*)())entry_ptr;

  int result = _start();

  printf("User _start return value = %d\n", result);
  printf("Total page faults handled: %d\n", pgfault_cnt);
  printf("Total page allocations:    %d\n", pgalloc_cnt);
  printf("Total internal fragmentation: %.2f KB\n", internal_frag / 1024.0);
}

int main(int argc, char **argv)
{
  if (argc != 2)
  {
    printf("incorrect usage!");
    exit(1);
  }

  load_and_run(&argv[1]);

  loader_cleanup();
  return 0;
}