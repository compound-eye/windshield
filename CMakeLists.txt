project(windshield)
cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT   REQUIRED)
find_package(GLEW   REQUIRED)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME}
               ${SRC_LIST}
               queue.h
               VideoSource.h
               View.h)

target_link_libraries(${PROJECT_NAME}
                      pthread
                      ${OpenCV_LIBS}
                      ${OPENGL_LIBRARIES}
                      ${GLUT_LIBRARY}
                      ${GLEW_LIBRARIES})
