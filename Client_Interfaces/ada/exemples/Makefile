
.PHONY: all clean

ifeq ($(OS),Windows_NT)
BIN = .exe
ENTETELIB =
LIB = .dll
PATH := C:\msys\1.0\bin;$(PATH)
else
BIN =
ENTETELIB = lib
LIB = .so
PATH_GNAT = $(PATH)
endif

PP_DIR = ../../../pp

ADB_FILES = $(wildcard ./*.adb)
ALI_FILES = $(ADB_FILES:.adb=.ali)
O_FILES = $(ADB_FILES:.adb=.o)
BIN_FILES = $(ADB_FILES:.adb=$(BIN))

CONSTANTLIST_FILES = $(wildcard ../constantlist*.ads)

all: $(O_FILES)

%.o: %.adb ../pp.ads ../pp.adb $(CONSTANTLIST_FILES) ../$(ENTETELIB)adaDevice$(LIB) $(PP_DIR)/$(ENTETELIB)pp-client$(LIB)
	@gnatmake -I.. -L$(PP_DIR) -L.. $< -largs -ladaDevice -largs -lpp-client

clean:
	rm $(BIN_FILES) $(O_FILES) $(ALI_FILES)
