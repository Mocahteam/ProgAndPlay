# Prog&amp;Play low level library
This directory includes low level source code of Prog&amp;Play. This part of the project builds two libraries:
1. pp-client is the "client" side of the library. It is required for "[client interfaces](../Client_Interfaces)" (or high level interfaces used by players);
2. pp-supplier is the "server" side of the library. It has to be integrated into a game engine in order to enable players to control game's units by programming thanks to pp-client. Currently Prog&amp;Play is integrated into a [fork of the Spring engine](../SpringPP).

