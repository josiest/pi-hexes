set(CMAKE_CXX_STANDARD 23)

# find the SFML library
set(SFML_STATIC_LIBRARIES ON)
find_package(SFML COMPONENTS System Graphics Window)
find_package(pi-geometry REQUIRED)

# compilation target for simple example
add_executable(simple src/simple.cpp)
target_link_libraries(simple PRIVATE SFML::System SFML::Window SFML::Graphics pi::hexes pi::geometry)
target_compile_options(simple PRIVATE -fconcepts-diagnostics-depth=5)

# compilation target for highlight example
add_executable(highlight src/highlight.cpp)
target_link_libraries(highlight PRIVATE sfml-graphics sfml-system sfml-window pi::hexes)
target_compile_options(highlight PRIVATE -fconcepts-diagnostics-depth=5)
