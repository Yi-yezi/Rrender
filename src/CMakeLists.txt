file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(Rrender ${SOURCES} "core/Window.cpp")

configure_file(
  ${PROJECT_SOURCE_DIR}/include/project_root_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/project_root_config.h
  @ONLY
)

target_include_directories(Rrender PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}  # 包含生成的头文件路径
)

target_link_libraries(Rrender
    PRIVATE
        glad
        glfw
        glfw_link
        ImGui 
        stbimage
        tinyobjloader
        glm
        opengl32
)





