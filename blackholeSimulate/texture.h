#pragma once
#include<GL/glew.h>
#include<string>

GLuint loadTexture2D(const std::string& path);

GLuint loadTextureCubeMap(const std::string& cubemapDir);