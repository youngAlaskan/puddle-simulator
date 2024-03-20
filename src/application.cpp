#include "application.h"

#include <filesystem>

#include "glErrors.h"

// Runs the main application loop
void Application::Run()
{
	// Link VAO pointers
	m_Renderer->SetVAOs(m_Scene->m_VAOs);

	// Create Terrain
	std::vector<Vertex> vertices = std::vector<Vertex>();

	vertices.emplace_back(glm::vec4(-0.5f, -0.5f, -1.0f, 1.0f), glm::vec3(0.0f), glm::vec2(0.0f));
	vertices.emplace_back(glm::vec4( 0.5f, -0.5f, -1.0f, 1.0f), glm::vec3(0.0f), glm::vec2(0.0f));
	vertices.emplace_back(glm::vec4( 0.0f,  0.5f, -1.0f, 1.0f), glm::vec3(0.0f), glm::vec2(0.0f));

	m_Scene->SetTerrain(vertices);

	// Create Terrain Shader
	ShaderProgram simpleShader = ShaderProgram(
		{
			{ GL_VERTEX_SHADER, "src\\shaders\\vertex\\simple.vert" },
			{ GL_FRAGMENT_SHADER, "src\\shaders\\fragment\\simple.frag" }
		}
	);

	// Register Shader ID with VAO ID
	m_Renderer->RegisterVAOShaderMatch(m_Scene->m_Terrain->m_VAO->GetID(), simpleShader.GetID());

	// Main loop
	while (WindowIsOpen())
	{
		OnFrameStart();

		m_Renderer->Render();

		OnFrameEnd();
	}
}

// Initializes GLFW, glad, and ImGui
void Application::Init()
{
	// Initialize GLFW to use OpenGL 4.6
	// ---------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create Window
	// -------------
	m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, "Overflow Shallow Water Simulation", nullptr, nullptr);
	if (!m_Window)
	{
		CleanUp();
		PrintAndThrowException(GLFWWindowCreationException());
	}

	glfwMakeContextCurrent(m_Window);

	// Register window resizing callback
	glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);

	// Initialize GLAD
	// ---------------
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		CleanUp();
		PrintAndThrowException(GLADLoaderException());
	}

	// Set OpenGL flags and variables
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MessageCallback, nullptr);

	glEnable(GL_CULL_FACE);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Initialize ImGui
	// ----------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	m_Renderer = new Renderer();
	m_Scene = new Scene();
}

// Sets up start of new frame
void Application::OnFrameStart()
{
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

// Renders ImGui elements, swaps glfw buffers, and polls glfw events
void Application::OnFrameEnd()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(m_Window);
	glfwPollEvents();
}

bool Application::WindowIsOpen()
{
	return !glfwWindowShouldClose(m_Window);
}

void Application::CleanUp() const
{
	if (m_Renderer)
		delete m_Renderer;
	if (m_Scene)
		delete m_Scene;

	glfwTerminate();
}
