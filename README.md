# UNIX Shell Project

This project is a simple UNIX-like shell implemented in C for educational purposes, as part of the Operating Systems course at UMA (Universidad de Málaga).

## Features
- Command parsing and execution (foreground and background)
- Job control (list, add, delete, bring to foreground/background)
- Signal handling (SIGCHLD, SIGINT, SIGTSTP, etc.)
- Built-in commands: `cd`, `jobs`, `fg`, `bg`
- Input/output redirection (`<`, `>`)
- Process group and terminal management

## File Structure
- `Shell_project.c`: Main shell implementation
- `job_control.c` / `job_control.h`: Job control module (job list, signal handling, status analysis)

## Compilation
To compile the shell, use:

```sh
gcc Shell_project.c job_control.c -o Shell
```

## Usage
Run the shell with:

```sh
./Shell
```

Type commands as you would in a normal shell. Use `^D` (Ctrl+D) to exit.

### Built-in Commands
- `cd <dir>`: Change directory
- `jobs`: List background and stopped jobs
- `fg [n]`: Bring job n (default 1) to foreground
- `bg [n]`: Continue job n (default 1) in background

### Redirections
- `command < input.txt > output.txt`
