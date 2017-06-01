
.PHONY: all clean

ifeq ($(OS),Windows_NT)
BIN = .exe
ENTETELIB = 
LIB = .dll
INCLUDES = -I../../$(SPRING_VERSION)/mingwlibs/include
LIBS = -L../../$(SPRING_VERSION)/mingwlibs/lib
BOOST_THREAD = -lboost_thread-mt
else
BIN = 
ENTETELIB = lib
LIB = .so
INCLUDES = 
LIBS = 
BOOST_THREAD = -lboost_thread
endif

C_FILES = $(wildcard ./*.c)
O_FILES = $(C_FILES:.c=.o)
BIN_FILES = $(C_FILES:.c=$(BIN))

all: testClientPrivate.o testSupplier.o

testClientPrivate.o: testClientPrivate.c constantList_KP4.1.h ../PP_Client_Private.h ../$(ENTETELIB)pp-client-private$(LIB)
	g++ -o $@ -c $< -I.
	g++ -o $*$(BIN) $@ -L.. -lpp-client-private

testSupplier.o: testSupplier.c constantList_KP4.1.h ../PP_Supplier.h ../$(ENTETELIB)pp-supplier$(LIB)
	g++ -o $@ -c $< -I. $(INCLUDES)
	g++ -o $*$(BIN) $@ -L.. -lpp-supplier $(LIBS) $(BOOST_THREAD)

clean:
	rm $(BIN_FILES) $(O_FILES)
