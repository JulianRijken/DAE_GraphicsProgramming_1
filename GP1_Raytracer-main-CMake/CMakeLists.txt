cmake_minimum_required(VERSION 3.18)

# Change project name here
set(PROJECT_NAME "game" CACHE STRING "Project name")

set(ASPECT_RATIO "4/3" CACHE STRING "Aspect ratio")
set(CLICK_TO_START ON CACHE BOOL "A click is required to activate sound on web")

# --------------------------------------
# no need to change anything after this!
# --------------------------------------

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/platform)

# Microsoft Windows support
if(WIN32)
    include(MicrosoftWindows)

    # on legacy platforms, we must download the libraries because there is no package manager :(
    MicrosoftWindows_SetupLibs()
else()
    set(CMAKE_CXX_FLAGS "-g -fPIC")
endif()

# enable C++20
set(CMAKE_CXX_STANDARD 20)

# for LSP support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project("${PROJECT_NAME}")

file(GLOB_RECURSE DIR_SRC source/*.cpp engine/*.cpp)
add_executable("${PROJECT_NAME}" ${DIR_SRC})


add_compile_options(-O3 -ffast-math -GA)


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()



if(EMSCRIPTEN)
    include(Emscripten)

elseif(WIN32)
    # legacy platforms require very painful library setups
    MicrosoftWindows_AddLibs()

else()
    find_package(SDL2 REQUIRED)
    target_link_libraries(${PROJECT_NAME} SDL2::Main)

    find_package(OpenGL)
    include_directories(${PROJECT_NAME} ${OPENGL_INCLUDE_DIRS})

    add_compile_options(-march=native -flto -Wl,-flto)
endif()

#find_package(OpenMP)
#if (OpenMP_FOUND)
#    target_link_libraries(${PROJECT_NAME} OpenMP::OpenMP_CXX)
#endif()

# include directories
include_directories(source)

# disable warnings for external libraries
file(GLOB_RECURSE EXTERNAL_DIR "source/**")
set_source_files_properties(
  ${EXTERNAL_DIR}
  PROPERTIES
  COMPILE_FLAGS "-w"
)

# symlink resources folder on supported platforms (sorry, Microsoft Windows!)
if(NOT WIN32)
    add_custom_command(TARGET "${PROJECT_NAME}" PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E create_symlink
                   ${CMAKE_SOURCE_DIR}/source/Resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources)
else()
    MicrosoftWindows_AddCompileCommands()
endif()
