# Compilation Guide

This document describes the compilation pipeline and dependency validation steps required to build the PAXM engine object matrix across distinct operating system matrices.

## COMPILATION DEPENDENCIES
To compile this project, the host operating system must provide:
1. A C compiler (`cc` / `clang` / `gcc` / OpenWatcom `wcl386`)
2. Standard C runtime headers (`glibc`, `musl`, BSD `libc`, `ucrt` in Windows)
3. Standard system build utilities (`make`, `bmake`, or OpenWatcom `wmake`)

---

## DEPENDENCY INSTALLATION (DEBIAN / LINUX MINT / UBUNTU)
Initialize your system's package management engine and install the universal compilation utilities:

```sh
sudo apt update
sudo apt install build-essential clang
```
*Note: `clang` is optional, but available in case you want to use it instead of `gcc`.*

---

## DEPENDENCY INSTALLATION (ARCH LINUX)
Synchronize your system's rolling-release core packages and deploy the native base-devel toolkit:

```sh
sudo pacman -Syu
sudo pacman -S base-devel clang
```
*Note: `clang` is optional, but available in case you want to use it.*

---

## DEPENDENCY INSTALLATION (FREEBSD)
Synchronize the native package management database. The baseline Clang compiler and `bmake` engine are already built natively into the FreeBSD base operating system layer:

```sh
sudo pkg update
```

---

## BUILD INSTRUCTIONS (UNIX-LIKE SYSTEM)
1. Open your terminal environment (`sh` / `bash` / `zsh`) inside the root project directory.
2. Initialize and grant execution permissions to the configuration script:
   ```sh
   chmod +x ./configure
   ```
3. Run the configuration script to probe local system headers, locate the system compiler, and dynamically write a Makefile:
   ```sh
   ./configure
   ```
4. Execute the build automation engine to compile the final object target archive (`paxm.o`):
   ```sh
   make
   ```
5. To wipe compiled object files and restore the source tree to a clean slate (this also self-destructs the generated Makefile):
   ```sh
   make clean
   ```

---

## BUILD INSTRUCTIONS (WINDOWS VIA MSYS2 ENVIRONMENT)
1. Download and initialize the MSYS2 environment subsystem.
2. Select your preferred environment profile and deploy the corresponding package toolchains via the pacman subsystem:

   * **OPTION A: PURE LLVM / CLANG STACK**
     * Launch the "CLANG64" terminal profile environment window.
     * Synchronize and install the LLVM tools:
       ```sh
       pacman -Syu
       pacman -S mingw-w64-clang-x86_64-toolchain
       ```

   * **OPTION B: TRADITIONAL GNU / GCC STACK**
     * Launch the "UCRT64" terminal profile environment window.
     * Synchronize and install the GNU tools:
       ```sh
       pacman -Syu
       pacman -S mingw-w64-x86_64-toolchain
       ```

3. Execute the standard shell commands inside your project path:
   ```sh
   ./configure
   ```
   ```sh
   make
   ```
   The toolchain will compile a native standalone Windows object component file (`paxm.o`).

---

## BUILD INSTRUCTIONS (FREEDOS / DOS ENVIRONMENTS)
Bypass the shell configuration script entirely. Execute your compilation rules natively inside the OpenWatcom toolchain environment using the dedicated `WMAKE.WPJ` file:

```cmd
wmake -f WMAKE.WPJ
```

This compiles the code, outputting a clean 32-bit linear protected-mode DOS static archive library container (`paxm.lib`).

To purge the compiled object matrices and libraries under FreeDOS:
```cmd
wmake -f WMAKE.WPJ clean
```
---

## DEVELOPER LINKING TUTORIAL (COMPILING YOUR CODE WITH PAXM)

If you are building your own custom software (e.g., `my_game.c` or `player.c`) and want to link it directly with the compiled PAXM machine object matrix (`paxm.o`), execute the explicit host compiler instructions set forth below.

### UNIX-LIKE SYSTEMS
1. First, compile the PAXM core code into a raw binary object file using this project's configuration loop:
   ```sh
   ./configure && make
   ```
2. Link your custom application file directly with the resulting `paxm.o` machine object chunk:
   ```sh
   # Using the default system compiler:
   cc -Wall -Wextra -std=c99 my_game.c paxm.o -o my_game_app

   # Using the Clang compiler collection engine:
   clang -Wall -Wextra -std=c99 my_game.c paxm.o -o my_game_app
   ```

---

### WINDOWS ENVIRONMENTS VIA MSYS2 TOOLCHAINS
1. Run the local build configuration script inside your active MSYS2 terminal profile window path (`CLANG64` or `UCRT64`):
   ```sh
   ./configure && make
   ```
2. Feed your custom Windows application code file and the compiled `paxm.o` segment to your environment's compiler:
   ```sh
   # Inside the CLANG64 LLVM environment window:
   clang -Wall -Wextra -std=c99 my_game.c paxm.o -o my_game_app.exe

   # Inside the UCRT64 GNU GCC environment window:
   gcc -Wall -Wextra -std=c99 my_game.c paxm.o -o my_game_app.exe
   ```

*Note: If your downstream code is a graphical game window layout (like an SDL2 or raylib game) and you want to suppress the background command console pop-up entirely on launch, append the `-mwindows` flag to your compilation parameters.*

---

### FREEDOS / x86 DOS REAL-MODE COMPILERS (OPENWATCOM)
1. First, process the project rules inside your FreeDOS terminal workspace environment using the dedicated Watcom build tool script to archive the flat static library container:
   ```cmd
   wmake -f WMAKE.WPJ
   ```
2. Link the compiled static library container matrix (`paxm.lib`) straight into your native 32-bit linear protected-mode application executable:
   ```cmd
   wcl386 -od -zq -bt=dos -l=dos4g -std=c99 my_game.c paxm.lib -fe=my_game.exe
   ```
