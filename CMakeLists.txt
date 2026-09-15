# set cmake and compiler options
cmake_minimum_required(VERSION 4.4)
project(tess VERSION 0.2.0)

#
# Define the library
#

# add the interface library target
add_library(tess INTERFACE)
add_library(tess::tess ALIAS tess)
target_compile_features(tess INTERFACE cxx_std_23)

include(GNUInstallDirs)
target_sources(tess
    INTERFACE FILE_SET HEADERS
    BASE_DIRS include
    FILES
       ${CMAKE_SOURCE_DIR}/include/tess/basis.hpp
       ${CMAKE_SOURCE_DIR}/include/tess/hex.hpp
       ${CMAKE_SOURCE_DIR}/include/tess/math.hpp
       ${CMAKE_SOURCE_DIR}/include/tess/tess.hpp)

#
# Export and install library targets
#

include(InterfaceLibraryHelpers.cmake)
install_interface_targets(tess)
install_package(tess)

# Add option to build examples
option(EXAMPLES "Build Examples" ON)
if (EXAMPLES)
    add_subdirectory(examples)
endif()

# Add an option to compile the documentation
option(DOCS "Compile documentation" OFF)
if (DOCS)
    add_subdirectory(docs)
endif()

# Add an option to compile tests
option(TESTING "Compile tests" OFF)
if (TESTING)
    add_subdirectory(test)
endif()
