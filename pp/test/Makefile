
.PHONY: all clean

SPRING_VERSION = spring-0.82.5.1

ifeq ($(OS),Windows_NT)
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

all: testClientPrivate.o testSupplier.o

testClientPrivate.o: testClientPrivate.c constantList_KP4.1.h ../PP_Client_Private.h ../$(ENTETELIB)pp-client-private$(LIB)
	g++ -o $@ -c $< -I.
	g++ -o $*$(BIN) $@ -L.. -lpp-client-private

testSupplier.o: testSupplier.c constantList_KP4.1.h ../PP_Supplier.h ../$(ENTETELIB)pp-supplier$(LIB)
	g++ -o $@ -c $< -I. -I../../../SpringPP/$(SPRING_VERSION)/mingwlibs/include
	g++ -o $*$(BIN) $@ -L.. -lpp-supplier -L../../../SpringPP/$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt

clean:
	rm $(BIN_FILES) $(O_FILES)