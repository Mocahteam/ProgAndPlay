
.PHONY: all clean

LIBNAME = adaDevice

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
ENTETELIB =
LIB = .dll 
OPTLIB = -Wl,--add-stdcall-alias,--output-def,$(LIBNAME).def,--out-implib,lib$(LIBNAME).a
FILE_SUPPR = $(LIBNAME).def lib$(LIBNAME).a
INSTALL_CLIENT = install_client
else
ENTETELIB = lib
LIB = .so
OPTLIB =
FILE_SUPPR =
INSTALL_CLIENT =
endif

PP_DIR = ../../pp
INCLUDE_DIRS = . $(PP_DIR)

CONSTANTLIST_FILES = $(wildcard constantlist*.ads)

TARGETLIB = $(ENTETELIB)$(LIBNAME)$(LIB)
CIBLES = $(TARGETLIB) pp.o $(CONSTANTLIST_FILES:.ads=.o)

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLES)

$(TARGETLIB): adaDevice.o $(PP_DIR)/$(ENTETELIB)pp-client$(LIB)
	g++ -shared -o $@ $< -L$(PP_DIR) $(OPTLIB) -lpp-client

adaDevice.o: adaDevice.c $(PP_DIR)/PP_Client.h
	g++ -c $< $(INCLUDES)

pp.o: pp.adb pp.ads
	@gnatmake -c pp.adb
	@echo gnatmake -c pp.adb

%.o: %.ads
	@gnatmake -c $<
	@echo gnatmake -c $<

clean:
	rm $(CIBLES) adaDevice.o pp.ali $(CONSTANTLIST_FILES:.ads=.ali) $(FILE_SUPPR)

install: $(INSTALL_CLIENT)

install_client: exemples/$(TARGETLIB)

exemples/$(TARGETLIB): $(TARGETLIB)
	cp $(TARGETLIB) exemples