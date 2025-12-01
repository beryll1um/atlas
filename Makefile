# Directory where the virtual environment will be created.
PACENV_DIR ?= $(CURDIR)/.pacenv

# Path to the pacenv configuration file.
PACENV_CONFIG ?= $(CURDIR)/pacenv.json

# Create the virtual environment if it does not already exist.
.PHONY: setup-pacenv
setup-pacenv:
ifeq (,$(wildcard $(PACENV_DIR)))
	@echo "The pacenv directory ($(PACENV_DIR)) does not exist. Creating..."
endif
ifneq (,$(PACENV_DIR))
	@pacenv -vc $(PACENV_CONFIG) $(PACENV_DIR)
endif

# Remove the virtual environment directory.
.PHONY: clean-pacenv
clean-pacenv:
	@rm -rf $(PACENV_DIR)

# Dependencies are installed in the virtual environment.
LD_LIBRARY_PATH := $(PACENV_DIR)/usr/lib/:$(LD_LIBRARY_PATH)
export LD_LIBRARY_PATH

# Prefix used to activate the virtual environment before executing commands.
PACENV_RUN := $(if $(PACENV_DIR),. $(PACENV_DIR)/usr/local/bin/activate &&)

# Directory where CMake will be set up.
BUILD_DIR ?= $(CURDIR)/build

# It would be nice if this was the default release,
# since the developers know what they are doing and the gnomes don't.
BUILD_TYPE ?= Release

# The compiler binary that will be used to compile the project.
COMPILER ?= clang++

# Directory where userver's source cache is stored.
CPM_SOURCE_CACHE ?= $(PACENV_DIR)/var/cache/userver

# By default tests are not included in build.
TESTSUITE ?= OFF

# Setup CMake if it does not already set up.
.PHONY: setup-cmake
setup-cmake: setup-pacenv
	# It's better to store userver cache locally to avoid
	# downloading everything again each time (especially Boost is quite big).
	@mkdir -p $(CPM_SOURCE_CACHE)
	# It's better to disable static libraries for now,
	# since I need to review them.
	@$(PACENV_RUN) cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_CXX_COMPILER=$(COMPILER) \
		-DUSERVER_USE_STATIC_LIBS=OFF -DUSERVER_FEATURE_STACKTRACE=OFF -DUSERVER_FEATURE_TESTSUITE=OFF \
		-DCPM_SOURCE_CACHE=$(CPM_SOURCE_CACHE) -DATLAS_FEATURE_TESTSUITE=$(TESTSUITE)

# Remove CMake build directory.
.PHONY: clean-cmake
clean-cmake:
	@rm -rf $(BUILD_DIR)

# Maximum number of build jobs that can be spawned.
J ?= 1

# Fast and convenient way to compile project without knowing any details.
.PHONY: compile
compile:
ifeq (,$(wildcard $(BUILD_DIR)))
	@$(MAKE) setup-cmake
endif
	@$(PACENV_RUN) cmake --build build -- -j $(J)

# Remove all build artifacts, including the virtual environment.
.PHONY: clean-all
clean-all: clean-pacenv clean-cmake
	@echo "All artifacts cleaned."

# vim: set ts=4 sw=4 noexpandtab:

