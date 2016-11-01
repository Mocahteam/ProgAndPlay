
.PHONY: all clean

LIB_NAME = ProgAndPlayPlugin

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
ENTETELIB1 = 
ENTETELIB2 = 
LIB1 = .dll
LIB2 = .dll
OPT =
OPTLIB = -Wl,--add-stdcall-alias,--output-def,$(LIB_NAME).def,--out-implib,lib$(LIB_NAME).a
else
ENTETELIB1 =
ENTETELIB2 = lib
LIB1 =
LIB2 = .so
OPT = -fPIC
OPTLIB = -fPIC -lrt
endif

PP_DIR = ../../pp
INCLUDE_DIRS = . $(PP_DIR)

CIBLE = $(ENTETELIB1)$(LIB_NAME)$(LIB1)

SUPPR := $(CIBLE) $(LIB_NAME).o
ifeq ($(OS),Windows_NT)
SUPPR := $(SUPPR) lib$(LIB_NAME).a $(LIB_NAME).def
endif

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLE)

$(CIBLE): $(LIB_NAME).o $(PP_DIR)/$(ENTETELIB2)pp-client$(LIB2)
	g++ -shared -o $@ $< $(OPTLIB) -L$(PP_DIR) -lpp-client

$(LIB_NAME).o: $(LIB_NAME).c
	g++ -c $< $(INCLUDES) $(OPT)

clean:
	rm $(SUPPR)
