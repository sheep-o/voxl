cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

link_directories("C:\\Users\\ramik\\Downloads\\glfw-3.4.bin.WIN64\\glfw-3.4.bin.WIN64\\lib-vc2022"
                 "C:\\Users\\ramik\\Downloads\\glew-2.3.1-win32\\glew-2.3.1\\lib\\Release\\x64"
                 "C:\\Users\\ramik\\Downloads\\glm-1.0.3\\glm\\build\\glm\\Debug")
include_directories("C:\\Users\\ramik\\Downloads\\glfw-3.4.bin.WIN64\\glfw-3.4.bin.WIN64\\include"
                    "C:\\Users\\ramik\\Downloads\\glew-2.3.1-win32\\glew-2.3.1\\include"
                    "C:\\Users\\ramik\\Downloads\\glm-1.0.3\\glm")

find_package(OpenGL REQUIRED)
add_executable(main src/main.cpp src/Game.cpp src/Render.cpp src/Shader.cpp src/Camera.cpp
                 src/Chunk.hpp
                 src/Chunk.cpp)
target_compile_features(main PRIVATE cxx_std_17)
target_compile_definitions(main PRIVATE GLEW_STATIC=1)
target_link_libraries(main PRIVATE OpenGL::GL glew32s glfw3 glm)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)