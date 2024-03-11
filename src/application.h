#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad\glad.h>
#include <GLFW\glfw3.h>

#include <exception>
#include <iostream>

#include "rendering/renderer.h"
#include "scene/scene.h"

// Handles all GLFW and glad window management
class Application
{
public:
	Application() { Init(); }
	Application(uint32_t width, uint32_t height)
		: m_WindowWidth(width), m_WindowHeight(height) { Init(); }
	~Application() { CleanUp(); }

	void Run();

public:
	GLFWwindow* m_Window = nullptr;
	uint32_t m_WindowWidth = 400;
	uint32_t m_WindowHeight = 400;
	Renderer m_Renderer = {};
	Scene m_Scene = {};

private:
	void Init();

	void OnFrameStart();

	void OnFrameEnd();

	bool WindowIsOpen();

	void CleanUp() { glfwTerminate(); }
};

// Application Callbacks:

// Sets the size of the OpenGL Viewport
inline void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// Exceptions

inline void PrintAndThrowException(const std::exception& e)
{
	std::cerr << e.what() << std::endl;
	throw e;
}

// The function glfwCreateWindow failed (returned nullptr)
class GLFWWindowCreationException : public std::exception {
public:
	char* what()
	{
		return "Failed to create GLFW window";
	}
};

// The function gladLoadGLLoader failed (returned 0)
class GLADLoaderException : public std::exception {
public:
	char* what()
	{
		return "Failed to initialize GLAD";
	}
};
