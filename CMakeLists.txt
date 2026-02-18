cmake_minimum_required(VERSION 3.21)

project(simpleengine
    DESCRIPTION "A simple OpenGL-based game engine written in C++"
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(VERBOSE "*")
message(VERBOSE "* ${PROJECT_NAME} v${PROJECT_VERSION})")
message(VERBOSE "*")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLFW_INCLUDE_NONE")
find_package(glm CONFIG REQUIRED)
target_compile_definitions(${PROJECT_NAME} PRIVATE "GLM_ENABLE_EXPERIMENTAL")
find_package(Stb REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Stb_INCLUDE_DIR})
target_compile_definitions(${PROJECT_NAME} PRIVATE "STBI_FAILURE_USERMSG")
find_path(SIMPLEINI_INCLUDE_DIRS "SimpleIni.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${SIMPLEINI_INCLUDE_DIRS})
find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")
target_include_directories(${PROJECT_NAME} PRIVATE ${TINYGLTF_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
    glm::glm-header-only
    PRIVATE
    glad::glad
    glfw
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src) # to avoid using relative paths in includes

include(${CMAKE_CURRENT_LIST_DIR}/cmake/copy_assets.cmake)
