cmake_minimum_required (VERSION 3.3)

set(LANGUAGE "CXX")
project(binomial-pricer LANGUAGES ${LANGUAGE})

set(CMAKE_CXX_STANDARD 11)

# Find OpenCL package
find_package(OpenCL REQUIRED)

# Add dependencies to OpenCL library
set(LIBS ${LIBS} ${OpenCL_LIBRARY})
include_directories(${OpenCL_INCLUDE_DIR})
include_directories(SYSTEM ${OPENCL_INCLUDE_DIRS})
if(NOT OPENCL_HAS_CXX)
  message(WARNING "C++ bindings not found. Using Kronos supplied reference OpenCL v1.2 C++ bindings.")
  list(APPEND OpenCL_INCLUDE_DIRS ${SOURCE_DIR}/kronos)
endif()
add_definitions(${OPENCL_CXX_DEFINITIONS})
include_directories(utils PUBLIC src/CXX_common)

# Add subdirectories with other cmake config
add_subdirectory(src)
