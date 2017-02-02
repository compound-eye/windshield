project(rover)
cmake_minimum_required(VERSION 2.8)

find_package(OpenCV REQUIRED)

include_directories(../think)

aux_source_directory(. SRC_LIST)
aux_source_directory(../think SRC_LIST)

add_executable(${PROJECT_NAME}
               ${SRC_LIST}
               ../think/queue.h
               Navio/PWM.cpp
               Navio/RCInput.cpp
               Navio/Util.cpp)

target_link_libraries(${PROJECT_NAME}
                      pigpio
                      pthread
                      ${OpenCV_LIBS})
