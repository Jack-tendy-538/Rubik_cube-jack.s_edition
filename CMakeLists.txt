cmake_minimum_required(VERSION 3.10)
project(RubikCubeCPP)
set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
# glad is assumed to be provided as a source (or submodule). You may also use system glad.

add_executable(RubikCube src/RubikCube.cpp)
# Link with glfw and math libraries; adjust as necessary for your environment
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
if (GLFW_FOUND)
    target_link_libraries(RubikCube PRIVATE ${GLFW_LIBRARIES})
endif()
# If using system glm
if (TARGET glm::glm)
    target_link_libraries(RubikCube PRIVATE glm::glm)
endif()

# If you have glad as a source or library, link it here. Otherwise, you'll need to provide it.
# target_link_libraries(RubikCube PRIVATE glad)

# On Linux you may need to link X11 / pthreads / dl
if(UNIX)
    target_link_libraries(RubikCube PRIVATE dl pthread)
endif()