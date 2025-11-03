# Compilation and Execution Instructions

This guide outlines how to compile and run the project on a Linux system using the provided `Makefile`.

## Prerequisites

You need standard build tools installed on your Linux system, specifically `gcc` (the GNU C Compiler) and `make`.

## Usage

Navigate to the project directory in your terminal and use the following commands:

### Compile All Files

This command uses the `Makefile` to compile all source files and produce the executable `graph_app`.

```bash
make
```
### Run the Application

Execute the compiled program:

```bash
./graph_app
```
### Clean Project Files

This command removes all generated object files (*.o) and the main executable (graph_app).

```bash
make clean
```
