# Bonus Assignment - Group 85 - EGOS -2000 Operating system
**Instructor**: Vivek Kumar

## Team Members
- **Aditya Kumar** (Roll No: 2024035)
- **Dipanshu** (Roll No: 2024196)
  
## Table of Content
- [Summary](#Summary)
- [Functionality](#Functionality)
- [Technical Implementation Details](#Technical-Implementation-Details)
- [How to Run](#How-to-Run)
- [Contribution](#Contribution)

## Summary
This project extends the **EGOS-2000** operating system by implementing user-space shell utilities and a kernel-space scheduler. The assignment is divided into two parts:
1. **Custom Shell Commands**: Implementation of `grep` and `wcl` using low-level EGOS system calls, bypassing standard C file libraries.
2. **MLFQ Scheduler**: A preemptive Multi-Level Feedback Queue scheduler to manage process execution based on priority levels and runtime behavior
   
## Functionality

**1. Shell Utilities (User Space)**
**wcl (Word Count - Lines)**: Accepts multiple filenames as arguments, reads the full content of each file, and prints the total number of newlines across all files
* **grep (Global Regular Expression Print)**: Accepts a search pattern and a filename. [cite_start]It searches the file line-by-line and prints any line containing the specified pattern

**2. MLFQ Scheduler (Kernel Space)**
* **Prioritization**: Maintains 5 priority levels (0-4), where Level 0 is the highest priority
* **Dynamic Adjustment**: Processes move down levels based on CPU usage time slices ($(level+1) \times 100$ ms).
* **Aging/Reset**: To prevent starvation, all processes are reset to Level 0 every 10 seconds or upon keyboard interrupts (for the Shell process)

## Technical Implementation Details

**1. Shell Commands (`wcl` & `grep`)**
To comply with the strict requirement of not using standard C file APIs (like `fopen`, `fgets`):
* **System Calls**: We utilized `dir_lookup` to retrieve inode numbers and `file_read` to access file content directly from the disk.
* **Block-Based Reading**: Since `file_read` retrieves data in fixed chunks (`BLOCK_SIZE`), we implemented a loop to read files block-by-block until the end of the file.
* **Manual Buffering**:
    * For `wcl`: We iterate through the buffer counting `\n` characters.
    * For `grep`: We manually reconstruct lines from the block buffer. If a line spans across two blocks, the logic preserves the partial line from the first block and appends the start of the second block before checking for the pattern using `strstr`.

## How to Run

**1. Compile the Project**
Navigate to the root `egos-2000` directory and run the following commands to build the kernel and user applications

```bash
# Clean previous builds
make clean

# Compile the OS and applications
make
```
### 2. Run QEMU Simulation
Launch the OS in the QEMU emulator:

```bash
make qemu
```
### 3. Boot Options
Once the emulator starts, you will be prompted to choose a memory translation mechanism.

* Enter `0` for page tables.

### 4. Testing Shell Commands
Once inside the EGOS shell (`➜ /home/yunhao`), you can test the commands as follows:
**Test wcl:**

```bash
wcl README
# Output: Prints line count (e.g., 20)
```
**Test grep:**

```bash
grep code README
# Output: Prints lines containing "code"
```
## Contribution
- All the work was collectively and equally done by both participants, Aditya and Depanshu.
### Biblography 
- EGOS-2000 Book: https://egos.fun/book/overview.html
- Bonus Assignment PDF: Section - A | OS Monsoon'2025'.
- Tutorial 7 - EGOS-2000 operating system
- AI Assistance - used for the debugging 
