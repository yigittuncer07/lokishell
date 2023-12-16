<font color="red">                                                                                                                                                                                                                                                                                                                          
░█░░░█▀█░█░█░▀█▀░█▀▀░█░█░█▀▀░█░░░█░░                                          .                                                                                                                                                                                                               
░█░░░█░█░█▀▄░░█░░▀▀█░█▀█░█▀▀░█░░░█░░                                                                                                                                                                                                   
░▀▀▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀░▀░▀▀▀░▀▀▀░▀▀▀                                                                                                                                                                                                                                                              
</font>                     

LokiShell is a simple Unix shell with additional features like bookmarks, I/O redirection, and background process handling.

## Features

- **Bookmarks**: Save and execute frequently used commands with named bookmarks.
- **I/O Redirection**: Redirect input and output for commands using `<`, `>`, `>>`, and `2>` operators.
- **Background Processes**: Run commands in the background by appending `&` at the end.

## Usage

### Basic Commands

- **exit**: Exit LokiShell. Use `exit` to terminate the shell.

### Bookmarks

- **bookmark -l**: List all saved bookmarks.
- **bookmark -i <index>**: Execute the bookmark at the specified index.
- **bookmark -d <index>**: Delete the bookmark at the specified index.
- **bookmark <command>**: Add a new bookmark.

### I/O Redirection

Use the following operators for I/O redirection:

- `<`: Redirect input from a file.
- `>`: Redirect output to a file (overwriting existing content).
- `>>`: Redirect output to a file (appending to existing content).
- `2>`: Redirect error output to a file.

### Background Processes

Append `&` at the end of a command to run it in the background.

## Building and Running

To compile and run LokiShell, use the following commands:

```bash
gcc lokishell.c -o lokishell
./lokishell
