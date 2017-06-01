
.PHONY: all clean doc exemples install

LIBNAME = pp-client-java

ifeq ($(OS),Windows_NT)
PATH := /c/msys/1.0/bin:$(PATH)
ENTETELIB = 
LIB = .dll
OPT = 
OPTLIB = -L../../$(SPRING_VERSION)/mingwlibs/lib -lboost_thread-mt -Wl,--add-stdcall-alias,--output-def,$(LIBNAME).def,--out-implib,lib$(LIBNAME).a
FILE_SUPPR = $(LIBNAME).def lib$(LIBNAME).a
INCLUDE_OS = -I"$(JAVA_HOME)" -I"$(JAVA_HOME)\include" -I"$(JAVA_HOME)\include\win32"
else
ENTETELIB = lib
LIB = .so
OPT = -fPIC
OPTLIB = -lboost_thread -fPIC -lrt
FILE_SUPPR = 
# We try to found java
ifeq ($(JAVA_HOME),)
JAVA_HOME = $(shell dirname $(shell dirname $(shell readlink -f $(shell which javac))))
endif
INCLUDE_OS = -I$(JAVA_HOME) -I$(JAVA_HOME)/include/ -I$(JAVA_HOME)/include/linux
endif

CIBLE = $(ENTETELIB)$(LIBNAME)$(LIB)

PP_DIR = ../../pp
PP_CLIENT = ../c-cpp

INCLUDE_DIRS = -I. -I$(PP_DIR) -I$(PP_CLIENT) $(INCLUDE_OS)

JAVA_FILES = $(wildcard ./pp/*.java) $(wildcard ./pp/exceptions/*.java)
CLASS_FILES = $(JAVA_FILES:.java=.class)

all: $(CIBLE) pp.jar

$(CIBLE): PP_jni.o $(PP_DIR)/PP_Client_Private.o $(PP_DIR)/PP_Error.o $(PP_CLIENT)/PP_Client.o
	g++ -shared -o $@ $^ $(OPTLIB)

PP_jni.o: PP_jni.cpp PP_jni.h $(PP_CLIENT)/PP_Client.h $(PP_DIR)/PP.h $(PP_DIR)/PP_Client_Private.h $(PP_DIR)/traces/TraceConstantList.h
	g++ -c PP_jni.cpp $(INCLUDE_DIRS) $(OPT)

PP_jni.h: pp/PPNative.class
	javah -o PP_jni.h pp.PPNative

%.class: %.java
	javac $<

clean:
	rm -f $(CIBLE) $(FILE_SUPPR) PP_jni.o PP_jni.h pp/*.class pp/exceptions/*.class pp.jar exemple/*.class ../compalgo/$(CIBLE)

doc: ./javadoc/index.html

# liste des fichiers .class
JAVA_EXCEP_FILES = $(wildcard ./pp/exceptions/*.java)

javadoc/index.html: pp/PP.java pp/Unit.java pp/PendingCommand.java $(JAVA_EXCEP_FILES)
	javadoc -docencoding "ISO-8859-15" -d javadoc -author pp/PP.java pp/Unit.java pp/PendingCommand.java pp/exceptions/*.java

pp.jar: $(CLASS_FILES)
	jar cf pp.jar pp/*.class pp/exceptions/*.class

# liste des fichiers exemples
EXEMPLES_JAVA_FILES = $(wildcard ./exemple/*.java)
EXEMPLES_CLASS_FILES = $(EXEMPLES_JAVA_FILES:.java=.class)
exemples: $(EXEMPLES_CLASS_FILES)

exemple/%.class: exemple/%.java pp.jar $(CIBLE)
	javac $<
	
install: ../compalgo/$(CIBLE)

../compalgo/$(CIBLE): $(CIBLE)
	cp $(CIBLE) ../compalgo
