# Custom Command Line Shell
**CS-2006 — Operating Systems | Spring 2026**

## Group Members

| # | Name | ID | Role |
|---|------|----|------|
| 1 | Muhammad Maaz | 24K-0968 | Shell Core & Process Management |
| 2 | Fahad Zuberi | 24K-0717 | Built-ins, Pipes & Redirection |
| 3 | Muhammad Saad Khan | 24K-0680 | Signal Handling & Job Control |

---

## Project Overview

A fully functional interactive UNIX-like command-line shell implemented in C (C11 standard),  
demonstrating core OS concepts:
- Process creation & management (`fork`, `execvp`, `waitpid`)
- Inter-process communication (`pipe`, `dup2`)
- I/O redirection (`>`, `>>`, `<`)
- Signal handling (`SIGINT`, `SIGTSTP`, `SIGCHLD`)
- Job control (foreground, background `&`)
- Environment variable expansion (`$VAR`)
- Built-in commands

---

## Building

```bash
# Build
make

# Run the shell
make run
# or
./myshell

# Run smoke tests
make test

# Clean build artifacts
make clean
```

**Requirements:** GCC, GNU Make, Linux (Ubuntu 24.04 LTS recommended)

---

## Features

### Built-in Commands

| Command | Description |
|---------|-------------|
| `cd [dir]` | Change directory (defaults to `$HOME`) |
| `cd -` | Return to previous directory (`$OLDPWD`) |
| `pwd` | Print current working directory |
| `history [n]` | Show command history (optionally last n entries) |
| `help` | Display built-in command reference |
| `env` | List all environment variables |
| `export VAR=VALUE` | Set/export an environment variable |
| `unset VAR` | Remove an environment variable |
| `exit [code]` | Exit the shell with optional exit code |

### Shell Operators

| Operator | Example | Description |
|----------|---------|-------------|
| `\|` | `ls -l \| grep .c` | Pipe stdout of left into stdin of right |
| `>` | `echo hi > out.txt` | Redirect stdout (overwrite) |
| `>>` | `echo hi >> out.txt` | Redirect stdout (append) |
| `<` | `sort < data.txt` | Redirect stdin from file |
| `&` | `sleep 10 &` | Run in background |
| `$VAR` | `echo $HOME` | Environment variable expansion |

### Multi-stage Pipelines

```bash
myshell$ ls -l | grep ".c" | wc -l
myshell$ cat /etc/passwd | cut -d: -f1 | sort | uniq
```

### I/O Redirection

```bash
myshell$ echo "hello world" > output.txt
myshell$ cat < input.txt | tr a-z A-Z >> output.txt
```

### Background Execution

```bash
myshell$ sleep 5 &
[bg] pid 12345
myshell$           # shell returns immediately
```

### Signal Handling

| Signal | Key | Behaviour |
|--------|-----|-----------|
| `SIGINT` | Ctrl+C | Interrupts foreground process; shell continues |
| `SIGTSTP` | Ctrl+Z | Stops foreground process |
| `SIGCHLD` | (automatic) | Reaps finished background children |

---

## File Structure

```
myshell/
├── config.h       — Macros, student IDs, limits, ANSI colours
├── parser.h/.c    — Tokeniser, operator detection, Pipeline struct
├── builtins.h/.c  — cd, pwd, exit, history, help, env, export, unset
├── signals.h/.c   — sigaction() setup for SIGINT, SIGTSTP, SIGCHLD
├── redirect.h/.c  — dup2() for <, >, >>, pipe wiring
├── executor.h/.c  — fork/exec/waitpid, pipeline execution, bg jobs
├── main.c         — REPL loop, banner, prompt
├── Makefile       — Build automation + test target
└── README.md      — This file
```

---

## OS Concepts Demonstrated

| Concept | System Call / API | Where Used |
|---------|-------------------|------------|
| Process Creation | `fork()`, `execvp()` | Every external command |
| Process Synchronisation | `waitpid()`, `WIFEXITED()` | Foreground wait |
| Inter-Process Communication | `pipe()`, `dup2()` | `cmd1 \| cmd2` |
| File I/O Redirection | `open()`, `close()`, `dup2()` | `>`, `<`, `>>` |
| Signal Handling | `sigaction()`, `kill()` | Ctrl+C, Ctrl+Z, bg reap |
| Environment Variables | `getenv()`, `setenv()` | `$PATH`, `$HOME` |
| Directory Management | `chdir()`, `getcwd()` | `cd`, `pwd` |
| Job Control | `setpgid()`, `tcsetpgrp()` | Background & foreground |

---

## Example Session

```
myshell$ ls -l | grep ".c" | wc -l
6
myshell$ echo "CS-2006" > course.txt
myshell$ cat < course.txt
CS-2006
myshell$ echo "Spring 2026" >> course.txt
myshell$ cat course.txt
CS-2006
Spring 2026
myshell$ sleep 30 &
[bg] pid 5678
myshell$ history
   1  ls -l | grep ".c" | wc -l
   2  echo "CS-2006" > course.txt
   3  cat < course.txt
   4  echo "Spring 2026" >> course.txt
   5  cat course.txt
   6  sleep 30 &
   7  history
myshell$ exit
Goodbye!
```

---

## Implementation Notes

- All signal handlers use `sigaction()` (not deprecated `signal()`)
- `SIGCHLD` handler is async-signal-safe (uses `waitpid` with `WNOHANG`)
- Child processes reset signal dispositions to defaults before `exec()`
- Process groups ensure Ctrl+C/Ctrl+Z target the right process
- Parser handles single-quoted strings (no expansion) and double-quoted strings (env expansion)
- History uses a ring buffer capped at `MAX_HISTORY` (100) entries

---

*Project submitted for CS-2006 Operating Systems, Spring 2026.*  
*Muhammad Maaz (24K-0968) · Fahad Zuberi (24K-0717) · Muhammad Saad Khan (24K-0680)*