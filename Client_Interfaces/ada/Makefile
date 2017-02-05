
.PHONY: all clean

LIBNAME = pp-client-ada

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

CONSTANTLIST_FILES = $(wildcard constantlist*.ads)

TARGETLIB = $(ENTETELIB)$(LIBNAME)$(LIB)
CIBLES = $(TARGETLIB) pp.o $(CONSTANTLIST_FILES:.ads=.o)

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLES)

$(TARGETLIB): adaSpecialFeatures.o $(PP_DIR)/PP_Client_Private.o $(PP_DIR)/PP_Error.o $(PP_CLIENT)/PP_Client.o
	g++ -shared -o $@ $^ $(OPTLIB)

adaSpecialFeatures.o: adaSpecialFeatures.c $(PP_CLIENT)/PP_Client.h $(PP_DIR)/PP.h $(PP_DIR)/PP_Client_Private.h $(PP_DIR)/traces/TraceConstantList.h
	g++ -c $< $(INCLUDES)

pp.o: pp.adb pp.ads
	@gnatmake -c pp.adb
	@echo gnatmake -c pp.adb

%.o: %.ads
	@gnatmake -c $<
	@echo gnatmake -c $<

clean:
	rm $(CIBLES) adaSpecialFeatures.o pp.ali $(CONSTANTLIST_FILES:.ads=.ali) $(FILE_SUPPR)

install: $(INSTALL_CLIENT)

install_client: exemples/$(TARGETLIB)

exemples/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) exemples