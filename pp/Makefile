# Compile with g++ 4.4 or later

.PHONY: all clean

LIB_NAME_CLIENT = pp-client
LIB_NAME_SUPPLIER = pp-supplier

SPRING_VERSION = spring_0.82.5.1

REP_CLIENT = ../Client_Interfaces
REP_SUPPLIER = ../../SpringPP/$(SPRING_VERSION)/rts/lib/pp

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
ENTETELIB = 
LIB = .dll
OPT =
OPTLIB_CLIENT = -L../../SpringPP/$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -Wl,--add-stdcall-alias,--output-def,$(LIB_NAME_CLIENT).def,--out-implib,lib$(LIB_NAME_CLIENT).a
OPTLIB_SUPPLIER = -Wl,--add-stdcall-alias,--output-def,$(LIB_NAME_SUPPLIER).def,--out-implib,lib$(LIB_NAME_SUPPLIER).a
INCLUDE_DIRS = . ../../SpringPP/$(SPRING_VERSION)/mingwlibs/include
INSTALL_CLIENT = install_client
else
ENTETELIB = lib
LIB = .so
OPT = -fPIC
OPTLIB_CLIENT = -lboost_thread-mt -fPIC -lrt
OPTLIB_SUPPLIER = $(OPTLIB_CLIENT)
INCLUDE_DIRS = .
INSTALL_CLIENT = 
endif

CIBLE_CLIENT = $(ENTETELIB)$(LIB_NAME_CLIENT)$(LIB)
CIBLE_SUPPLIER = $(ENTETELIB)$(LIB_NAME_SUPPLIER)$(LIB)

SUPPR := $(CIBLE_CLIENT) $(CIBLE_SUPPLIER) PP_Client.o PP_Supplier.o PP_Error.o
ifeq ($(OS),Windows_NT)
SUPPR := $(SUPPR) lib$(LIB_NAME_CLIENT).a lib$(LIB_NAME_SUPPLIER).a $(LIB_NAME_CLIENT).def $(LIB_NAME_SUPPLIER).def
endif

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLE_CLIENT) $(CIBLE_SUPPLIER)

$(CIBLE_CLIENT): PP_Client.o PP_Error.o
	g++ -shared -o $@ $^ $(OPTLIB_CLIENT)

$(CIBLE_SUPPLIER): PP_Supplier.o PP_Error.o
	g++ -shared -o $@ $^ $(OPTLIB_SUPPLIER)

PP_Supplier.o: PP_Supplier.cpp PP_Supplier.h PP.h
	g++ -c $< $(INCLUDES) $(OPT)

PP_Client.o: PP_Client.cpp PP_Client.h PP_Client_Private.h PP.h
	g++ -c $< $(INCLUDES) $(OPT)
	
PP_Error.o: PP_Error.c PP_Error.h PP_Error_Private.h
	g++ -c $< $(INCLUDES) $(OPT)

clean:
	rm $(SUPPR)
	
install: install_supplier $(INSTALL_CLIENT)

install_supplier: $(REP_SUPPLIER)/PP_Error.c $(REP_SUPPLIER)/PP_Error.h $(REP_SUPPLIER)/PP_Error_Private.h  $(REP_SUPPLIER)/PP.h $(REP_SUPPLIER)/PP_Private.h $(REP_SUPPLIER)/PP_Supplier.cpp $(REP_SUPPLIER)/PP_Supplier.h

$(REP_SUPPLIER)/PP_Error.c: PP_Error.c
	cp PP_Error.c $(REP_SUPPLIER)

$(REP_SUPPLIER)/PP_Error.h: PP_Error.h
	cp PP_Error.h $(REP_SUPPLIER)

$(REP_SUPPLIER)/PP_Error_Private.h: PP_Error_Private.h
	cp PP_Error_Private.h $(REP_SUPPLIER)

$(REP_SUPPLIER)/PP.h: PP.h
	cp PP.h $(REP_SUPPLIER)

$(REP_SUPPLIER)/PP_Private.h: PP_Private.h
	cp PP_Private.h $(REP_SUPPLIER)

$(REP_SUPPLIER)/PP_Supplier.cpp: PP_Supplier.cpp
	cp PP_Supplier.cpp $(REP_SUPPLIER)

$(REP_SUPPLIER)/PP_Supplier.h: PP_Supplier.h
	cp PP_Supplier.h $(REP_SUPPLIER)

# Windows => copy new dll
# Linux, it is better that using symbolic links. Then, it is not necessary to make a copy of library
install_client: $(REP_CLIENT)/ada/exemples/$(CIBLE_CLIENT) $(REP_CLIENT)/c-cpp/exemple/$(CIBLE_CLIENT) $(REP_CLIENT)/compalgo/$(CIBLE_CLIENT) $(REP_CLIENT)/java/$(CIBLE_CLIENT) $(REP_CLIENT)/ocaml/$(CIBLE_CLIENT) $(REP_CLIENT)/scratch/$(CIBLE_CLIENT) ./test/$(CIBLE_CLIENT)

$(REP_CLIENT)/ada/exemples/$(CIBLE_CLIENT): $(CIBLE_CLIENT)
	cp $(CIBLE_CLIENT) ../Client_Interfaces/ada/exemples/

$(REP_CLIENT)/c-cpp/exemple/$(CIBLE_CLIENT): $(CIBLE_CLIENT)
	cp $(CIBLE_CLIENT) $(REP_CLIENT)/c-cpp/exemple
	cp $(CIBLE_CLIENT) $(REP_CLIENT)/c-cpp/exemple/projet

$(REP_CLIENT)/compalgo/$(CIBLE_CLIENT): $(CIBLE_CLIENT)
	cp $(CIBLE_CLIENT) $(REP_CLIENT)/compalgo

$(REP_CLIENT)/java/$(CIBLE_CLIENT): $(CIBLE_CLIENT)
	cp $(CIBLE_CLIENT) $(REP_CLIENT)/java

$(REP_CLIENT)/ocaml/$(CIBLE_CLIENT): $(CIBLE_CLIENT)
	cp $(CIBLE_CLIENT) $(REP_CLIENT)/ocaml

$(REP_CLIENT)/scratch/$(CIBLE_CLIENT): $(CIBLE_CLIENT)
	cp $(CIBLE_CLIENT) $(REP_CLIENT)/scratch
	
./test/$(CIBLE_CLIENT): $(CIBLE_CLIENT)
	cp $(CIBLE_CLIENT) ./test