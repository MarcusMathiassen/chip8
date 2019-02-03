#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stddef.h>

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* context = glfwCreateWindow(640, 480, "CHIP-8", NULL, NULL);

    glewInit();

    glfwMakeContextCurrent(context);

    glClearColor(0.02f, 0.02f, 0.02f, 1.0f);

    while (!glfwWindowShouldClose(context))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwWaitEvents();
    
        int width, height;
        glfwGetFramebufferSize(context, &width, &height);
        glViewport(0, 0, width, height);
    
        glEnable(GL_FRAMEBUFFER_SRGB);
    
        glBegin(GL_TRIANGLES);
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glVertex2f(-0.5, -0.5f);
        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
        glVertex2f(0.0f, 0.5f);
        glColor4f(0.0f, 0.0f, 1.0f, 1.0f);
        glVertex2f(0.5f, -0.5f);
        glEnd();

        glfwSwapBuffers(context);
    }
    return 0;
}
