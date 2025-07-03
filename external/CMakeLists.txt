# GLAD
add_library(glad STATIC GLAD/src/glad.c)
target_include_directories(glad PUBLIC GLAD/include)

# GLFW（预编译）
add_library(glfw INTERFACE)
target_include_directories(glfw INTERFACE ${PROJECT_SOURCE_DIR}/external/GLFW)
add_library(glfw_link STATIC IMPORTED GLOBAL)
set_target_properties(glfw_link PROPERTIES
    IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/libs/glfw3.lib)

# GLM（头文件-only）
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${PROJECT_SOURCE_DIR}/external/glm)

# imgui
set(IMGUI_DIR  ${PROJECT_SOURCE_DIR}/external/imgui)
file(GLOB IMGUI_SOURCES
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
)
add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(ImGui PRIVATE glfw)

