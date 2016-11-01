
.PHONY: all clean doc jar

LIBNAME = pp_java

ifeq ($(OS),Windows_NT)
PATH := /c/msys/1.0/bin:$(PATH)
ENTETELIB = 
LIB = .dll
OPT = 
OPTLIB = -Wl,--add-stdcall-alias
INCLUDES = . ../../pp
INCLUDE_DIRS = -I. -I../../pp -I"C:\Program Files\Java\jdk1.8.0_25\include"\
               -I"C:\Program Files\Java\jdk1.8.0_25\include\win32"
INSTALL_CLIENT = install_client
else
ENTETELIB = lib
LIB = .so
OPT = -fPIC
OPTLIB = -fPIC -lrt
INCLUDE_DIRS = -I. -I../../pp -I/usr/lib/jvm/java-6-openjdk/include/\
               -I/usr/lib/jvm/java-6-openjdk/include/linux/
INSTALL_CLIENT = 
endif

CIBLE = $(ENTETELIB)$(LIBNAME)$(LIB)

PP_DIR = ../../pp

JAVA_FILES = $(wildcard ./pp/*.java) $(wildcard ./pp/exceptions/*.java)
CLASS_FILES = $(JAVA_FILES:.java=.class)

all: $(CLASS_FILES) $(CIBLE)

$(CIBLE): PP_jni.o $(PP_DIR)/$(ENTETELIB)pp-client$(LIB)
	g++ -shared -o $@ $< $(OPTLIB) -L$(PP_DIR) -lpp-client

PP_jni.o: PP_jni.cpp PP_jni.h
	g++ -c PP_jni.cpp $(INCLUDE_DIRS) $(OPT)

PP_jni.h: pp/PPNative.class
	javah -o PP_jni.h pp.PPNative

%.class: %.java
	javac $<

clean:
	rm $(CIBLE) PP_jni.o PP_jni.h pp/*.class pp/exceptions/*.class

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
