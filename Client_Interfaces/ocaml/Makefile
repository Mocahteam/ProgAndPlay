.PHONY: all clean

LIBNAME = pp-client-ocaml

SPRING_VERSION = spring-0.82.5.1

ifeq ($(OS),Windows_NT)
# Compile with g++ 3.4.5 instead of 4.4 => crash on execution
GCC3.4.5 = MinGW-gcc3.4.5
PATH := C:\$(GCC3.4.5)\bin;C:\msys\1.0\bin;$(PATH)
ENTETELIB = dll
ENTETELIB2 =
LIB = .dll
export OCAMLLIB=C:\OCaml\lib
INCLUDE_OS = $(OCAMLLIB)
# In order to find "dllcrt2.o" file
OPTLIB = -L"C:\$(GCC3.4.5)\lib" -ldopt -noentry
else
ENTETELIB = lib
ENTETELIB2 = lib
LIB = .so
INCLUDE_OS =
OPTLIB = -lrt -ldopt -fPIC
endif

PP_DIR = ../../pp
PP_CLIENT = ../c-cpp
INCLUDE_DIRS = . $(PP_DIR) $(PP_CLIENT) $(INCLUDE_OS) 

TARGETLIB = $(ENTETELIB)$(LIBNAME)$(LIB)
CIBLES = $(TARGETLIB)

# Addition of "-I" in front of each source directories
INCLUDES = $(foreach tmp, $(INCLUDE_DIRS),-I$(tmp)) 

all: pp.cma

pp.cma: pp.cmo $(TARGETLIB)
	ocamlc -a -o pp.cma pp.cmo -dllib $(TARGETLIB)

pp.cmo: pp.ml
	ocamlc -c pp.ml

$(TARGETLIB): pp_ocaml.o $(PP_CLIENT)/$(ENTETELIB2)pp-client-c$(LIB)
	ocamlmklib -oc $(LIBNAME) pp_ocaml.o -lpp-client-c -L$(PP_CLIENT) $(OPTLIB)

pp_ocaml.o: pp_ocaml.c
	g++ -o pp_ocaml.o $(INCLUDES) -c pp_ocaml.c

clean:
	rm pp.cmi pp.cma pp.cmo pp_ocaml.o $(TARGETLIB) lib$(LIBNAME).a