#ifdef __EMSCRIPTEN__
    #include <emscripten.h>
    #define GLFW_INCLUDE_ES3
#else
    #include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>

#include "OpenDriveMap.h"
#include "OrbitControls.h"
#include "RoadNetworkMesh.h"
#include "utils.hpp"

#include <algorithm>
#include <cstdlib>
#include <stdio.h>
#include <vector>

const char* vertex_shader_src = "#version 100\n"
                                "attribute vec3 xyz;\n"
                                "uniform mat4 MVP;\n"
                                "void main(){\n"
                                "   gl_Position = MVP * vec4(xyz, 1);\n"
                                "}\0";

const char* fragment_shader_src = "#version 100\n"
                                  "precision mediump float;\n"
                                  "void main(){\n"
                                  "    gl_FragColor = vec4(1.0, 0.5, 0.2, 1.0);\n"
                                  "}\n";

const double    FOV = 70;
const double    NEAR_PLANE = 0.1;
const double    FAR_PLANE = 1000.0;
const glm::vec3 UP = glm::vec3(0, 0, 1);

glm::vec3 pan_start = glm::vec3(0, 0, -1);
glm::vec3 rotate_start = glm::vec3(0, 0, -1);
float     zoom_delta = 0;

int window_width = 800;
int window_height = 600;

bool shader_ok(const GLuint& shader)
{
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    char info_log[512];
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", info_log);
    }

    return success;
}

bool program_ok(const GLuint& program)
{
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    char info_log[512];
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", info_log);
    }

    return success;
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    window_height = height;
    window_width = width;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            double x_pos, y_pos;
            glfwGetCursorPos(window, &x_pos, &y_pos);
            if (mods == GLFW_MOD_SHIFT)
            {
                rotate_start = glm::vec3(x_pos, y_pos, 0);
            }
            else
            {
                pan_start = glm::vec3(x_pos, y_pos, 0);
            }
        }
        else if (action == GLFW_RELEASE)
        {
            pan_start[2] = -1;
            rotate_start[2] = -1;
        }
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { zoom_delta += yoffset; }

std::function<void()> loop;
void                  main_loop() { loop(); }

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("ERROR: too few arguments\n");
        return -1;
    }
    odr::OpenDriveMap    odr(argv[1]);
    odr::RoadNetworkMesh road_network_mesh = odr.get_mesh(0.1);

    std::vector<float> vertices;
    vertices.reserve(road_network_mesh.lanes_mesh.vertices.size());
    for (const auto& vert : road_network_mesh.lanes_mesh.vertices)
    {
        vertices.push_back(vert[0]);
        vertices.push_back(vert[1]);
        vertices.push_back(vert[2]);
    }

    std::vector<unsigned int> indices;
    indices.reserve(road_network_mesh.lanes_mesh.indices.size());
    for (const auto& idx : road_network_mesh.lanes_mesh.indices)
        indices.push_back(idx);

    if (!glfwInit())
    {
        printf("fail!\n");
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#endif

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "ObjViewer", NULL, NULL);
    if (!window)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);

#ifdef __EMSCRIPTEN__
#else
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize OpenGL context\n");
        exit(EXIT_FAILURE);
    }
#endif

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));

    glfwSwapInterval(0);

    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader);
    if (!shader_ok(vertex_shader))
        exit(EXIT_FAILURE);

    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(fragment_shader);
    if (!shader_ok(fragment_shader))
        exit(EXIT_FAILURE);

    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    if (!program_ok(program))
        exit(EXIT_FAILURE);

    // const float vertices[] = {2.0, 1.0, 0.0, -2.0, 1.0, 0.0, 0.0, 1.0, 3.0, 2.0, 5.0, 0.0, -2.0, 5.0, 0.0, 0.0, 5.0, 3.0};

    // unsigned int indices[] = {0, 1, 2, 3, 5, 4};

    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

    GLuint indices_buffer;
    glGenBuffers(1, &indices_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    OrbitControls orbit_controls(glm::vec3(2, -3, 1), glm::vec3(0, 0, 0), UP);

    GLuint    matrix_id = glGetUniformLocation(program, "MVP");
    glm::mat4 projection = glm::perspective(glm::radians(FOV), (double)(window_width / window_height), NEAR_PLANE, FAR_PLANE);
    glm::mat4 view = glm::lookAt(orbit_controls.cam_position, orbit_controls.cam_target, UP);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 mvp = projection * view * model;

    glUseProgram(program);

    loop = [&]
    {
        double x_pos, y_pos;
        glfwGetCursorPos(window, &x_pos, &y_pos);

        if (rotate_start[2] == 0)
        {
            const glm::vec3 rotate_end = glm::vec3(x_pos, y_pos, 0);
            const glm::vec2 rotate_dxy = glm::vec2(rotate_end - rotate_start);
            const glm::vec2 rotate_left_up = rotate_dxy * 2.0f * (float)M_PI / (float)window_height;
            orbit_controls.rotate(rotate_left_up[0], rotate_left_up[1]);
            rotate_start = rotate_end;
        }

        if (zoom_delta != 0)
        {
            orbit_controls.zoom(zoom_delta);
            zoom_delta = 0;
        }

        if (pan_start[2] == 0)
        {
            const glm::vec3 pan_end = glm::vec3(x_pos, y_pos, 0);
            const glm::vec2 pan_dxy = glm::vec2(pan_end - pan_start);
            const float target_distance = glm::length(orbit_controls.cam_target - orbit_controls.cam_position) * glm::tan((FOV / 2.0) * M_PI / 180.0);
            const glm::vec2 pan_left_up = pan_dxy * 2.0f * target_distance / (float)window_height;
            orbit_controls.pan(pan_left_up[0], pan_left_up[1]);
            pan_start = pan_end;
        }

        view = glm::lookAt(orbit_controls.cam_position, orbit_controls.cam_target, UP);
        mvp = projection * view * model;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);
        glEnableVertexAttribArray(0);

        glBindVertexArray(vertex_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_buffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glDrawElements(GL_TRIANGLES, vertices.size(), GL_UNSIGNED_INT, 0);
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    };

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(main_loop, 0, true);
#else
    while (!glfwWindowShouldClose(window))
        main_loop();
#endif

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}