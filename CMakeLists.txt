cmake_minimum_required(VERSION 3.19)
project(miniSQL)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

set(INCLUDE_SRC
        src/include/table.cpp
        )

set(INDEX_SRC
        src/IndexManager/BpTree.cpp
        src/IndexManager/BpTreeNode.cpp
        src/IndexManager/BpKey.cpp
        src/IndexManager/IndexManager.cpp
        )

set(BUFFER_SRC
        src/BufferManager/bufferPoolManager.h
        src/BufferManager/lruReplacer.h
        src/BufferManager/page.h
        )

set(CATALOG_SRC
        src/CatalogManager/CatalogManager.cpp
        )

set(RECORD_SRC
        src/RecordManager/RecordFunction.cpp
        src/RecordManager/RecordManager.cpp
        )

set(API_SRC
        src/API/API.cpp
        )

set(RECORD_SRC
        src/RecordManager/RecordManager.cpp
        src/RecordManager/RecordFunction.cpp
        )

set(INTERPRETER_SRC
        src/Interpreter/Interpreter.cpp)

#set(INDEX_DEBUG
#        src/IndexManager/BufferManagerDemo.cpp
#        )

add_executable(miniSQL
        src/main.cpp
        ${INCLUDE_SRC}
        ${INDEX_SRC}
        ${CATALOG_SRC}
        ${BUFFER_SRC}
        ${API_SRC}
        ${RECORD_SRC}
        ${INTERPRETER_SRC}
        )