
.PHONY: all clean

all: compile_pp compile_client_interfaces

compile_pp:
	@echo "compilation de Prog&Play ============================================="
	@(cd ./pp && $(MAKE) && $(MAKE) install)

compile_client_interfaces:
	@echo "compilation des interfaces Client ===================================="
	@(cd ./Client_Interfaces && $(MAKE))

clean:
	@echo "nettoyage de Prog&Play ==============================================="
	@(cd ./pp && $(MAKE) $@)
	@echo "nettoyage des interfaces Client ======================================"
	@(cd ./Client_Interfaces && $(MAKE) $@)