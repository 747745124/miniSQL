project(miniSQL)

set(CMAKE_CXX_STANDARD 14)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

add_executable(miniSQL main.cpp Interpreter.cpp)