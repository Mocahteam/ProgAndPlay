
.PHONY: all clean install

FLASH_DEV_DIR = "C:/Users/Mathieu Muratet/AppData/Local/FlashDevelop/Apps/flexairsdk/4.6.0+29.0.0/bin/"

LIB_NAME = ProgAndPlayExtension

PATH := C:\msys\1.0\bin;$(PATH)
OPTLIB = -L../../../../../$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt
PP_DIR = ../../../../../pp
PP_CLIENT = ../../../../c-cpp
INCLUDE_DIRS = . $(PP_DIR) $(PP_CLIENT)

CIBLE = $(LIB_NAME).ane

SUPPR := $(CIBLE) $(LIB_NAME).dll $(LIB_NAME).o library.swf ProgAndPlayANE.swc

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(CIBLE)

$(LIB_NAME).o: $(LIB_NAME).cpp
	g++ -c $< $(INCLUDES)
	
$(LIB_NAME).dll: $(LIB_NAME).o FlashRuntimeExtensions.lib $(PP_DIR)/PP_Client_Private.o $(PP_DIR)/PP_Error.o $(PP_CLIENT)/PP_Client.o
	g++ -shared -o $@ $^ $(OPTLIB)
	
$(CIBLE): ProgAndPlayANE.swc extensionDescriptor.xml library.swf $(LIB_NAME).dll
	$(FLASH_DEV_DIR)adt -package -target ane $(CIBLE) extensionDescriptor.xml -swc ProgAndPlayANE.swc -platform Windows-x86 library.swf $(LIB_NAME).dll
	
library.swf: ProgAndPlayANE.swc
	7za e -y ProgAndPlayANE.swc library.swf
	
ProgAndPlayANE.swc: ../ProgAndPlayANE.swc
	cp ../ProgAndPlayANE.swc .
	
clean:
	rm -f $(SUPPR)

install:
	cp $(CIBLE) ../../../Scratch/ane/$(CIBLE)
	rm -rf ../../../Scratch/ext/ProgAndPlayExtension.ane/*
	7za x -y $(CIBLE) -o../../../Scratch/ext/ProgAndPlayExtension.ane/
