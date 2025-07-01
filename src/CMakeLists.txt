file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)

add_executable(Rrender ${SOURCES} "core/Window.cpp")

target_include_directories(Rrender PRIVATE ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(Rrender
    PRIVATE
        glad
        glfw
        glfw_link
        ImGui 
        glm
        opengl32
)
