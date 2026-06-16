cmake_minimum_required(VERSION 3.28)
project(CMakeSFMLProject LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm CONFIG REQUIRED)

add_executable(main src/main.cpp src/Game.cpp src/Render.cpp src/Shader.cpp src/Camera.cpp)
target_compile_features(main PRIVATE cxx_std_17)
target_link_libraries(main PRIVATE glfw GLEW::GLEW OpenGL glm::glm)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)