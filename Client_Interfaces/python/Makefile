
.PHONY: all clean

LIBNAME = pp-client-python

SPRING_VERSION = spring-0.82.5.1

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
ENTETELIB =
LIB = .dll 
OPTLIB = -L../../../SpringPP/$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -Wl,--add-stdcall-alias,--output-def,$(LIBNAME).def,--out-implib,lib$(LIBNAME).a
FILE_SUPPR = $(LIBNAME).def lib$(LIBNAME).a
INSTALL_CLIENT = install_client
else
ENTETELIB = lib
LIB = .so
OPTLIB = -lboost_thread-mt -fPIC -lrt
FILE_SUPPR =
INSTALL_CLIENT =
endif

PP_DIR = ../../pp
PP_CLIENT = ../c-cpp
INCLUDE_DIRS = . $(PP_DIR) $(PP_CLIENT)

TARGETLIB = $(ENTETELIB)$(LIBNAME)$(LIB)
CIBLES = $(TARGETLIB)

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLES)

$(TARGETLIB): pythonSpecialFeatures.o $(PP_DIR)/PP_Client_Private.o $(PP_DIR)/PP_Error.o $(PP_CLIENT)/PP_Client.o
	g++ -shared -o $@ $^ $(OPTLIB)

pythonSpecialFeatures.o: pythonSpecialFeatures.c $(PP_CLIENT)/PP_Client.h $(PP_DIR)/PP.h $(PP_DIR)/PP_Client_Private.h $(PP_DIR)/traces/TraceConstantList.h
	g++ -c $< $(INCLUDES)

clean:
	rm $(CIBLES) pythonSpecialFeatures.o $(FILE_SUPPR)

install: $(INSTALL_CLIENT)

install_client: exemples/$(TARGETLIB) exemples/pp.py

exemples/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) exemples

exemples/pp.py: pp.py
	cp pp.py exemples