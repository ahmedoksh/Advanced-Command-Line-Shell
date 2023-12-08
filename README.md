# Advanced Command-Line Shell in C

This project is a custom shell implemented in C. It provides a command prompt where you can execute commands, with support for background calls and several built-in functions for managing background processes.

## Features

- Command Prompt: The shell provides a command prompt where you can type and execute commands.

- Background Calls: You can run commands in the background by prefixing them with `bg`.

- Background Process Management: The shell provides several built-in functions for managing background processes:

    - `bgkill`: Kills a background process. Usage: `bgkill <process_id>`

    - `bgstop`: Pauses a background process. Usage: `bgstop <process_id>`

    - `bgstart`: Resumes a paused background process. Usage: `bgstart <process_id>`


## Getting Started

### Prerequisites
- GCC compiler
- Make

### Building the Project
To build the project using the Makefile, run
```
make
``` 
### Running the Project
To run the shell
```
./man
```
