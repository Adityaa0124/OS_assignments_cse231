# SimpleLoader
SimpleLoader is a program for loading an ELF 32-bit executable in plain-C 
without using any library APIs available for manipulating ELF files, without using os's default loader.
This README provides a concise overview of the loader's functionality and usage.
## Steps to use the SimpleLoader
**Inside the linux terminal**
1. Using make all command to compile loader.c and the test case file:
   ```
   make all
   ```
   then run the compiled loader program with the path to the ELF executable as a command-line
   argument:
   ```
   ./loader fib
   ```
3. Using gcc commands manually in the terminal:
```
gcc -m32 -o loader loader.c
```
```
gcc  -m32 -no-pie -nostdlib -o fib fib.c
```
```
./loader fib
```
3. To use make command, run this command on your terminal:
```
sudo apt install make
```
## SimpleLoader implementation
1. **Open the ELF file:**
   The loader uses open() to open the given ELF file in read-only mode. If the file cannot be opened, the loader prints an error and exits.

2. **Read the ELF header:**
   The ELF header (Elf32_Ehdr) contains important content, such as:
- Program header offset (e_phoff)
- Number of program headers (e_phnum)
- Entry point address (e_entry)
- The loader reads this header first.

4. **Read the Program Headers:**
   Program headers (Elf32_Phdr) describe the segments that need to be loaded into memory. The loader reads all program headers into memory using lseek() to move to the correct file offset.

5. **Load PT_LOAD Segments into Memory:**
   Only segments of type PT_LOAD are actually loaded.
   For each loadable segment:
   Memory is allocated with mmap() using read, write, and execute permissions. The segment data from the ELF file (p_filesz bytes) is read into this memory.

6. **Locate the Entry Point:**
   The ELF header specifies the program’s entry point (e_entry). The loader checks which PT_LOAD segment contains this address. It then calculates the exact pointer inside the mapped memory where execution should begin.

7. **Jump to the Entry Point:**
   The entry address is cast to a function pointer _ _start_ . The loader then calls this function, effectively transferring control to the loaded program. When the program finishes, its return value is printed.

8. **Cleanup routine:**
   In the cleanup, allocated ELF and program headers are freed, the file descriptor is closed and the mapped segments are also unmapped.

**Bibliography**
   - Chatgpt
   - Gemini
   - Lecture slides
   - Tutorial slides
   - https://man7.org/linux/man-pages/man5/elf.5.html
   - https://wiki.osdev.org/ELF_Tutorial
   Using the above sources we learned how to use mmap, munmap, cleared our minor doubts during the
assignment. Also, we had to use filesz to tell the system how many bytes to read while reading from the ELF file to the memory. 
