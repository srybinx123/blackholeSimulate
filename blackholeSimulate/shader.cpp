#include "shader.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <GL/glew.h>

std::string loadFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::in);
	if (!file.is_open())
	{
		throw std::runtime_error("Failed to open file: " + filename);
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();
	return buffer.str();
}

GLuint compileShader(const std::string& source, GLenum type)
{
	// 创建着色器对象
	GLuint shader = glCreateShader(type);

	// 设置着色器源代码
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	// 编译着色器
	glCompileShader(shader);

	// 检查编译是否成功
	GLint success = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		// 获取编译错误日志的长度
		GLint length = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		// 分配内存存储错误日志
		std::vector<char> log(length);
		// 获取编译错误日志
		glGetShaderInfoLog(shader, length, &length, log.data());
		// 输出错误日志到标准错误流
		std::cerr << "Shader compilation failed: " << log.data() << std::endl;
		// 删除着色器对象
		glDeleteShader(shader);
		// 抛出异常表示编译失败
		throw std::runtime_error("Failed to compile the shader.");
	}
	// 返回编译成功的着色器对象
	return shader;
}

GLuint createShader(const std::string& vertCode, const std::string& fragCode)
{
	std::cout << "compiling " << vertCode << std::endl;
	GLuint vertShader = compileShader(loadFile(vertCode), GL_VERTEX_SHADER);
	std::cout << "compiling " << fragCode << std::endl;
	GLuint fragShader = compileShader(loadFile(fragCode), GL_FRAGMENT_SHADER);
	// 创建程序对象
	GLuint program = glCreateProgram();
	// 附加着色器对象到程序对象
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	// 链接程序对象
	glLinkProgram(program);
	// 检查链接是否成功
	GLint success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		// 获取链接错误日志的长度
		GLint length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		// 分配内存存储错误日志
		std::vector<char> log(length);
		// 获取链接错误日志
		glGetProgramInfoLog(program, length, &length, log.data());
		// 输出错误日志到标准错误流
		std::cerr << "Program linking failed: " << log.data() << std::endl;
		// 删除程序对象
		glDeleteProgram(program);
		// 抛出异常表示链接失败
		throw std::runtime_error("Failed to link the program.");
	}
	glDetachShader(program, vertShader);
	glDetachShader(program, fragShader);
	glDeleteShader(vertShader);
	glDeleteShader(fragShader);

	return program;
}