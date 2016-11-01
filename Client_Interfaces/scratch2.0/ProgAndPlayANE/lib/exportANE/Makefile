
.PHONY: all clean

LIB_NAME = ProgAndPlayExtension
PATH := C:\msys\1.0\bin;$(PATH)
PP_DIR = ../../../../../pp
#OPTLIB = -Wl,--add-stdcall-alias,--output-def,$(LIB_NAME).def,--out-implib,lib$(LIB_NAME).a
INCLUDE_DIRS = . $(PP_DIR)

CIBLE = $(LIB_NAME).ane

SUPPR := $(CIBLE) $(LIB_NAME).dll $(LIB_NAME).o library.swf ProgAndPlayANE.swc

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLE)

$(LIB_NAME).o: $(LIB_NAME).cpp
	g++ -c $< $(INCLUDES)
	
$(LIB_NAME).dll: $(LIB_NAME).o $(PP_DIR)/pp-client.dll FlashRuntimeExtensions.lib
	g++ -shared -o $@ $(LIB_NAME).o FlashRuntimeExtensions.lib -L$(PP_DIR) -lpp-client
	
$(CIBLE): ProgAndPlayANE.swc extensionDescriptor.xml library.swf $(LIB_NAME).dll
	C:\Users\mmuratet\AppData\Local\FlashDevelop\Apps\flexairsdk\4.6.0+19.0.0\bin\adt -package -target ane $(CIBLE) extensionDescriptor.xml -swc ProgAndPlayANE.swc -platform Windows-x86 library.swf $(LIB_NAME).dll
	
library.swf: ProgAndPlayANE.swc
	7za e -y ProgAndPlayANE.swc library.swf
	
ProgAndPlayANE.swc: ../ProgAndPlayANE.swc
	cp ../ProgAndPlayANE.swc .
	
clean:
	rm $(SUPPR)

install:
	cp $(CIBLE) ../../../Scratch/ane/$(CIBLE)
	rm -rf ../../../Scratch/ext/ProgAndPlayExtension.ane/*
	7za x -y $(CIBLE) -o../../../Scratch/ext/ProgAndPlayExtension.ane/