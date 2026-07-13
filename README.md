# RISC-V-Emulator-V2
This is a RISC-V Emulator made by a 17 year old. It's architecture is currently only RV32IM with F (and maybe D) being planned (hardfloat).

## Current Features
-ELF Parsing

-Interrupts

-Privilege modes (Machine, Superviser and User)

-Traps and Trap handling

-PLIC

-UART

-Interactive Debugger

-Lazy allocation of memory for optimization

## How to compile project
In Powershell on Windows 10/11 using GCC from the project root:

```bash
gcc -O3 src\elf.c src\memory.c src\cpu.c src\peripheral.c src\main.c -o bin\emu.exe
```

Replace `bin` with the directory of the binary executable if needed

## Obtaining the RISC-V Dev Tools for Windows 10/11
Install NodeJs if you havent

Install xpacks ```npm install --global xpm@latest```

Install the RISC-V Dev Tools ```xpm install --global @xpack-dev-tools/riscv-none-elf-gcc@latest --verbose```

Find the bin directory of the Dev Tools and add them to PATH if they aren't already there

Check functionality through ```riscv-none-elf-gcc --version```

## How to use the debugger
In your program files, add an `ebreak` instruction to anywhere you desire, this will trigger the debug handler when that instruction is hit

For use, there are 7 commands:

```regdump``` dumps the entire register file as 8 digit hex and decimal

```memdump low high``` dumps a region of memory from the low byte to the high byte

```floatdump``` dumps all floating point registers as 8 digit hex and floating point

```csrdump csr``` dumps a specific csr register depending on which csr is chosen

```statedump``` dumps the current privilage mode of the hart

```step``` exits the forever while loop without disabling debug mode, allowing for debug mode to continue to the next instruciton

```quit``` fully exits out of debug mode and continues normal execution

## How to compile a program
Using Riscv Dev Tools:

```bash
riscv-none-elf-as -c -mabi=ilp32f *.s -o asm.o // all assembly files
riscv-none-elf-gcc -c -ffreestanding -nostdlib -march=rv32imf -mabi=ilp32f *.c -o c.o // all C files
riscv-none-elf-ld -T link.ld asm.o c.o -o program.elf 
```

A linker file like in the "hello_world" test program is required

## How to run a program
Running a program is simple:
In powershell with GCC:
```bash
./emu.exe -k"program.elf" -m1M
```
The `-k` flag is the program elf executable, the path to the elf file must be in quotations

The `-m` flag is the maximum memory size of the emulator instance, use K, M or G for prefixes

## How to help
By being kind, cooporative and constructive, pointing out bugs suggesting new features, and writing issues

It is also recommended to star the repository if you desire, or also fork and merge the project
