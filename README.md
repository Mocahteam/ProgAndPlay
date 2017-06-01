# Prog&amp;Play
Prog&amp;Play is a library designed for Real Time Strategy games (RTS). It enables player to program easily and interactively units of the video game. More details on [Prog&amp;Play pages](http://progandplay.lip6.fr/index_en.php). 

Currently Prog&Play is integrated into the [Spring engine](https://github.com/Mocahteam/SpringPP).

Binaries are available at: <http://progandplay.lip6.fr/download.php?LANG=en>

## Repository architecture
This repository includes source code of the Prog&amp;Play library. It is composed by the [pp](./pp) directory that includes low level source code of Prog&amp;Play and [Client_Interfaces](./Client_Interfaces) directory that includes interfaces of Prog&amp;Play in different programming langages (It is one of these interfaces that the player has to use).

## Windows Compilation
* Important note: Prog&amp;Play need libraries included into [SpringPP repository](https://github.com/Mocahteam/SpringPP), you have to organise your folders to install ProgAndPlay repository and SpringPP repository in the same folder.
* Download following archive ([MinGW-gcc4.4.zip](http://progandplay.lip6.fr/ressources/MinGW-gcc4.4.zip)), it contains g++ compiler based on MinGW. Unzip this archive at the root of the drive "C:\". Update your environment variables:
  * Create a new variable named MINGW with "C:\MinGW-gcc4.4" as value;
  * Update your PATH by adding in first value: "%MINGW%\bin".
* Msys (Minimal system), install it in your C:\ directory => C:\msys\1.0\bin do not have been included in your pass
* For Ocaml, install standard [GCC3.4.5](http://progandplay.lip6.fr/ressources/MinGW-gcc3.4.5.zip) (GCC4.4 is incompatible), install in C:\MinGW-gcc3.4.5\
* To compile, type: mingw32-make
* To test an example
  * Launch the update version of [Spring engine](https://github.com/Mocahteam/SpringPP) for Prog&amp;Play and play a game (advice: download binaries from web site - [http://progandplay.lip6.fr/download.php?LANG=en](http://progandplay.lip6.fr/download.php?LANG=en)).
  * Move into the Client_Interface folder you want to test
  * Run example program
  * Observe results into the game

## Linux compilation (tested on Ubuntu 16.04)
* Get dependencies (install development packages)
  * Boost (tested with v1.58)
  * Rapidxml (tested with v1.13)
  * Rapidjson  (tested with v0.12)
  * Jdk (tested with v8)
  * Ocaml (tested with v4)
  * Gnat (tested with v4.9)
* Build libraries and examples
  * To compile, just type: make
* Test an example
  * Launch the updated version of [Spring engine](https://github.com/Mocahteam/SpringPP) for Prog&amp;Play and play a game (you have to compile this version by yourself).
  * Move into the Client_Interface folder you want to test
  * Run example program (You probably need to add "." to your LD_LIBRARY_PATH or install .so files in suitable folder)
  * Observe results into the game
