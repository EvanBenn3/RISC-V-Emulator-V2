# RISC-V-Emulator-V2
This is a RISC-V Emulator made by a 17 year old. It's architecture is currently only RV32IM with F (and maybe D) being planned (hardfloat).

## Current Features
-ELF Parsing

-Interrupts

-Privilage modes (Machine, Superviser and User)

-Traps and Trap handeling

-PLIC

-UART

-Interractive Debugger

-Lazy allocation of memory for optimization

## How to compile project
in Powershell on Windows 10/11 using GCC from the project root:

```bash
gcc -O3 src\elf.c src\memory.c src\cpu.c src\peripheral.c src\main.c -o bin\emu.exe
```

replace bin with the directory of the binary executable if needed

## Obtaining the RISC-V Dev Tools for Windows 10/11
Install NodeJs if you havent

Install xpacks ```npm install --global xpm@latest```

Install the RISC-V Dev Tools ```xpm install --global @xpack-dev-tools/riscv-none-elf-gcc@latest --verbose```

Find the bin directory of the Dev Tools and add them to PATH if they aren't already there

Check functionality through ```riscv-none-elf-gcc --version```

## How to compile a program
Using Riscv Dev Tools:

```bash
riscv-none-elf-as -c -mabi=ilp32f *.s -o asm.o // all assembly files
riscv-none-elf-gcc -c -ffreestanding -nostdlib -march=rv32imf -mabi=ilp32f *.c -o c.o // all C files
riscv-none-elf-ld -T link.ld asm.o c.o -o program.elf 
```

A linker file like in the "hello_world" test program is required

## How to run a program
Unfortunetely, the emulator currently hard codes the directory to the program elf of choice. To run a program, it is necessary to change this directory for the directory of choice

## How to help
By being kind, cooporative and constructive, pointing out bugs and suggesting new features

It is also recommended to star the repository if you desire, or also fork and merge the project
