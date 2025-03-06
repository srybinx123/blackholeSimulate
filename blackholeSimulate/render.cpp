#include "render.h"
#include "shader.h"

#include <iostream>

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

GLuint createColorTexture(int width, int height, bool hdr) {
    GLuint colorTexture;
    glGenTextures(1, &colorTexture);

    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, hdr ? GL_RGB16F : GL_RGB, width, height, 0,
        GL_RGB, hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return colorTexture;
}

GLuint CreateQuadVAO() {
    std::vector<glm::vec3> vertices;

    vertices.push_back(glm::vec3(-1, -1, 0));
    vertices.push_back(glm::vec3(-1, 1, 0));
    vertices.push_back(glm::vec3(1, 1, 0));

    vertices.push_back(glm::vec3(1, 1, 0));
    vertices.push_back(glm::vec3(1, -1, 0));
    vertices.push_back(glm::vec3(-1, -1, 0));

    // Create VBO
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
        &vertices[0], GL_STATIC_DRAW);

    // 1st attribute buffer: positions
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0,        // attribute
        3,        // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0,        // stride
        (void*)0 // array buffer offset
    );

    glBindVertexArray(0);

    return vao;
}

GLuint CreateFramebuffer(const FramebufferCreateInfo& info)
{
    GLuint framebuffer;

    // 创建帧缓冲区
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // 创建颜色附件
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
        info.TextureID, 0);

    if (info.UseDepth) {
        GLuint rbo;
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, info.width,
            info.height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, rbo);
    }

    // 检查帧缓冲区完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return framebuffer;
}

bool bindToTextureUnit(GLuint program, const std::string& name, GLenum type, GLuint textureID, int textureUnitID)
{
    GLint loc = glGetUniformLocation(program, name.c_str());
    if (loc != -1) {
        glUniform1i(loc, textureUnitID);

        // Set up the texture units.
        glActiveTexture(GL_TEXTURE0 + textureUnitID);
        glBindTexture(type, textureID);
        return true;
    }
    else {
        std::cout << "WARNING: uniform " << name << " is not found in shader"
            << std::endl;
        return false;
    }
}

void RendertoTexture(const RendertoTextureInfo& info)
{
    static std::map<GLuint, GLuint> textureFramebufferMap;
    GLuint targetFramebuffer;
    if (!textureFramebufferMap.count(info.targetTexture)) {
        FramebufferCreateInfo createInfo;
        createInfo.TextureID = info.targetTexture;
        targetFramebuffer = CreateFramebuffer(createInfo);
        textureFramebufferMap[info.targetTexture] = targetFramebuffer;
    }
    else {
        targetFramebuffer = textureFramebufferMap[info.targetTexture];
    }

    static std::map<std::string, GLuint> shaderProgramMap;
    GLuint program;
    if (!shaderProgramMap.count(info.fragShader)) {
        program = createShader(info.vertexShader, info.fragShader);
        shaderProgramMap[info.fragShader] = program;
    }
    else {
        program = shaderProgramMap[info.fragShader];
    }

    // Rendering a quad.
    {
        glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);

        glViewport(0, 0, info.width, info.height);

        glDisable(GL_DEPTH_TEST);

        glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);

        // Set up the uniforms.
        {
            glUniform2f(glGetUniformLocation(program, "resolution"),
                (float)info.width, (float)info.height);

            glUniform1f(glGetUniformLocation(program, "time"), (float)glfwGetTime());

            // Update float uniforms
            for (auto const& [name, val] : info.floatUniforms) {
                GLint loc = glGetUniformLocation(program, name.c_str());
                if (loc != -1) {
                    glUniform1f(loc, val);
                }
                else {
                    std::cout << "WARNING: uniform " << name << " is not found"
                        << std::endl;
                }
            }

            // Update texture uniforms
            int textureUnit = 0;
            for (auto const& [name, texture] : info.textureUniforms) {
                bindToTextureUnit(program, name, GL_TEXTURE_2D, texture, textureUnit++);
            }
            for (auto const& [name, texture] : info.cubemapUniforms) {
                bindToTextureUnit(program, name, GL_TEXTURE_CUBE_MAP, texture,
                    textureUnit++);
            }
        }

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(0);
    }
}