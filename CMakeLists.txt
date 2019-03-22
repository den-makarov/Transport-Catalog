cmake_minimum_required (VERSION 3.10.2)

# build configurations
option (RELEASE_BUILD "release build option" OFF)
message ("RELEASE_BUILD: ${RELEASE_BUILD}")

# common compilation and linking flags
# set (CMAKE_C_COMPILER "/usr/bin")
# set (CMAKE_CXX_COMPILER "/usr/bin")
# set (CMAKE_C_FLAGS_INIT "${ARCH_FLAGS}")
# set (CMAKE_CXX_FLAGS_INIT "${ARCH_FLAGS}")
# set (CMAKE_EXE_LINKER_FLAGS_INIT "${ARCH_FLAGS} -lc -T${CMAKE_SOURCE_DIR}/linker_script.ld -Wl,--gc-sections")

# generic target
set (CMAKE_SYSTEM_NAME Generic)
project (Transport-Catalog-A C CXX)

# compiler bins location
get_filename_component(C_COMPILER_PATH ${CMAKE_C_COMPILER} DIRECTORY)
set (TARGET_NAME application)

# preprocessor defintions
# add_definitions(-DSOMETHING_TO_DEFINE)

# additional compilation flags
 if (RELEASE_BUILD)
    add_compile_options(-Os -g0)
#    set (TARGET_NAME ${TARGET_NAME}-release)
 else ()
    add_compile_options(-O0 -g3)
#    set (TARGET_NAME ${TARGET_NAME}-debug)
 endif (RELEASE_BUILD)
add_compile_options(-std=c++1z)

# source dirs
set (APP_DIR   src)
set (LIB_DIR   src/lib)
set (OBJ_DIR   src/objects)

# include dirs
include_directories(
    ${APP_DIR}/inc
)

# application source files
add_library(app_lib OBJECT
    ${APP_DIR}/main.cpp
)

# additional library source files
add_library(sub_lib OBJECT
    ${LIB_DIR}/stringparser.cpp
    ${LIB_DIR}/request.cpp
    ${LIB_DIR}/json.cpp
)

# objects source files
add_library(obj_lib OBJECT
    ${OBJ_DIR}/bus.cpp
    ${OBJ_DIR}/busstop.cpp
    ${OBJ_DIR}/BusRoute.cpp
    ${OBJ_DIR}/BusStopMap.cpp
)

# executable
add_executable(${TARGET_NAME}
    $<TARGET_OBJECTS:app_lib>
    $<TARGET_OBJECTS:sub_lib>
    $<TARGET_OBJECTS:obj_lib>
)

# post build commands
add_custom_command(TARGET ${TARGET_NAME}
    COMMAND ${C_COMPILER_PATH}/size ${TARGET_NAME}
    COMMENT "--- size of sections ---"
)
