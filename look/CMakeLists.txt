project(windshield)
cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(GLEW   REQUIRED)

include_directories(../think)

aux_source_directory(. SRC_LIST)
aux_source_directory(../think SRC_LIST)

add_executable(${PROJECT_NAME}
               ${SRC_LIST}
               ../think/queue.h
               ../think/VideoSource.h)

target_link_libraries(${PROJECT_NAME}
                      pthread
                      ${OpenCV_LIBS}
                      ${OPENGL_LIBRARIES}
                      ${GLUT_glut_LIBRARY}
                      ${GLEW_LIBRARIES})
