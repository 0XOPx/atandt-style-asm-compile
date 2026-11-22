# AT&T Style ASM Compile

A Windows-first C++ command-line driver for assembling AT&T-style x86 assembly with LLVM and optionally linking the resulting object file into an executable.

## Requirements

- C++17 compiler
- LLVM tools: llvm-mc
- Clang driver: clang (needed for executable linking)

Both LLVM tools must be available on PATH, or supplied with --llvm-mc and --clang.

## Build

### MSYS2 UCRT64

```powershell
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp -o atandt-asm.exe
```

### LLVM Clang

```powershell
clang++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp -o atandt-asm.exe
```

## Usage

Assemble and link:

```powershell
.atandt-asm.exe hello.s
```

Choose the output executable:

```powershell
.atandt-asm.exe hello.s -o hello.exe
```

Generate only a COFF object:

```powershell
.atandt-asm.exe hello.s --no-link
```

Choose the target:

```powershell
.atandt-asm.exe hello.s --target x86_64-pc-windows-msvc
```

Specify LLVM tools:

```powershell
.atandt-asm.exe hello.s --llvm-mc "F:\Program Files\LLVM\bin\llvm-mc.exe" --clang "F:\Program Files\LLVM\bin\clang.exe"
```

## Initial scope

- AT&T x86-64 assembly through LLVM's assembler.
- Windows x64 COFF object output.
- Optional executable linking through the Clang driver.
- Configurable target triple and tool paths.
- Clear diagnostics and non-zero exit codes on failure.

LLVM performs instruction parsing, encoding, relocations, and object-file generation. This project provides the command-line workflow and build orchestration.

## License

MIT
