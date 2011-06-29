cmake_minimum_required(VERSION 2.8)

project(scope)

ADD_DEFINITIONS (-D WITH_SOUND)
#target_link_libraries(main ${EXTRA_LIBS})

#target_link_libraries(scope X11 GL GLU)
set(EXTRA_SOURCES line.cpp sound.cpp fileseriesproducer.cpp TFuzzy.c)

add_executable(scope main.cpp ${EXTRA_SOURCES} ${EXTRA_LIBS})

include_directories(/usr/include/pulse)

target_link_libraries(${PROJECT_NAME} X11 m pulse-simple GL GLU glut)
