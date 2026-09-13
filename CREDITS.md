# Acknowledgments

We would like to thank these people, committees, and projects for their hard work:

* **`The Standard MIDI Specification Model (MIDI 1.0)`**
  * **Architects:** Dave Smith (Sequential Circuits), Ikutaro Kakehashi (Roland), and the MIDI Manufacturers Association (1983).
  * Pioneered the universal digital musical instrument interface paradigm, status byte command configurations, and delta-tick event streaming concepts that served as the foundational conceptual blueprint for this format.

* **`The Variable-Length Quantity (VLQ) Architecture Antidote`**
  * **Architects:** ISO/IEC JTC 1/SC 22/WG14 Committee (C99 Specification)
  * By defining explicit, invariant fixed bit-widths (`uint32_t`, `uint16_t`, `uint8_t`), the C99 specification allowed this format to completely eliminate variable-length compression bloat, paving the way for a strict 4-byte machine-aligned event structure.

* **`The C Standard Runtime Stream Infrastructure`**
  * **Architects:** Dennis Ritchie, Brian Kernighan, and the ISO C Committees.
  * Provides the primitive, unbuffered disk I/O stream structures (`<stdio.h>`), fast linear buffer block manipulations (`memcpy`), and heap lifecycle management algorithms (`malloc`, `free`, `realloc`) used to achieve zero-allocation data piping.

* **`OpenWatcom C/C++ Compiler & Linker Engine (wcl386 & wlib)`**
  * **Architects:** Watcom International Corporation, Sybase, and the OpenWatcom Community.
  * Provides the specialized 32-bit linear protected-mode compilation framework and strict alignment control vectors (`#pragma pack`) used to translate file layouts straight into flat x86 DOS machine targets without compiler overhead.

* **`GCC / GNU Compiler Collection`**
  * **Architects:** Free Software Foundation and the Open Source Community.
  * Provides the foundational POSIX compilation loop and strict binary linking mapping used to compile clean static target objects on Linux Mint and modern Unix layers.

* **`Clang / LLVM Compiler Infrastructure`**
  * **Architects:** Chris Lattner, the LLVM Community, and contributors.
  * Provides high-performance, standard-compliant code diagnostics and quick binary linking structures for modern operating system pipelines.

* **`Git Version Control System`**
  * **Architects:** Linus Torvalds and the Git Development Community.
  * Provides the completely decentralized version control tracking framework used by systems programmers to branch, log, and maintain source repositories locally on their machines without corporate gatekeeping.

* **`midis2jam2 Alternative Software Project`**
  * **Architects:** The midis2jam2 Development Community.
  * For building a cool, high-quality, lightweight alternative MIDI processing architecture that serves as an excellent reference tool for modern tracking pipelines.

* **`GitLab Open Ecosystem`**
  * **Architects:** GitLab Inc. and Contributors.
  * For hosting a clean, lightweight remote project ecosystem to deploy these open-source repository files without tracking-pixel bloat.

***

## THANK YOU!

And most importantly, **THANK YOU** to you, the programmer, the tester, the person, any person at all!
