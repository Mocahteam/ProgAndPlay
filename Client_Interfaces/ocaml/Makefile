.PHONY: all clean

ifeq ($(OS),Windows_NT)
# Compile with g++ 3.4.5 instead of 4.4 => crash on execution
GCC3.4.5 = MinGW-gcc3.4.5
PATH := C:\$(GCC3.4.5)\bin;C:\msys\1.0\bin;$(PATH)
ENTETELIB = 
LIB = .dll
INCLUDE_DIRS = -I../../pp -I"C:\OCaml\lib"
# In order to find "dllcrt2.o" file
OPTLIB = -L"C:\$(GCC3.4.5)\lib" -ldopt -noentry
else
ENTETELIB = lib
LIB = .so
INCLUDE_DIRS = -I../../pp -I.
OPTLIB = -lrt -ldopt -fPIC
endif

PP_DIR = ../../pp

all: pp.cma

pp.cma: pp.cmo dllpp_ocaml$(LIB)
	ocamlc -a -o pp.cma pp.cmo -dllib dllpp_ocaml$(LIB)

pp.cmo: pp.ml
	ocamlc -c pp.ml

dllpp_ocaml$(LIB): pp_ocaml.o $(PP_DIR)/$(ENTETELIB)pp-client$(LIB)
	ocamlmklib -oc pp_ocaml pp_ocaml.o -lpp-client -L$(PP_DIR) $(OPTLIB)

pp_ocaml.o: pp_ocaml.c
	g++ -o pp_ocaml.o $(INCLUDE_DIRS) -c pp_ocaml.c

clean:
	rm pp.cmi pp.cma pp.cmo pp_ocaml.o dllpp_ocaml$(LIB) libpp_ocaml.a