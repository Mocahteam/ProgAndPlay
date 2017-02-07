
.PHONY: all clean

IMM_LIBNAME = ppimm
CLASSIC_LIBNAME = pp-client-c

SPRING_VERSION = spring-0.82.5.1

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
ENTETELIB =
LIB = .dll 
OPT =
OPTLIB_IMM = -L../../../SpringPP/$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -Wl,--add-stdcall-alias,--output-def,$(IMM_LIBNAME).def,--out-implib,lib$(IMM_LIBNAME).a
OPTLIB_CLASSIC = -L../../../SpringPP/$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -Wl,--add-stdcall-alias,--output-def,$(CLASSIC_LIBNAME).def,--out-implib,lib$(CLASSIC_LIBNAME).a
FILE_SUPPR = $(IMM_LIBNAME).def lib$(IMM_LIBNAME).a $(CLASSIC_LIBNAME).def lib$(CLASSIC_LIBNAME).a 
INSTALL_CLIENT = install_client
else
ENTETELIB = lib
LIB = .so
OPT = -fPIC
OPTLIB_CLASSIC = -lboost_thread-mt -fPIC -lrt
OPTLIB_IMM = -lboost_thread-mt -fPIC -lrt
FILE_SUPPR =
INSTALL_CLIENT =
endif

PP_DIR = ../../pp
INCLUDE_DIRS = . $(PP_DIR)

CONSTANTLIST_FILES = $(wildcard constantlist*.h)

TARGETLIB_IMM = $(ENTETELIB)$(IMM_LIBNAME)$(LIB)
TARGETLIB_CLASSIC = $(ENTETELIB)$(CLASSIC_LIBNAME)$(LIB)
CIBLES = $(TARGETLIB_IMM) $(TARGETLIB_CLASSIC)

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLES)

# Compile IMM library
$(TARGETLIB_IMM): PP_IMM.o $(PP_DIR)/PP_Client_Private.o $(PP_DIR)/PP_Error.o
	g++ -shared -o $@ $^ $(OPTLIB_IMM)

PP_IMM.o: PP_IMM.c PP_IMM.h $(PP_DIR)/PP_Client_Private.h $(PP_DIR)/PP.h $(PP_DIR)/PP_Error.h
	g++ -c $< $(INCLUDES) $(OPT)

# Compile CLASSIC library
$(TARGETLIB_CLASSIC): PP_Client.o $(PP_DIR)/PP_Client_Private.o $(PP_DIR)/PP_Error.o
	g++ -shared -o $@ $^ $(OPTLIB_CLASSIC)

PP_Client.o: PP_Client.cpp PP_Client.h $(PP_DIR)/PP_Client_Private.h $(PP_DIR)/PP.h $(PP_DIR)/PP_Error.h $(PP_DIR)/traces/TraceConstantList.h
	g++ -c $< $(INCLUDES) $(OPT)
	
clean:
	rm $(CIBLES) PP_IMM.o PP_Client.o $(FILE_SUPPR)

install: $(INSTALL_CLIENT)

install_client: exemples/projet/$(TARGETLIB_IMM) exemples/$(TARGETLIB_CLASSIC) exemples/projet/$(TARGETLIB_CLASSIC) ../ocaml/$(TARGETLIB_CLASSIC)

exemples/projet/$(TARGETLIB_IMM): $(TARGETLIB_IMM)
	cp $(TARGETLIB_IMM) exemple/projet

exemples/$(TARGETLIB_CLASSIC): $(TARGETLIB_CLASSIC)
	cp $(TARGETLIB_CLASSIC) exemple

exemples/projet/$(TARGETLIB_CLASSIC): $(TARGETLIB_CLASSIC)
	cp $(TARGETLIB_CLASSIC) exemple/projet

../ocaml/$(TARGETLIB_CLASSIC): $(TARGETLIB_CLASSIC)
	cp $(TARGETLIB_CLASSIC) ../ocaml