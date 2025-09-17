# SimpleShell

SimpleShell is a Unix-like command-line interpreter implemented in **C** from scratch as part of the **CSE 231: Operating Systems** course assignment.  
This project demonstrates the use of system calls such as `fork()`, `exec()`, `wait()`, and `pipe()` to create a working shell that can execute commands, handle arguments, and support piping and history.

---

## Steps to use the SimpleShell

### 1. Compile the shell
Using `make`:
```bash
make shell
```
Or manually with `gcc`:

```bash
gcc simpleshell.c -o simpleshell
```

### 2. Run the shell
```bash
./simpleshell
```
After running, the shell will show a prompt where you can start typing commands.

## SimpleShell Implementation

### 1. **Read Command**
- The shell runs in an infinite loop, continuously waiting for user input.  
- Each loop iteration begins with displaying the **prompt** (`meowshell:~$  ` in our case).  
- Input is read using `getline()`, and the trailing newline is stripped off before processing.  
- If the user presses **Ctrl+D** (EOF), the shell exits gracefully.

### 2. **Parse Command**
- Input lines are first split on the pipe character (`|`) to identify potential pipelines.  
- Currently, the first command in the pipeline is parsed and executed.  
- Each command is further split into tokens (separated by spaces/tabs).  
- These tokens are stored in `argv[]` format to be passed to `execvp()`.  
- Restrictions:  
  - Quotes (`" "`) and backslashes (`\`) are **not supported**.  
  - Commands and arguments must be space-separated.

### 3. **Execute Command**
- Built-in commands are handled directly in the shell:  
  - `cd`: Changes the working directory (defaults to `$HOME` if no argument is given).  
  - `history`: Displays a numbered list of all commands executed in the current session.  
- For all other commands:  
  - A child process is created using `fork()`.  
  - The child executes the command using `execvp()`.  
  - The parent process waits for the child to finish using `waitpid()`.  
- If `execvp()` fails, an error message is displayed.

### 4. **Supported Commands**
Since the shell uses `execvp()`, it can execute most Linux commands that exist in the system’s `$PATH`.  
Examples include:  
```bash
ls
ls /home
echo hello world
wc -l fib.c
grep printf helloworld.c
./fib 40
./helloworld
sort fib.c
uniq file.txt
cat fib.c | wc -l
cat helloworld.c | grep print | wc -l
```
### 5. **History Feature**
-The shell maintains a history of executed commands in a fixed-size array (`history[100]`).
-Each entry stores:
 - The command string
 - Process ID (pid)
 - Start time
 - Duration of execution
-The history built-in command prints out the list of past commands.

### 6. Pipes
- Commands are split on the pipe (|), and parsing is in place for multiple commands.
- Full multi-command pipe is supported and executed using connecting processes with pipe() and dup2().

### 7. Termination

- The shell handles termination via Ctrl+C (SIGINT).
- On exit, it performs cleanup and prints a summary of all commands executed in the session and prints the history of all commands.
- Example exit output:
```bash
Exiting meowshell...
ls -l (pid=1234, ran at Mon Sep 16 21:00:00 2025, took 0.05 sec)
echo hello (pid=1235, ran at Mon Sep 16 21:00:02 2025, took 0.00 sec)
```
**Bibliography**
   - Chatgpt
   - Geekforgeeks
   - Lecture slides
   - Tutorial slides
- Using the above sources we learned how to use fork, exec, getline, dup, strtok, getset and cleared our minor doubts during the assignment.









