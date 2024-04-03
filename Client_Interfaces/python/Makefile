# WARNING !!!!
# build exe in a 32 bits platform

.PHONY: all clean install

EXE_NAME = 32bitWrapper

ifeq ($(OS),Windows_NT)
PATH := C:\msys\1.0\bin;$(PATH)
EXTENTION = .exe
OPTLIB = -L../../$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -lws2_32
else
EXTENTION = 
OPTLIB = -lboost_thread -fPIC -lrt
endif

PP_DIR = ../../pp
PP_CLIENT = ../c-cpp
INCLUDE_DIRS = . $(PP_DIR) $(PP_CLIENT)

TARGET_EXE = $(EXE_NAME)$(EXTENTION)

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: $(TARGET_EXE)

$(TARGET_EXE): $(EXE_NAME).o $(PP_DIR)/PP_Client_Private.o $(PP_DIR)/PP_Error.o $(PP_CLIENT)/PP_Client.o
	g++ -o $@ $^ $(OPTLIB)

$(EXE_NAME).o: $(EXE_NAME).c $(PP_CLIENT)/PP_Client.h $(PP_DIR)/PP.h $(PP_DIR)/PP_Client_Private.h $(PP_DIR)/traces/TraceConstantList.h
	g++ -c $< $(INCLUDES)

clean:
	rm -f $(TARGET_EXE) $(EXE_NAME).o exemples/$(TARGET_EXE) exemples/pp.py

install: exemples/$(TARGET_EXE) exemples/pp.py

exemples/$(TARGET_EXE): $(TARGET_EXE)
	cp $(TARGET_EXE) exemples

exemples/pp.py: pp.py
	cp pp.py exemples
