# The same minimum version supported by userver.
cmake_minimum_required(VERSION 3.14)

# Nothing but the project name and programming language to be used (ONLY!).
project(atlas LANGUAGES CXX)

# These components are used in the Atlas runtime.
set(USERVER_FEATURE_KAFKA ON)
set(USERVER_FEATURE_ROCKS ON)

# Add option to control building of project test suite
option(ATLAS_FEATURE_TESTSUITE "Build testsuite ('*_test.cpp' files)" OFF)

# The userver test feature is a dependency of the project's test suite.
set(USERVER_FEATURE_UTEST ${ATLAS_FEATURE_TESTSUITE})

# Since I plan to contribute to userver, we need it as a submodule.
set(USERVER_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/submodules/userver")
if(EXISTS ${USERVER_PROJECT_DIR})
	add_subdirectory(${USERVER_PROJECT_DIR})
else()
	message(FATAL_ERROR "Unable to find userver in the submodules directory")
endif()
userver_setup_environment()

# There are not many submodules now, so I prefer this to CPM.
set(JSON_PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/submodules/nlohmann_json")
if(EXISTS ${JSON_PROJECT_DIR})
	add_subdirectory(${JSON_PROJECT_DIR})
else()
	message(FATAL_ERROR "Unable to find json in the submodules directory")
endif()

# Common paths should not be duplicated throughout the build scenario.
set(PROJECT_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(ATLAS_SRC_DIR "${PROJECT_SRC_DIR}/atlas")

# Recursive search of source files of the main project components.
file(GLOB_RECURSE TESTSUITE_SRC_FILES "${ATLAS_SRC_DIR}/*_test.cpp")
# Exclude test suite files from the solana component's source file list.
file(GLOB_RECURSE SOLANA_SRC_FILES "${ATLAS_SRC_DIR}/solana/*.cpp")
list(REMOVE_ITEM SOLANA_SRC_FILES ${TESTSUITE_SRC_FILES} "")
# Exclude test suite files from the core component's source file list.
file(GLOB_RECURSE CORE_SRC_FILES "${ATLAS_SRC_DIR}/core/*.cpp")
list(REMOVE_ITEM CORE_SRC_FILES ${TESTSUITE_SRC_FILES} "")

# The library components are separated to allow them to be linked into tests.
add_library(${PROJECT_NAME} ${SOLANA_SRC_FILES} ${CORE_SRC_FILES})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

# It helps to organize the heading structure greatly when using a base anchor.
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SRC_DIR})

# Link userver and jsoncons to the project library.
find_package(userver COMPONENTS core kafka rocks)
target_link_libraries(${PROJECT_NAME} PUBLIC userver::kafka userver::rocks)
find_package(nlohmann_json)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json)

if(ATLAS_FEATURE_TESTSUITE)
	# Set name for the unit test executable.
	set(TESTSUITE_PROJECT_NAME "${PROJECT_NAME}_unittests")
	add_executable(${TESTSUITE_PROJECT_NAME} ${TESTSUITE_SRC_FILES})
	# Link userver test (UTest) component to the unit tests excutable.
	target_link_libraries(${TESTSUITE_PROJECT_NAME} PRIVATE
		${PROJECT_NAME} userver::utest)
	add_google_tests(${TESTSUITE_PROJECT_NAME})
endif()

# Define server executable and link it with the components library.
add_executable(server "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(server PRIVATE ${PROJECT_NAME})

# vim: set ts=4 sw=4 noexpandtab:

