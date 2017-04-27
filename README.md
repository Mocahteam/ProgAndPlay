# Prog&amp;Play
Prog&amp;Play is a library designed for Real Time Strategy games (RTS). It enables player to program easily and interactively units of the video game. More details on [Prog&amp;Play pages](https://www.irit.fr/ProgAndPlay/index_en.php) 

## Repository architecture
This repository includes source code of the Prog&amp;Play library. It is composed by the [pp](./pp) directory that includes low level source code of Prog&amp;Play and [Client_Interfaces](./Client_Interfaces) directory that includes interfaces of Prog&amp;Play in different programming langages (It is one of these interfaces that the player has to use).

## Windows Compilation
* Prog&Play compilation and associated interfaces
  * Important note: you have to install several dependencies:
    * Download following archive ([MinGW-gcc4.4.zip](https://www.irit.fr/ProgAndPlay/ressources/MinGW-gcc4.4.zip)), it contains g++ compiler based on MinGW. Unzip this archive at the root of the drive "C:\". Update your environment variables:
      * Create a new variable named MINGW with "C:\MinGW-gcc4.4" as value;
      * Update your PATH by adding in first value: "%MINGW%\bin".
    * Msys (Minimal system), install it in your C:\ directory => C:\msys\1.0\bin do not have been included in your pass
    * For Ocaml, install standard GCC3.4.5 (GCC4.4 is incompatible), install in C:\MinGW-gcc3.4.5\ directory following packages:
      * MinGW Runtime - mingwrt (dev & dll)
      * MinGW API for MS-Windows - w32api (dev)
      * GNU Binutils - binutils (bin)
      * GCC 3 - gcc-core
      * GCC 3 - gcc-c++
    * To compile, type: mingw32-make

## Linux compilation
* Prog&Play compilation and associated interfaces
  * to compile, just type: make
