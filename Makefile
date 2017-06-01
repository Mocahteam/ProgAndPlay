
.PHONY: all clean install

export SPRING_VERSION = ../SpringPP/spring-0.82.5.1_official

# Try to find Spring folder
ifeq ($(wildcard $(SPRING_VERSION)),)
INSTALL_PP = not_found
else
INSTALL_PP = install_pp
endif

ifeq ($(OS),Windows_NT)
ECHO_OPT = -e
else
ECHO_OPT =
endif

YELLOW = \033[0;33m
GREEN = \033[0;32m
DEFAULT_COLOR = \033[0m

all: compile_pp $(INSTALL_PP) compile_client_interfaces end

compile_pp:
	@echo "==============================================================================="
	@echo "=                            Prog&Play Compilation                            ="
	@echo "==================================== (MAKE) ==================================="
	@(cd ./pp && $(MAKE))

install_pp:
	@echo ""
	@echo "==============================================================================="
	@echo "=                            Prog&Play Installation                           ="
	@echo "================================ (MAKE INSTALL) ==============================="
	@(cd ./pp && $(MAKE) install)

not_found:
	@echo ""
	@echo "==============================================================================="
	@echo "=                            Prog&Play Installation                           ="
	@echo "================================ (MAKE INSTALL) ==============================="
	@echo $(ECHO_OPT) "$(YELLOW)Installation aborted: $(SPRING_VERSION) doesn't exist.\nCheck if the target directory exists or update SPRING_VERSION value defined into this Makefile in agreement with your folder organisation.$(DEFAULT_COLOR)"

compile_client_interfaces:
	@echo ""
	@echo "==============================================================================="
	@echo "=                        Client Interfaces Compilation                        ="
	@echo "==============================================================================="
	@(cd ./Client_Interfaces && $(MAKE))

end:
	@echo ""
	@echo $(ECHO_OPT) "$(GREEN)Compilation succeed.$(DEFAULT_COLOR)"
	@echo ""

clean:
	@echo "Prog&Play cleaning ==================================================="
	@(cd ./pp && $(MAKE) $@)
	@echo "Client Interfaces cleaning =========================================="
	@(cd ./Client_Interfaces && $(MAKE) $@)
	@echo $(ECHO_OPT) "$(GREEN)Cleaning succeed.$(DEFAULT_COLOR)"
	@echo ""
