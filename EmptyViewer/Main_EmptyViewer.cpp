#define _USE_MATH_DEFINES
#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

using namespace glm;

const int WIDTH = 512;
const int HEIGHT = 512;

std::vector<vec3> gVertices;
std::vector<int> gIndexBuffer;
std::vector<float> DepthBuffer(WIDTH* HEIGHT, 1e9f);
std::vector<float> OutputImage(WIDTH* HEIGHT * 3, 0.0f);

void create_scene() {
    int width = 32;
    int height = 16;
    float theta, phi;

    gVertices.resize((height - 2) * width + 2);
    gIndexBuffer.resize((height - 2) * (width - 1) * 6 + 6 * (width - 1));

    int t = 0;
    for (int j = 1; j < height - 1; ++j) {
        for (int i = 0; i < width; ++i) {
            theta = (float)j / (height - 1) * M_PI;
            phi = (float)i / (width - 1) * 2 * M_PI;
            float x = sinf(theta) * cosf(phi);
            float y = cosf(theta);
            float z = -sinf(theta) * sinf(phi);
            gVertices[t++] = vec3(x, y, z);
        }
    }
    gVertices[t++] = vec3(0, 1, 0);
    gVertices[t++] = vec3(0, -1, 0);

    t = 0;
    for (int j = 0; j < height - 3; ++j) {
        for (int i = 0; i < width - 1; ++i) {
            gIndexBuffer[t++] = j * width + i;
            gIndexBuffer[t++] = (j + 1) * width + (i + 1);
            gIndexBuffer[t++] = j * width + (i + 1);
            gIndexBuffer[t++] = j * width + i;
            gIndexBuffer[t++] = (j + 1) * width + i;
            gIndexBuffer[t++] = (j + 1) * width + (i + 1);
        }
    }
    for (int i = 0; i < width - 1; ++i) {
        gIndexBuffer[t++] = (height - 2) * width;
        gIndexBuffer[t++] = i;
        gIndexBuffer[t++] = i + 1;
        gIndexBuffer[t++] = (height - 2) * width + 1;
        gIndexBuffer[t++] = (height - 3) * width + (i + 1);
        gIndexBuffer[t++] = (height - 3) * width + i;
    }
}

vec3 compute_phong_lighting(vec3 pos, vec3 normal) {
    // Material properties
    vec3 ka(0.0f, 1.0f, 0.0f);
    vec3 kd(0.0f, 0.5f, 0.0f);
    vec3 ks(0.5f, 0.5f, 0.5f);
    float p = 32.0f;

    // Light properties
    vec3 lightPos(4.0f, -4.0f, 3.0f);
    vec3 viewPos(0.0f, 0.0f, 0.0f);
    float Ia = 0.2f;

    // Lighting calculations
    vec3 L = normalize(lightPos - pos);
    vec3 V = normalize(viewPos - pos);
    vec3 R = reflect(-L, normal);

    vec3 ambient = Ia * ka;
    vec3 diffuse = kd * std::max(dot(normal, L), 0.0f);
    vec3 specular = ks * pow(std::max(dot(R, V), 0.0f), p);

    vec3 color = ambient + diffuse + specular;

    // Gamma correction
    color.r = pow(color.r, 1.0f / 2.2f);
    color.g = pow(color.g, 1.0f / 2.2f);
    color.b = pow(color.b, 1.0f / 2.2f);

    return clamp(color, 0.0f, 1.0f);
}

void rasterize_triangle_gouraud(vec4 v0, vec4 v1, vec4 v2,
    vec3 c0, vec3 c1, vec3 c2) {
    vec3 p0 = vec3(v0) / v0.w;
    vec3 p1 = vec3(v1) / v1.w;
    vec3 p2 = vec3(v2) / v2.w;

    auto to_screen = [](vec3 p) -> vec2 {
        return vec2((p.x + 0.5f) * WIDTH, (p.y + 0.5f) * HEIGHT);
        };

    vec2 s0 = to_screen(p0);
    vec2 s1 = to_screen(p1);
    vec2 s2 = to_screen(p2);

    float minX = std::max(0.0f, floor(std::min({ s0.x, s1.x, s2.x })));
    float maxX = std::min((float)WIDTH - 1, ceil(std::max({ s0.x, s1.x, s2.x })));
    float minY = std::max(0.0f, floor(std::min({ s0.y, s1.y, s2.y })));
    float maxY = std::min((float)HEIGHT - 1, ceil(std::max({ s0.y, s1.y, s2.y })));

    for (int y = (int)minY; y <= (int)maxY; ++y) {
        for (int x = (int)minX; x <= (int)maxX; ++x) {
            vec2 p(x + 0.5f, y + 0.5f);

            vec2 e0 = s1 - s0, e1 = s2 - s1, e2 = s0 - s2;
            vec2 vp0 = p - s0, vp1 = p - s1, vp2 = p - s2;

            float a = e0.x * vp0.y - e0.y * vp0.x;
            float b = e1.x * vp1.y - e1.y * vp1.x;
            float c = e2.x * vp2.y - e2.y * vp2.x;

            if ((a >= 0 && b >= 0 && c >= 0) || (a <= 0 && b <= 0 && c <= 0)) {
                float area = std::abs((s1.x - s0.x) * (s2.y - s0.y) - (s2.x - s0.x) * (s1.y - s0.y));
                float w0 = std::abs((s1.x - p.x) * (s2.y - p.y) - (s2.x - p.x) * (s1.y - p.y)) / area;
                float w1 = std::abs((s2.x - p.x) * (s0.y - p.y) - (s0.x - p.x) * (s2.y - p.y)) / area;
                float w2 = 1.0f - w0 - w1;

                float depth = w0 * p0.z + w1 * p1.z + w2 * p2.z;
                int idx = y * WIDTH + x;

                if (depth < DepthBuffer[idx]) {
                    DepthBuffer[idx] = depth;
                    vec3 color = w0 * c0 + w1 * c1 + w2 * c2;
                    OutputImage[3 * idx + 0] = color.r;
                    OutputImage[3 * idx + 1] = color.g;
                    OutputImage[3 * idx + 2] = color.b;
                }
            }
        }
    }
}


void render_scene() {
    create_scene();

    mat4 model = glm::scale(mat4(1.0f), vec3(2.0f));
    model = glm::translate(model, vec3(0, 0, -7));

    mat4 view = lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, 1.0f, 0.0f));
    mat4 persp = glm::frustum(-0.1f, 0.1f, -0.1f, 0.1f, -0.1f, -1000.0f); 
    mat4 ortho = glm::scale(mat4(1.0f), vec3(1, 1, -1));
    mat4 MVP = ortho * persp * view * model;

    std::vector<vec3> VertexColors(gVertices.size());
    for (int i = 0; i < gVertices.size(); ++i) {
        vec3 pos = vec3(model * vec4(gVertices[i], 1.0f)); // 모델 좌표로 변환
        vec3 normal = normalize(mat3(transpose(inverse(model))) * gVertices[i]); // 노멀 변환
        VertexColors[i] = compute_phong_lighting(pos, normal);
    }

    for (int i = 0; i < gIndexBuffer.size(); i += 3) {
        int ia = gIndexBuffer[i];
        int ib = gIndexBuffer[i + 1];
        int ic = gIndexBuffer[i + 2];

        vec4 v0 = MVP * vec4(gVertices[ia], 1.0f);
        vec4 v1 = MVP * vec4(gVertices[ib], 1.0f);
        vec4 v2 = MVP * vec4(gVertices[ic], 1.0f);

        rasterize_triangle_gouraud(v0, v1, v2,
            VertexColors[ia], VertexColors[ib], VertexColors[ic]);
    }
}


int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Gouraud Shading", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    render_scene();

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(WIDTH, HEIGHT, GL_RGB, GL_FLOAT, &OutputImage[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}