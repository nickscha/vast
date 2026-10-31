# vast
A C89 standard compliant, nostdlib (no C Standard Library) Virtual Adaptive Sparse Tiles (VAST).

> [!WARNING]
> THIS PROJECT IS A WORK IN PROGRESS! ANYTHING CAN CHANGE AT ANY MOMENT WITHOUT ANY NOTICE! USE THIS PROJECT AT YOUR OWN RISK!

## Download the executable
<p align="center">
  <a href="https://github.com/nickscha/vast/releases/latest/download/win32_vast_x64.exe">
    <img src="https://img.shields.io/badge/Windows%20x64-green?style=flat-square" alt="Windows x64">
  </a>
  <a href="https://github.com/nickscha/vast/releases/latest/download/win32_vast_arm.exe">
    <img src="https://img.shields.io/badge/Windows%20ARM-green?style=flat-square" alt="Windows ARM">
  </a>
  <a href="https://github.com/nickscha/vast/releases">
    <img src="https://img.shields.io/github/v/release/nickscha/vast?style=flat-square&color=blue" alt="Latest Release">
  </a>
  <a href="https://github.com/nickscha/vast/releases">
    <img src="https://img.shields.io/github/downloads/nickscha/vast/total?style=flat-square&color=brightgreen" alt="Downloads">
  </a>
  <a href="https://opensource.org/licenses/MIT">
    <img src="https://img.shields.io/badge/License-MIT-yellow.svg?style=flat-square" alt="License">
  </a>
  <img src="https://img.shields.io/badge/Standard-C89-orange?style=flat-square" alt="C Standard">
  <img src="https://img.shields.io/badge/nolib-nostdlib-lightgrey?style=flat-square" alt="nostdlib">
</p>

## Features
- **C89 compliant** — portable and legacy-friendly  
- **nostdlib** — no dependency on the C Standard Library
- **Zero dependencies** — own win32 layer implementation, own OpenGL context creation, ...
- **Strict compilation** — built with aggressive warnings & safety checks  

## Quick Start

### Download Windows Executable (Prebuilt)

Download the latest Windows executable for **X64** or **ARM** from the **Latest Release**:
- https://github.com/nickscha/vast/releases/latest

### Building From Source

Clone the repository and build manually.
Either GCC or Clang and LLD linker is required to run the build script.

```bash
git clone https://github.com/nickscha/vast.git
cd vast
win32_vast_build.bat
```

You can now run the `win32_vast.exe` program.
