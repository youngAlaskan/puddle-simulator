#include "application.h"

#include <thread>

#include "generators/terrainGenerator.h"

// #include "glErrors.h"

// Runs the main application loop
void Application::Run()
{
	// Link VAO pointers
	m_Renderer->SetVAOs(m_Scene->m_VAOs);

	// Create Terrain
	auto vertices = TerrainGenerator::GenerateVertices();
	m_Scene->SetTerrain(vertices);
	auto positions = std::vector<glm::vec3>();
	for (const auto& vertex : vertices)
	{
		positions.push_back(vertex.Position);
	}
	m_Simulator->SetTerrain(positions);

	// Create Terrain Shader
	std::shared_ptr<ShaderProgram> terrainShader = m_Renderer->AddShaderProgram(
		std::vector<std::pair<GLenum, const char*>>
		{
			{ GL_VERTEX_SHADER, "src\\shaders\\vertex\\terrain.vert" },
			{ GL_FRAGMENT_SHADER, "src\\shaders\\fragment\\terrain.frag" }
		}
	);

	m_Scene->CreateDroplets(0.1f);
	std::vector<glm::vec3> centers = std::vector<glm::vec3>();
	for (const auto& droplet : m_Scene->m_Droplets->GetDroplets())
	{
		centers.push_back(droplet.GetPosition());
	}
	m_Simulator->RegisterParticles(centers, 0.1f);

	// Create Droplets shader
	std::shared_ptr<ShaderProgram> dropletShader = m_Renderer->AddShaderProgram(
		std::vector<std::pair<GLenum, const char*>>
		{
			{ GL_VERTEX_SHADER, "src\\shaders\\vertex\\droplets.vert" },
			{ GL_FRAGMENT_SHADER, "src\\shaders\\fragment\\droplets.frag" }
		}
	);

	// Register Shader ID with VAO ID
	m_Renderer->RegisterVAOShaderMatch(m_Scene->m_Terrain->m_VAO->GetID(), terrainShader->GetID());
	m_Renderer->RegisterVAOShaderMatch(m_Scene->m_Droplets->GetVAO()->GetID(), dropletShader->GetID());

	m_Renderer->RegisterUniformBuffer(m_Scene->m_Camera->m_ViewProjMatrices);

	// Main loop
	while (WindowIsOpen())
	{
		OnFrameStart();

		m_Simulator->Step();

		m_Scene->OnUpdate();

		m_Renderer->Render();

		OnFrameEnd();
	}
}

// Initializes GLFW, glad, and ImGui
void Application::Init()
{
	// Initialize GLFW to use OpenGL 3.3
	// ---------------------------------
	glfwInit();
#ifdef OPENGL_DEBUGGING
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#else // OPENGL_BEBUGGING
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif // OPENGL_BEBUGGING
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

	// Register mouse callback
	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(m_Window, MouseCallback);

	// Register scroll callback
	glfwSetScrollCallback(m_Window, ScrollCallback);

	// Initialize GLAD
	// ---------------
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		CleanUp();
		PrintAndThrowException(GLADLoaderException());
	}

	// Set OpenGL flags and variables
#ifdef OPENGL_DEBUGGING
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MessageCallback, nullptr);
#endif // OPENGL_DEBUGGING

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	// Initialize ImGui
	// ----------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	m_Renderer = std::make_unique<Renderer>();
	m_Scene = std::make_unique<Scene>();
	m_Simulator = std::make_unique<Simulator>(100U, 100U, 100U);

	m_Scene->m_Camera->m_AspectRatio = static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight);
	m_Scene->m_Camera->m_ViewProjMatrices.SetEmptyBuffer(2 * sizeof(glm::mat4));
	g_ActiveCamera = m_Scene->m_Camera;

	m_Simulator->SetDeltaTime(1.0f);
}

// Sets up start of new frame
void Application::OnFrameStart()
{
	processInput(m_Window);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	SetImGuiWindows();
}

// Renders ImGui elements, swaps glfw buffers, and polls glfw events
void Application::OnFrameEnd()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(m_Window);
	glfwPollEvents();
}

void Application::SetImGuiWindows() const
{
	ImGui::Begin("Droplet Spawner");

	if (ImGui::DragFloat("Particle Radius", &g_ParticleRadius, 0.01f, 0.0f, 5.0f))
	{
		m_Scene->m_Droplets->UpdateVertexVBO(g_ParticleRadius);

		m_Scene->m_Droplets->ClearDroplets();
		m_Scene->m_Droplets->UpdateInstanceVBO();
		m_Simulator->ClearParticles();
		m_Simulator->ClearParticleGrid();
	}

	static int count = 1;

	if (ImGui::InputInt("Droplet Count", &count))
	{
		if (count < 0)
			count = 0;
	}

	ImGui::SameLine();

	if (ImGui::Button("Spawn Droplet"))
	{
		std::vector<glm::vec3> centers = std::vector<glm::vec3>(count);
		std::vector<Droplet> droplets = std::vector<Droplet>(count);
		for (int i = 0; i < count; i++)
		{
			float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (100.0f))) - 50.0f;
			float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (5.0f))) + 25.0f;
			float z = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (100.0f))) - 50.0f;
			centers.emplace_back(x, y, z);
			droplets.emplace_back(glm::vec3(x, y, z));
		}
		m_Simulator->RegisterParticles(centers, g_ParticleRadius);
		m_Scene->m_Droplets->AddDroplets(droplets);
		m_Scene->m_Droplets->UpdateInstanceVBO();
	}

	ImGui::End();
}

void Application::Render()
{
	while (WindowIsOpen())
	{
		OnFrameStart();

		m_Scene->OnUpdate();

		m_Renderer->Render();

		OnFrameEnd();
	}
}

void Application::Simulate()
{
	while (WindowIsOpen())
	{
		m_Simulator->Step();
	}
}
