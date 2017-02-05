
.PHONY: all clean

all: compile_c compile_java compile_compalgo compile_ocaml compile_ada compile_python compile_scratch

compile_c:
	@echo "C interface ----------------------------------------------------\
--------------"
	@(cd ./c-cpp && $(MAKE) && $(MAKE) install)

compile_java:
	@echo "Java interface -------------------------------------------------\
--------------"
	@(cd ./java && $(MAKE) && $(MAKE) install && $(MAKE) jar && $(MAKE) doc)

compile_compalgo:
	@echo "Compalgo interface ---------------------------------------------\
--------------"
	@(cd ./compalgo && $(MAKE))

compile_ocaml:
	@echo "OCaml interface ------------------------------------------------\
--------------"
	@(cd ./ocaml && $(MAKE))

compile_ada:
	@echo "Ada interface --------------------------------------------------\
--------------"
	@(cd ./ada && $(MAKE) && $(MAKE) install)

compile_python:
	@echo "Python interface --------------------------------------------------\
--------------"
	@(cd ./python && $(MAKE) && $(MAKE) install)

compile_scratch:
	@echo "Scratch interface ----------------------------------------------\
--------------"
	@(cd ./scratch1.4 && $(MAKE))

clean:
	@echo "C interface cleaning -------------------------------------------\
--------------"
	@(cd ./c-cpp && $(MAKE) $@)
	@echo "Java interface cleaning ----------------------------------------\
--------------"
	@(cd ./java && $(MAKE) $@)
	@echo "Compalgo interface cleaning ------------------------------------\
--------------"
	@(cd ./compalgo && $(MAKE) $@)
	@echo "OCaml interface cleaning ---------------------------------------\
--------------"
	@(cd ./ocaml && $(MAKE) $@)
	@echo "Ada interface cleaning -----------------------------------------\
--------------"
	@(cd ./ada && $(MAKE) $@)
	@echo "Python interface cleaning -----------------------------------------\
--------------"
	@(cd ./python && $(MAKE) $@)
	@echo "Scratch interface cleaning -------------------------------------\
--------------"
	@(cd ./scratch1.4 && $(MAKE) $@)
