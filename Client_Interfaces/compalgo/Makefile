
.PHONY: all clean

ifeq ($(OS),Windows_NT)
# utilisation de ces fichiers pour des problèmes d'encodage des accents dans les
# noms de répertoires
#include Makefile.windows
#PACKAGE1 = $(ISO8859-1)
#include Makefile.windows2
#PACKAGE2 = $(MSDOS)
PATH := C:\msys\1.0\bin;$(PATH)
PACKAGE = /fr/irit/sig/compalgo/interpréteur/bibliothèque
else
# utilisation de ce fichier pour un problème d'encodage des accents dans les
# noms de répertoires
include Makefile.linux
PACKAGE = $(UTF8)
endif

REP = ./pp_pour_Compalgo
REP_SRC = $(REP)/src
REP_BIN = $(REP)/bin

RETOUR = ../../../../../../..
CIBLE = $(REP_BIN)$(PACKAGE)/pp.class

all: $(CIBLE) compalgo_pp.jar

$(CIBLE): $(REP_SRC)$(PACKAGE)/pp.java $(REP_BIN)
	cd $(REP_SRC)$(PACKAGE) && javac -encoding UTF8 ./pp.java -d $(RETOUR)/bin -classpath $(RETOUR)/compalgo.jar:$(RETOUR)/../../java

compalgo_pp.jar: $(CIBLE) ../java/pp/PPNative.class
	cp $(REP)/compalgo.jar ./compalgo_pp.jar && cd $(REP_BIN) && jar uf ../../compalgo_pp.jar fr/ && cd ../../../java && jar uf ../compalgo/compalgo_pp.jar pp/PPNative.class

$(REP_BIN):
	cd $(REP) && mkdir bin

clean:
	rm $(REP_BIN)$(PACKAGE)/pp.class ./compalgo_pp.jar
