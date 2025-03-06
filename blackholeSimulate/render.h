#pragma once
#include <map>
#include <string>
#include <vector>
#include <windows.h>

#include <GL/glew.h>

struct FramebufferCreateInfo
{
	GLuint TextureID = 0;
	DWORD width = 256;
	DWORD height = 256;
	bool UseDepth = false;
};

struct RendertoTextureInfo
{
	std::string vertexShader = "shader/simple.vert";
	std::string fragShader;
	std::map<std::string, float> floatUniforms;
	std::map<std::string, GLuint> textureUniforms;
	std::map<std::string, GLuint> cubemapUniforms;
	GLuint targetTexture;
	int width;
	int height;
};

GLuint createColorTexture(int width, int height, bool hdr = true);
GLuint CreateFramebuffer(const FramebufferCreateInfo& info);
GLuint CreateQuadVAO();
bool bindToTextureUnit(GLuint program, const std::string& name, GLenum type, GLuint textureID, int textureUnitID);
void RendertoTexture(const RendertoTextureInfo& info);

