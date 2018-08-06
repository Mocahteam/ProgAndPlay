
.PHONY: all clean install

LIBNAME = pp-client-python

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
ENTETELIB =
LIB = .dll 
OPTLIB = -L../../$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -Wl,--add-stdcall-alias,--output-def,$(LIBNAME).def,--out-implib,lib$(LIBNAME).a
FILE_SUPPR = $(LIBNAME).def lib$(LIBNAME).a
else
ENTETELIB = lib
LIB = .so
OPTLIB = -lboost_thread -fPIC -lrt
FILE_SUPPR =
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
	rm -f $(CIBLES) pythonSpecialFeatures.o $(FILE_SUPPR) exemples/$(TARGETLIB) exemples/pp.py

install: exemples/$(TARGETLIB) exemples/pp.py

exemples/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) exemples

exemples/pp.py: pp.py
	cp pp.py exemples
