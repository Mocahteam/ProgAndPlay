# Compile with g++ 4.4 or later

.PHONY: all clean install

LIB_NAME_CLIENT = pp-client-private
LIB_NAME_SUPPLIER = pp-supplier

REP_CLIENT = ../Client_Interfaces
REP_SUPPLIER = ../$(SPRING_VERSION)/rts/lib/pp

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
ENTETELIB = 
LIB = .dll
OPT = -std=c++11 -std=gnu++11
OPTLIB_CLIENT = -L../$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -lboost_system-mt -Wl,--add-stdcall-alias,--output-def,$(LIB_NAME_CLIENT).def,--out-implib,lib$(LIB_NAME_CLIENT).a
OPTLIB_SUPPLIER = -Wl,--add-stdcall-alias,--output-def,$(LIB_NAME_SUPPLIER).def,--out-implib,lib$(LIB_NAME_SUPPLIER).a
INCLUDE_DIRS = . ../$(SPRING_VERSION)/mingwlibs/include
else
ENTETELIB = lib
LIB = .so
OPT = -fPIC -std=c++11 -std=gnu++11
OPTLIB_CLIENT = -lboost_thread -fPIC -lrt
OPTLIB_SUPPLIER = $(OPTLIB_CLIENT)
INCLUDE_DIRS = .
endif

CIBLE_CLIENT = $(ENTETELIB)$(LIB_NAME_CLIENT)$(LIB)
CIBLE_SUPPLIER = $(ENTETELIB)$(LIB_NAME_SUPPLIER)$(LIB)

SUPPR := $(CIBLE_CLIENT) $(CIBLE_SUPPLIER) ./test/$(CIBLE_CLIENT) ./test/$(CIBLE_SUPPLIER) PP_Client_Private.o PP_Supplier.o PP_Error.o
ifeq ($(OS),Windows_NT)
SUPPR := $(SUPPR) lib$(LIB_NAME_CLIENT).a lib$(LIB_NAME_SUPPLIER).a $(LIB_NAME_CLIENT).def $(LIB_NAME_SUPPLIER).def
endif

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLE_CLIENT) $(CIBLE_SUPPLIER) compile_traces

$(CIBLE_CLIENT): PP_Client_Private.o PP_Error.o
	g++ -shared -o $@ $^ $(OPTLIB_CLIENT)

$(CIBLE_SUPPLIER): PP_Supplier.o PP_Error.o
	g++ -shared -o $@ $^ $(OPTLIB_SUPPLIER)

PP_Supplier.o: PP_Supplier.cpp PP_Supplier.h PP.h
	g++ -c $< $(INCLUDES) $(OPT)

PP_Client_Private.o: PP_Client_Private.cpp PP_Client_Private.h PP.h
	g++ -c $< $(INCLUDES) $(OPT)
	
PP_Error.o: PP_Error.c PP_Error.h PP_Error_Private.h
	g++ -c $< $(INCLUDES) $(OPT)

clean:
	rm -f $(SUPPR)
	@(cd ./traces && $(MAKE) $@)
	
compile_traces:
	@echo ""
	@echo "-------------------------------------------------------------------------------"
	@echo "| Compilation de pp-trace                                                     |"
	@echo "-------------------------------------------------------------------------------"
	@(cd ./traces && $(MAKE))
	
install: install_supplier install_client install_traces

install_supplier: $(REP_SUPPLIER)/PP_Error.c $(REP_SUPPLIER)/PP_Error.h $(REP_SUPPLIER)/PP_Error_Private.h  $(REP_SUPPLIER)/PP.h $(REP_SUPPLIER)/PP_Private.h $(REP_SUPPLIER)/PP_Supplier.cpp $(REP_SUPPLIER)/PP_Supplier.h ./test/$(CIBLE_SUPPLIER)

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
	
./test/$(CIBLE_SUPPLIER): $(CIBLE_SUPPLIER)
	cp $(CIBLE_SUPPLIER) ./test
	
install_client: ./test/$(CIBLE_CLIENT)

./test/$(CIBLE_CLIENT):
	cp $(CIBLE_CLIENT) ./test
	
install_traces:
	@echo ""
	@echo "-------------------------------------------------------------------------------"
	@echo "| Installation de pp-trace                                                    |"
	@echo "-------------------------------------------------------------------------------"
	@(cd ./traces && $(MAKE) install)
