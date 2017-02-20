
.PHONY: all clean

all: compile_pp compile_client_interfaces

compile_pp:
	@echo "==============================================================================="
	@echo "=                          Compilation de Prog&Play                           ="
	@echo "==================================== (MAKE) ==================================="
	@(cd ./pp && $(MAKE))
	@echo ""
	@echo "==============================================================================="
	@echo "=                          Compilation de Prog&Play                           ="
	@echo "================================ (MAKE INSTALL) ==============================="
	@(cd ./pp && $(MAKE) install)

compile_client_interfaces:
	@echo ""
	@echo "==============================================================================="
	@echo "=                      Compilation des interfaces Client                      ="
	@echo "==============================================================================="
	@(cd ./Client_Interfaces && $(MAKE))

clean:
	@echo "Nettoyage de Prog&Play ==============================================="
	@(cd ./pp && $(MAKE) $@)
	@echo "Nettoyage des interfaces Client ======================================"
	@(cd ./Client_Interfaces && $(MAKE) $@)