.PHONY: help

GREEN := $(shell tput -Txterm setaf 2)
YELLOW := $(shell tput -Txterm setaf 3)
RESET := $(shell tput -Txterm sgr0)

help: ## Show this help.
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "${YELLOW}%-16s${GREEN}%s${RESET}\n", $$1, $$2}' $(MAKEFILE_LIST)

clean: ## Remove compiled files.
	rm -f main

build: ## Compile source code.
	g++ main.c ble.c common.c -L$(PWD)/WCHBLElib/ -Wl,-rpath=$(PWD)/WCHBLElib -lbluetooth -lWCHBLElib -o main
