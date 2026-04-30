cmake_minimum_required(VERSION 3.29)
project(Proyecto_AED)

set(CMAKE_CXX_STANDARD 23)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System)

add_executable(Proyecto_AED
        main.cpp
        matrix/Node.h
        matrix/Header.h
        matrix/SparseMatrix.h
        matrix/SparseMatrix.cpp
        matrix/MatrixRenderer.h
        matrix/MatrixRenderer.cpp
        matrix/DetailNode.h
)

target_link_libraries(Proyecto_AED
        SFML::Graphics
        SFML::Window
        SFML::System
)

# 🔥 IMPORTANTE: copiar fuentes al runtime
file(COPY fuente DESTINATION ${CMAKE_BINARY_DIR})