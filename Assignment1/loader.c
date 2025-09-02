#include "loader.h"

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
int fd;
void **mapped_segments = NULL; // track or to remember the base address returned by mmap() for each phdr index
int num_segments = 0; // this is used to store the no. of entries in mapped_segment

/*
 * release memory and other cleanups
 */
void loader_cleanup() {
  if (mapped_segments){
    for (int i = 0; i < num_segments; i++){
      if(mapped_segments[i]){
        munmap(mapped_segments[i],phdr[i].p_memsz);
      }
    }
    free(mapped_segments);
    mapped_segments = NULL;

  }

  if(ehdr){
    free(ehdr);
    ehdr = NULL;
  } 
  if(phdr){
    free(phdr);
    phdr = NULL;
    }
  if(fd != -1){
        close(fd);
        fd = -1;
    }
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char** exe) {
  
  fd = open(*exe, O_RDONLY);
  // 1. Load entire binary content into the memory from the ELF file.

  // error check
  if(fd < 0){
    perror("not able to open file");
    exit(1);
  }
    ehdr = malloc(sizeof(Elf32_Ehdr));
    lseek(fd,0,SEEK_SET); // this is telling me the start of the file
    if(read(fd,ehdr,sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)){
      perror("error reading the elf header");
      exit(1);
    }
    
    // if(ehdr == NULL){
    //   perror("eror i allocating the memory for elf header\n");
    //   close(fd);
    //   exit(1);
    // };

// 2 - a - loading the program headers

  phdr = malloc(ehdr->e_phnum*sizeof(Elf32_Phdr)); // loading and allocating the size mem program header
  lseek(fd, ehdr->e_phoff, SEEK_SET); // lseek will move the pointer to from offset seek_set
  if(read(fd,phdr,ehdr->e_phnum * sizeof(Elf32_Phdr)) != ehdr->e_phnum * sizeof(Elf32_Phdr) ){
      perror("failed to read PHDRs");
      exit(1);
  }

// 2-b-tracking the program header remembering the to no. of entries and in mapped segment
  mapped_segments = malloc(sizeof(void*)* ehdr->e_phnum);
  num_segments = ehdr->e_phnum; // saving the no. of entries of mapped segement
  // 2. Iterate through the PHDR table and find the section of PT_LOAD 
  
  //    type that contains the address of the entrypoint method in fib.c
  for(int i = 0; i<ehdr->e_phnum; i++){
    
    if(phdr[i].p_type == PT_LOAD){
      void *virtual_mem = mmap(NULL , phdr[i].p_memsz, PROT_READ | PROT_WRITE | PROT_EXEC , MAP_ANONYMOUS | MAP_PRIVATE,0,0);
      if(virtual_mem == MAP_FAILED){
        perror("failed to mmap");
        exit(1);
      }
     
      lseek(fd,phdr[i].p_offset,SEEK_SET);
      if(read(fd,virtual_mem,phdr[i].p_filesz)!= phdr[i].p_filesz) {
        perror(" failed to read the segment");
        exit(1);
      }
       // zero out the remaining memory for .bss which is not initialized
       if(phdr[i].p_memsz > phdr[i].p_filesz){
        memset(virtual_mem + phdr[i].p_filesz, 0 , phdr[i].p_memsz - phdr[i].p_filesz);
       }
       mapped_segments[i] = virtual_mem;
    }
    else{
      mapped_segments[i] = NULL;
    }
  }
  // 3. Allocate memory of the size "p_memsz" using mmap function 
  //    and then copy the segment content
  // 4. Navigate to the entrypoint address into the segment loaded in the memory in above step
    void *entry_ptr = NULL;
    for(int i = 0; i <ehdr->e_phnum; i++){
      if(phdr[i].p_type == PT_LOAD && ehdr->e_entry >= phdr[i].p_vaddr && ehdr->e_entry < phdr[i].p_vaddr + phdr[i].p_memsz){
        entry_ptr = mapped_segments[i]+(ehdr->e_entry - phdr[i].p_vaddr);
        break;
      }
    }
    if(!entry_ptr){
      fprintf(stderr, "entry point does not found in any PT_load segment\n");
      exit(1);
    }
  // 5. Typecast the address to that of function pointer matching "_start" method in fib.c.
  int(*_start)() = (int(*)())entry_ptr;
  // 6. Call the "_start" method and print the value returned from the "_start"
  int result = _start();
  printf("User _start return value = %d\n",result);
}

int main(int argc, char** argv) 
{
  if(argc != 2) {
    printf("Usage: %s <ELF Executable> \n",argv[0]);
    exit(1);
  }
  // 1. carry out necessary checks on the input ELF file
  // 2. passing it to the loader for carrying out the loading/execution
  load_and_run_elf(&argv[1]);
  // 3. invoke the cleanup routine inside the loader  
  loader_cleanup();
  return 0;
}