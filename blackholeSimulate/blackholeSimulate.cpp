// blackholeSimulate.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>
#include <vector>
#include <map>
#include <conio.h>
#include <windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <imgui.h>

#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "render.h"
#include "shader.h"
#include "texture.h"
#include "GLDebugMessageCallback.h"

static int SCR_WIDTH = GetSystemMetrics(SM_CXSCREEN);
static int SCR_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

static float mouseX = 0.0f, mouseY = 0.0f;
static float fovScale = 1.0f;

#define IMGUI_TOGGLE(NAME, DEFAULT)                                            \
  static bool NAME = DEFAULT;                                                  \
  ImGui::Checkbox(#NAME, &NAME);                                               \
  rtti.floatUniforms[#NAME] = NAME ? 1.0f : 0.0f;

#define IMGUI_SLIDER(NAME, DEFAULT, MIN, MAX)                                  \
  static float NAME = DEFAULT;                                                 \
  ImGui::SliderFloat(#NAME, &NAME, MIN, MAX);                                  \
  rtti.floatUniforms[#NAME] = NAME;

void mouseCallback(GLFWwindow* window, double x, double y)
{
    static float lastX = 400.0f;
    static float lastY = 300.0f;
    static float yaw = 0.0f;
    static float pitch = 0.0f;
    static float firstMouse = true;

    mouseX = (float)x;
    mouseY = (float)y;//原点是左上角，因此实际上摄像机位置应当是(mouseX,-mouseY,z)
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    fovScale -= (float)yoffset * 0.1f;
    if (fovScale < 0.5f) fovScale = 0.5f;
    if (fovScale > 3.0f) fovScale = 3.0f;
}

static void glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

class PostProcessPass
{
private:
    GLuint program;

public:
    PostProcessPass(const std::string& fragShader)
    {
        this->program = createShader("shader/simple.vert", fragShader);

        glUseProgram(this->program);
        glUniform1i(glGetUniformLocation(program, "texture0"), 0);
        glUseProgram(0);
    }

    void render(GLuint inputColorTexture, GLuint destFramebuffer = 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, destFramebuffer);

        glDisable(GL_DEPTH_TEST);

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(this->program);

        glUniform2f(glGetUniformLocation(this->program, "resolution"),
            (float)SCR_WIDTH, (float)SCR_HEIGHT);

        glUniform1f(glGetUniformLocation(this->program, "time"),
            (float)glfwGetTime());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, inputColorTexture);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(0);
    }
};

int main()
{
    DEVMODE devMode = { 0 };
    devMode.dmSize = sizeof(DEVMODE);
    if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode)) {
        SCR_WIDTH = devMode.dmPelsWidth;   // 物理宽度
        SCR_HEIGHT = devMode.dmPelsHeight; // 物理高度
    }
    else {
        std::cerr << "Failed to get display settings...\n";
        return 1;
    }

    std::cout << SCR_WIDTH << " " << SCR_HEIGHT << "\n";
    //初始化error callback
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit())
        return 1;

    //初始化窗口
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    GLFWwindow* window =
        glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Wormhole", NULL, NULL);
    if (window == NULL)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetWindowPos(window, 0, 0);

    bool err = (glewInit() != GLEW_OK);
    if (err) {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    GLuint fboBlackhole, texBlackhole;
    texBlackhole = createColorTexture(SCR_WIDTH, SCR_HEIGHT);

    FramebufferCreateInfo info = {};
    info.TextureID = texBlackhole;
    if (!(fboBlackhole = CreateFramebuffer(info))) {
        assert(false);
    }

    GLuint quadVAO = CreateQuadVAO();
    glBindVertexArray(quadVAO);

    //主循环
    PostProcessPass passthrough("shader/passthrough.frag");

    GLuint galaxy = loadTextureCubeMap("assets/skybox_nebula_dark");
    GLuint colorMap = loadTexture2D("assets/color_map.png");
    GLuint uvChecker = loadTexture2D("assets/uv_checker.png");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowDemoWindow();

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        // renderScene(fboBlackhole);

        static GLuint texBlackhole = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RendertoTextureInfo rtti;
            rtti.fragShader = "shader/blackhole_main.frag";
            rtti.cubemapUniforms["galaxy"] = galaxy;
            rtti.textureUniforms["colorMap"] = colorMap;
            rtti.floatUniforms["mouseX"] = mouseX;
            rtti.floatUniforms["mouseY"] = mouseY;
            rtti.floatUniforms["fovScale"] = fovScale;
            rtti.targetTexture = texBlackhole;
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;

            IMGUI_TOGGLE(gravatationalLensing, true);
            IMGUI_TOGGLE(renderBlackHole, true);
            IMGUI_TOGGLE(mouseControl, true);
            IMGUI_SLIDER(cameraRoll, 0.0f, -180.0f, 180.0f);
            IMGUI_TOGGLE(frontView, false);
            IMGUI_TOGGLE(topView, false);
            IMGUI_TOGGLE(adiskEnabled, true);
            IMGUI_TOGGLE(adiskParticle, true);
            IMGUI_SLIDER(adiskDensityV, 2.0f, 0.0f, 10.0f);
            IMGUI_SLIDER(adiskDensityH, 4.0f, 0.0f, 10.0f);
            IMGUI_SLIDER(adiskHeight, 0.55f, 0.0f, 1.0f);
            IMGUI_SLIDER(adiskLit, 0.25f, 0.0f, 4.0f);
            IMGUI_SLIDER(adiskNoiseLOD, 5.0f, 1.0f, 12.0f);
            IMGUI_SLIDER(adiskNoiseScale, 0.8f, 0.0f, 10.0f);
            IMGUI_SLIDER(adiskSpeed, 0.5f, 0.0f, 1.0f);

            RendertoTexture(rtti);
        }

        static GLuint texBrightness = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RendertoTextureInfo rtti;
            rtti.fragShader = "shader/bloom_brightness_pass.frag";
            rtti.textureUniforms["texture0"] = texBlackhole;
            rtti.targetTexture = texBrightness;
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;
            RendertoTexture(rtti);
        }

        const int MAX_BLOOM_ITER = 8;
        static GLuint texDownsampled[MAX_BLOOM_ITER];
        static GLuint texUpsampled[MAX_BLOOM_ITER];
        if (texDownsampled[0] == 0) {
            for (int i = 0; i < MAX_BLOOM_ITER; i++) {
                texDownsampled[i] =
                    createColorTexture(SCR_WIDTH >> (i + 1), SCR_HEIGHT >> (i + 1));
                texUpsampled[i] = createColorTexture(SCR_WIDTH >> i, SCR_HEIGHT >> i);
            }
        }

        static int bloomIterations = MAX_BLOOM_ITER;
        ImGui::SliderInt("bloomIterations", &bloomIterations, 1, 8);
        for (int level = 0; level < bloomIterations; level++) {
            RendertoTextureInfo rtti;
            rtti.fragShader = "shader/bloom_downsample.frag";
            rtti.textureUniforms["texture0"] =
                level == 0 ? texBrightness : texDownsampled[level - 1];
            rtti.targetTexture = texDownsampled[level];
            rtti.width = SCR_WIDTH >> (level + 1);
            rtti.height = SCR_HEIGHT >> (level + 1);
            RendertoTexture(rtti);
        }

        for (int level = bloomIterations - 1; level >= 0; level--) {
            RendertoTextureInfo rtti;
            rtti.fragShader = "shader/bloom_upsample.frag";
            rtti.textureUniforms["texture0"] = level == bloomIterations - 1
                ? texDownsampled[level]
                : texUpsampled[level + 1];
            rtti.textureUniforms["texture1"] =
                level == 0 ? texBrightness : texDownsampled[level - 1];
            rtti.targetTexture = texUpsampled[level];
            rtti.width = SCR_WIDTH >> level;
            rtti.height = SCR_HEIGHT >> level;
            RendertoTexture(rtti);
        }

        static GLuint texBloomFinal = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RendertoTextureInfo rtti;
            rtti.fragShader = "shader/bloom_composite.frag";
            rtti.textureUniforms["texture0"] = texBlackhole;
            rtti.textureUniforms["texture1"] = texUpsampled[0];
            rtti.targetTexture = texBloomFinal;
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;

            IMGUI_SLIDER(bloomStrength, 0.1f, 0.0f, 1.0f);

            RendertoTexture(rtti);
        }

        static GLuint texTonemapped = createColorTexture(SCR_WIDTH, SCR_HEIGHT);
        {
            RendertoTextureInfo rtti;
            rtti.fragShader = "shader/tonemapping.frag";
            rtti.textureUniforms["texture0"] = texBloomFinal;
            rtti.targetTexture = texTonemapped;
            rtti.width = SCR_WIDTH;
            rtti.height = SCR_HEIGHT;

            IMGUI_TOGGLE(tonemappingEnabled, true);
            IMGUI_SLIDER(gamma, 2.5f, 1.0f, 4.0f);

            RendertoTexture(rtti);
        }

        passthrough.render(texTonemapped);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // // Cleanup
    // ImGui_ImplOpenGL3_Shutdown();
    // ImGui_ImplGlfw_Shutdown();
    // ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
