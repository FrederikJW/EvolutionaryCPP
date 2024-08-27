# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.10)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Project name
project(EvolutionaryCPP)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/CPPJovanovic/*.cpp
    ${PROJECT_SOURCE_DIR}/src/graph/*.cpp
    ${PROJECT_SOURCE_DIR}/src/partition/*.cpp
    ${PROJECT_SOURCE_DIR}/src/population/*.cpp
    ${PROJECT_SOURCE_DIR}/src/strategies/*.cpp
    ${PROJECT_SOURCE_DIR}/src/util/*.cpp
)

# Add the vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "D:/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Add TBB
find_package(TBB CONFIG REQUIRED)

# Executable
add_executable(EvolutionaryCPP ${SOURCES})

# Link TBB
target_link_libraries(EvolutionaryCPP TBB::tbb)

include_directories(${TBB_INCLUDE_DIRS})

target_compile_options(EvolutionaryCPP PRIVATE ${TBB_CXX_FLAGS})
