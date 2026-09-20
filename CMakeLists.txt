cmake_minimum_required(VERSION 4.4)
project(pi-hexes
        VERSION 0.3.0
        DESCRIPTION "Hex grid math"
        HOMEPAGE_URL "https://github.com/josiest/hexes"
        LANGUAGES CXX)

#
# Define the library
#

# add the interface library target
add_library(pi-hexes INTERFACE)
add_library(pi::hexes ALIAS pi-hexes)

set_target_properties(pi-hexes PROPERTIES
        SOVERSION 0
        EXPORT_NAME hexes)

target_sources(pi-hexes
    INTERFACE FILE_SET HEADERS
    BASE_DIRS include
    FILES
       include/pi/hexes.hpp)

#
# Export and install library targets
#

include(InterfaceLibraryHelpers.cmake)
install_pi_interface_targets(hexes)
install_pi_package(hexes)

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
