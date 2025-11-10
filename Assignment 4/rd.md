# SimpleSmartLoader
SimpleSmartLoader is an enhanced ELF 32-bit executable loader written entirely in plain C, designed to simulate a lightweight OS-level loader. It extends the basic SimpleLoader by introducing lazy loading, smart memory mapping, and better resource management, without using any library APIs or the operating system’s default loader.
This README provides a concise yet comprehensive overview of SimpleSmartLoader’s functionality, usage, and improvements.
## Steps to Use SimpleSmartLoader
**Inside the linux terminal**
1. Using make all command to compile loader.c and the test cases files:
   ```
   make all
   ```
   then run the compiled loader program with the path to the ELF executable as a command-line
   argument:
   ```
   ./loader <ELF testfile name>
   ```
3. Using gcc commands manually in the terminal:
```
gcc -m32 -o loader loader.c
```
```
gcc  -m32 -no-pie -nostdlib -o <testcase file> <testcase file>.c
```
```
./loader <ELF testfile name>
```
3. To use make command, run this command on your terminal:
```
sudo apt install make
```
## SimpleSmartLoader Implementation
1. **Open the ELF file:**
  Uses open() to read the ELF file in read-only mode.

Validates the ELF magic bytes to ensure it’s a proper ELF-32 file.

If invalid, prints an error and exits safely.

2. **Read the ELF header:**
   The ELF header (Elf32_Ehdr) contains important content, such as:
- Program header offset (e_phoff)
- Number of program headers (e_phnum)
- Entry point address (e_entry)
- These fields are crucial for understanding how the file is laid out in memory.

3. **Read the Program Headers:**
   Program headers (Elf32_Phdr) describe the segments that need to be loaded into memory. The loader reads all program headers into memory using lseek() to move to the correct file offset.

4. **Smart Segment Loading**
   Lazy Loading (SimpleSmartLoader feature)

In lazy loading, segments are not fully loaded right away.

Instead, the loader:

Maps only the address space for each segment with the correct permissions.

Defers reading the segment contents from the ELF file until that segment is first accessed.

When the program touches a page that isn’t yet loaded, a page fault (or a simulated fault) triggers the loader to bring that segment (or part of it) into memory on demand.
5. **Loading Segments (PT_LOAD)**

For each PT_LOAD segment:

Allocates memory using mmap() with proper permissions (read, write, execute).

Loads only p_filesz bytes from the ELF file, zero-fills the rest (p_memsz - p_filesz) for uninitialized data like .bss.

Implements error handling for failed mappings or reads.
6. **Locate the Entry Point:**
   The ELF header specifies the program’s entry point (e_entry). The loader checks which PT_LOAD segment contains this address. It then calculates the exact pointer inside the mapped memory where execution should begin.

7. **Jump to the Entry Point:**
   The entry address is cast to a function pointer _ _start_ . The loader then calls this function, effectively transferring control to the loaded program. When the program finishes, its return value is printed.

8. **Cleanup routine:**
   In the cleanup, allocated ELF and program headers are freed, the file descriptor is closed and the mapped segments are also unmapped.
Any dynamically allocated ELF structures are freed.

Ensures zero memory leaks and clean resource release.

**Bibliography**
   - Chatgpt
   - Lecture slides
   - https://man7.org/linux/man-pages/man5/elf.5.html
   - https://wiki.osdev.org/ELF_Tutorial
- These resources helped clarify ELF structure, lazy mapping concepts, and efficient use of memory-mapped I/O.
