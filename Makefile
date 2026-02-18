.DEFAULT_GOAL := help

BUILD_DIR := build

# AutoDoc
# -------------------------------------------------------------------------
.PHONY: help
help: ## Show this help
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "\033[36m%-30s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST)
.DEFAULT_GOAL := help

.PHONY: configure
configure: ## Configure CMake project
	cmake -S . -B $(BUILD_DIR) -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake

.PHONY: build
build: ## Build project
	cmake --build $(BUILD_DIR)

.PHONY: run
run: ##  Run the project
	./$(BUILD_DIR)/simpleengine

.PHONY: clean
clean: ## Remove build directory
	rm -rf $(BUILD_DIR)
