#include "texture.h"

#include <GL/glew.h>
#include <iostream>
#include <vector>

#include <stb_image.h>

GLuint loadTexture2D(const std::string& path)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height, comp;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &comp, 0);
	if (!data)
	{
		std::cout << "Failed to load texture: " << path << std::endl;
		stbi_image_free(data);
		return 0;
	}
	GLenum format = GL_SRGB_ALPHA;
	GLenum internalFormat = GL_SRGB_ALPHA;
	if (comp == 1)
	{
		format = GL_RED;
		internalFormat = GL_RED;
	}
	else if (comp == 3)
	{
		format = GL_RGB;
		internalFormat = GL_SRGB;
	}
	else if (comp == 4)
	{
		format = GL_RGBA;
		internalFormat = GL_SRGB_ALPHA;
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	if (comp == 1) {
		format = GL_RED;
		internalFormat = GL_RED;
	}
	else if (comp == 3) {
		format = GL_RGB;
		internalFormat = GL_SRGB;
	}
	else if (comp == 4) {
		format = GL_RGBA;
		internalFormat = GL_SRGB_ALPHA;
	}

	return textureID;
}

GLuint loadTextureCubeMap(const std::string& cubemapDir)
{
	const std::vector<std::string> faces = { "right", "left", "top", "bottom", "front", "back" };
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, comp;
	for (int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load((cubemapDir + "/" + faces[i] + ".png").c_str(), &width, &height, &comp, 0);

		if (!data)
		{
			std::cout << "Failed to load texture: " << cubemapDir + "/" + faces[i] + ".png" << std::endl;
			stbi_image_free(data);
			return 0;
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}