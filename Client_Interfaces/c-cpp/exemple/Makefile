
.PHONY: all clean

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
BIN = .exe
ENTETELIB = 
LIB = .dll
else
BIN = 
ENTETELIB = lib
LIB = .so
endif

C_FILES = $(wildcard ./*.c)
O_FILES = $(C_FILES:.c=.o)
BIN_FILES = $(C_FILES:.c=$(BIN))

CONSTANTLIST_FILES = $(wildcard ../constantList*.h)

REP_PP = ../../../pp

all: $(O_FILES)

%.o: %.c $(CONSTANTLIST_FILES) ../PP_Client.h $(ENTETELIB)pp-client-c$(LIB)
	g++ -o $@ -c $< -I. -I.. -I$(REP_PP)
	g++ -o $*$(BIN) $@ -L. -lpp-client-c


clean:
	rm -f $(BIN_FILES) $(O_FILES)
