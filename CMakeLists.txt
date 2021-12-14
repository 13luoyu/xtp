cmake_minimum_required(VERSION 3.14)	
project(XTP)
add_definitions(-std=c++17)
add_definitions(-mcmodel=large)

file(GLOB APP_HEAD ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB APP_SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(XTP ${APP_HEAD} ${APP_SRC})

include_directories(${PROJECT_SOURCE_DIR}/api)
include_directories(${PROJECT_SOURCE_DIR}/include)

link_directories(${PROJECT_SOURCE_DIR}/api)
target_link_libraries(XTP ${PROJECT_SOURCE_DIR}/api/libxtpquoteapi.so)
target_link_libraries(XTP ${PROJECT_SOURCE_DIR}/api/libxtptraderapi.so)
target_link_libraries(XTP pthread)
