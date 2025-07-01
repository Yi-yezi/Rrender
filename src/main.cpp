#include <glad/glad.h>
#include "core/Window.h"
#include <iostream>

int main() {
    try {
        core::Window window(1280, 720, "Rrender");

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cerr << "Failed to initialize GLAD\n";
            return -1;
        }

        glEnable(GL_DEPTH_TEST);

        while (!window.ShouldClose()) {
            window.PollEvents();

            glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // TODO: äÖÈ¾²Ù×÷

            window.SwapBuffers();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
