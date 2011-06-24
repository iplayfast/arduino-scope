cmake_minimum_required(VERSION 2.8)

project(scope)

ADD_DEFINITIONS (-D WITH_SOUND)
#target_link_libraries(main ${EXTRA_LIBS})

#target_link_libraries(scope X11 GL GLU)
set(EXTRA_LIBS line sound)
add_library(line line.cpp)
add_library(sound sound.cpp)
add_executable(scope main.cpp TFuzzy.c  ${EXTRA_LIBS})
include_directories(/usr/include/pulse)


target_link_libraries(${PROJECT_NAME} X11 m pulse-simple GL GLU glut)
