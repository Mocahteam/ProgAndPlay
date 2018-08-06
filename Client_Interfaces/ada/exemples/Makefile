
.PHONY: all clean

ifeq ($(OS),Windows_NT)
BIN = .exe
ENTETELIB =
LIB = .dll
PATH := C:\msys\1.0\bin;$(PATH)
# Compile with g++ 4.4 (gnat not available for 4.8.5)
GNATMAKE = C:\MinGW-gcc4.4\bin\gnatmake
else
BIN =
ENTETELIB = lib
LIB = .so
GNATMAKE = gnatmake
endif

ADB_FILES = $(wildcard ./*.adb)
ALI_FILES = $(ADB_FILES:.adb=.ali)
O_FILES = $(ADB_FILES:.adb=.o)
BIN_FILES = $(ADB_FILES:.adb=$(BIN))

CONSTANTLIST_FILES = $(wildcard ../constantlist*.ads)

all: $(O_FILES)

%.o: %.adb ../pp.ads ../pp.adb $(CONSTANTLIST_FILES) $(ENTETELIB)pp-client-ada$(LIB)
	@$(GNATMAKE) -I.. -L. $< -largs -lpp-client-ada

clean:
	rm -f $(BIN_FILES) $(O_FILES) $(ALI_FILES)
