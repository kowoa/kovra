SRC_DIR := src
SHADERS_DIR := shaders
BUILD_DIR := build-output
SHADERBUILD_DIR := shaderbuild

SRC_FILES = $(wildcard $(SRC_DIR)/*)
SHADER_FILES = $(wildcard $(SHADERS_DIR)/*)

EXECUTABLE := $(BUILD_DIR)/src/kovra

# Ensure that SRC_FILES and SHADER_FILES are non-empty
ifeq ($(strip $(SRC_FILES)),)
$(error No source files found in $(SRC_DIR))
endif
ifeq ($(strip $(SHADER_FILES)),)
$(error No shader files found in $(SHADERS_DIR))
endif

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(EXECUTABLE): $(SRC_FILES) $(SHADER_FILES) | $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. && cmake --build .

.PHONY: build run clean

build: $(EXECUTABLE)

run: $(EXECUTABLE)
	$(EXECUTABLE)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(SHADERBUILD_DIR)

