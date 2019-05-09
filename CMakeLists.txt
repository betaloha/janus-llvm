SET(CMAKE_CXX_FLAGS "-Wall -fno-rtti")
set (CMAKE_CXX_STANDARD 11)
find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

add_subdirectory(outpass)
