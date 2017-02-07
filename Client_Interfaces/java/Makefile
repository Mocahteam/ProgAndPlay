
.PHONY: all clean doc jar

LIBNAME = pp-client-java

SPRING_VERSION = spring-0.82.5.1

ifeq ($(OS),Windows_NT)
PATH := /c/msys/1.0/bin:$(PATH)
ENTETELIB = 
LIB = .dll
OPT = 
OPTLIB = -L../../../SpringPP/$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -Wl,--add-stdcall-alias,--output-def,$(LIBNAME).def,--out-implib,lib$(LIBNAME).a
FILE_SUPPR = $(LIBNAME).def lib$(LIBNAME).a
INCLUDE_OS = -I"$(JAVA_HOME)" -I"$(JAVA_HOME)\include" -I"$(JAVA_HOME)\include\win32"
INSTALL_CLIENT = install_client
else
ENTETELIB = lib
LIB = .so
OPT = -fPIC
OPTLIB = -lboost_thread-mt -fPIC -lrt
FILE_SUPPR = 
INCLUDE_OS = -I$(JAVA_HOME) -I$(JAVA_HOME)/include/ -I$(JAVA_HOME)/include/linux
INSTALL_CLIENT = 
endif

CIBLE = $(ENTETELIB)$(LIBNAME)$(LIB)

PP_DIR = ../../pp
PP_CLIENT = ../c-cpp

INCLUDE_DIRS = -I. -I$(PP_DIR) -I$(PP_CLIENT) $(INCLUDE_OS)

JAVA_FILES = $(wildcard ./pp/*.java) $(wildcard ./pp/exceptions/*.java)
CLASS_FILES = $(JAVA_FILES:.java=.class)

all: $(CLASS_FILES) $(CIBLE) jar

$(CIBLE): PP_jni.o $(PP_DIR)/PP_Client_Private.o $(PP_DIR)/PP_Error.o $(PP_CLIENT)/PP_Client.o
	g++ -shared -o $@ $^ $(OPTLIB)

PP_jni.o: PP_jni.cpp PP_jni.h $(PP_CLIENT)/PP_Client.h $(PP_DIR)/PP.h $(PP_DIR)/PP_Client_Private.h $(PP_DIR)/traces/TraceConstantList.h
	g++ -c PP_jni.cpp $(INCLUDE_DIRS) $(OPT)

PP_jni.h: pp/PPNative.class
	javah -o PP_jni.h pp.PPNative

%.class: %.java
	javac $<

clean:
	rm $(CIBLE) $(FILE_SUPPR) PP_jni.o PP_jni.h pp/*.class pp/exceptions/*.class pp.jar

doc: ./javadoc/index.html

# liste des fichiers .class
JAVA_EXCEP_FILES = $(wildcard ./pp/exceptions/*.java)

javadoc/index.html: pp/PP.java pp/Unit.java pp/PendingCommand.java $(JAVA_EXCEP_FILES)
	javadoc -docencoding "ISO-8859-15" -d javadoc -author pp/PP.java pp/Unit.java pp/PendingCommand.java pp/exceptions/*.java

jar: pp.jar

pp.jar: $(CLASS_FILES)
	jar cf pp.jar pp/*.class pp/exceptions/*.class

install: $(INSTALL_CLIENT)

install_client: ../compalgo/$(CIBLE)

../compalgo/$(CIBLE): $(CIBLE)
	cp $(CIBLE) ../compalgo
