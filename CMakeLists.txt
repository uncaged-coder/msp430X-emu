cmake_minimum_required(VERSION 3.10)

project(EmulatorProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON) 
set(CMAKE_AUTOMOC ON)

# Add the necessary flags
add_compile_options(-Wall -g -O0)

# Find Qt packages
find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)

# Define common sources and includes
set(COMMON_CORE_SRCS ${PROJECT_SOURCE_DIR}/src/core)
set(COMMON_PERIPHERALS_SRCS ${PROJECT_SOURCE_DIR}/src/peripherals)
set(COMMON_UTILS_TESTS ${PROJECT_SOURCE_DIR}/tests/test_utils)

# Peripherals Library
file(GLOB PERIPHERALS_SRCS ${PROJECT_SOURCE_DIR}/src/peripherals/*.cpp)
add_library(PeripheralsLib ${PERIPHERALS_SRCS})
target_include_directories(PeripheralsLib PUBLIC ${COMMON_CORE_SRCS})
target_compile_features(PeripheralsLib PUBLIC cxx_std_17)

# Core Library
file(GLOB CORE_SRCS ${COMMON_CORE_SRCS}/*.cpp)
add_library(CoreLib ${CORE_SRCS})
target_include_directories(CoreLib PUBLIC ${COMMON_PERIPHERALS_SRCS})
target_compile_features(CoreLib PUBLIC cxx_std_17)
target_link_libraries(CoreLib PRIVATE PeripheralsLib)

# Tests Utils Library
file(GLOB UTILS_TESTS_SRCS ${COMMON_UTILS_TESTS}/*.cpp)
add_library(UtilsTestsLib ${UTILS_TESTS_SRCS})
target_include_directories(UtilsTestsLib PUBLIC ${COMMON_CORE_SRCS} ${COMMON_PERIPHERALS_SRCS})
target_compile_features(UtilsTestsLib PUBLIC cxx_std_17)

# Emulator Application
file(GLOB UI_SRCS src/ui/*.cpp src/ui/*.h)
add_executable(Emulator src/main.cpp ${UI_SRCS})
target_include_directories(Emulator PRIVATE ${PROJECT_SOURCE_DIR}/src/ui ${COMMON_CORE_SRCS} ${COMMON_PERIPHERALS_SRCS})
target_link_libraries(Emulator PRIVATE CoreLib Qt6::Core Qt6::Widgets Qt6::Gui)

# Unit Tests
file(GLOB UNIT_TEST_SOURCES tests/unit_tests/*.cpp)
add_executable(UnitTests ${UNIT_TEST_SOURCES})
target_include_directories(UnitTests PRIVATE ${COMMON_UTILS_TESTS})
target_link_libraries(UnitTests PRIVATE CoreLib UtilsTestsLib)

# ReplayDsScript Application
file(GLOB REPLAY_SOURCES tests/replay_ds_script/*.cpp)
add_executable(DsScriptReplay ${REPLAY_SOURCES})
target_include_directories(DsScriptReplay PRIVATE ${COMMON_UTILS_TESTS})
target_link_libraries(DsScriptReplay PRIVATE CoreLib UtilsTestsLib)

# clang format
add_custom_target(
    format
    COMMAND find ${CMAKE_SOURCE_DIR} -iname *.cpp -o -iname *.h | xargs clang-format -i -style=file
    COMMENT "Running clang-format"
    VERBATIM
)
