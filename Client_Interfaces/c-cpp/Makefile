
.PHONY: all clean

LIBNAME = ppimm

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
ENTETELIB =
LIB = .dll 
OPT =
OPTLIB = -Wl,--add-stdcall-alias,--output-def,$(LIBNAME).def,--out-implib,lib$(LIBNAME).a
FILE_SUPPR = $(LIBNAME).def lib$(LIBNAME).a
INSTALL_CLIENT = install_client
else
ENTETELIB = lib
LIB = .so
OPT = -fPIC
OPTLIB = -fPIC -lrt
FILE_SUPPR =
INSTALL_CLIENT =
endif

PP_DIR = ../../pp
INCLUDE_DIRS = . $(PP_DIR)

CONSTANTLIST_FILES = $(wildcard constantlist*.h)

TARGETLIB = $(ENTETELIB)$(LIBNAME)$(LIB)
CIBLES = $(TARGETLIB)

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLES)

$(TARGETLIB): PP_IMM.o $(PP_DIR)/$(ENTETELIB)pp-client$(LIB)
	g++ -shared -o $@ $< $(OPTLIB) -L$(PP_DIR) -lpp-client

PP_IMM.o: PP_IMM.c PP_IMM.h $(PP_DIR)/PP_Client.h $(PP_DIR)/PP.h $(PP_DIR)/PP_Error.h
	g++ -c $< $(INCLUDES) $(OPT)

clean:
	rm $(CIBLES) PP_IMM.o $(FILE_SUPPR)

install: $(INSTALL_CLIENT)

install_client: exemples/$(TARGETLIB)

exemples/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) exemple/projet