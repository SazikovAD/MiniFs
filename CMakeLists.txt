cmake_minimum_required(VERSION 3.16)
project(MiniFS C)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")

set(MINI_FS_LIBRARY MiniFsLib)
set(MINI_FS_EXECUTABLE MiniFs)

add_library(${MINI_FS_LIBRARY} STATIC
        Src/Config.c
        Src/FsFileApi.c
        Src/FileApi.c
        Src/FsApi.c
        Src/Api.c
        )
target_include_directories(${MINI_FS_LIBRARY} PUBLIC Include)
target_link_libraries(${MINI_FS_LIBRARY} m)

add_executable(${MINI_FS_EXECUTABLE} Src/Main.c)
target_link_libraries(${MINI_FS_EXECUTABLE} ${MINI_FS_LIBRARY})