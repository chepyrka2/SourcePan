cmake_minimum_required(VERSION 3.16)

project(project
        VERSION 1.0
        LANGUAGES C CXX
)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -g")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(
        raylib REQUIRED
)

find_package(
        miniz REQUIRED
)

# --- inih library ---
add_library(inih
        inih/ini.c
        inih/INIReader.cpp
)

target_include_directories(inih
        PUBLIC
        inih
)

# --- core sources ---
add_library(core
        src/Levenshtein.cpp
        src/Recipe.cpp
        src/SRCPANmngr.cpp
)

target_include_directories(core
        PUBLIC
        src/headers
)

target_link_libraries(core
        PUBLIC
        inih
)

# --- main executable ---
add_executable(project
        src/main.cpp
)

#add_executable( RecipeMaker
#        src/RecipeMaker.cpp
#)
#
#target_link_libraries(RecipeMaker
#        PRIVATE
#        core
#)

target_link_libraries(project
        PRIVATE
        core
)

target_link_libraries(project PRIVATE raylib)
target_link_libraries(project PRIVATE miniz)