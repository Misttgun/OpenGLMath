#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Renderer.h"
#include "Polygon.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/glm.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

#include "tests/TestPolygon.h"
#include <cstdint>

// Variables globales -> Rendre ça propre si on a le temps...
bool polygonCreation;
bool fenetreCreation;
glm::mat4 proj, view;
const int WIDTH = 1024;
const int HEIGHT = 768;
std::unique_ptr<Polygon> polygon;
std::unique_ptr<Polygon> fenetre;


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

int main(void)
{
	// Initialisation des variables globales
	polygonCreation = false;
	fenetreCreation = false;
	proj = glm::ortho(0.0f, static_cast<float>(WIDTH), static_cast<float>(HEIGHT), 0.0f, -1.0f, 1.0f);
	view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
	auto vp = view * proj;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	/* Create a windowed mode window and its OpenGL context */
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Hello World", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	if (glewInit() != GLEW_OK)
		std::cout << "Error!" << std::endl;

	std::cout << glGetString(GL_VERSION) << std::endl;

	GL_CALL(glEnable(GL_BLEND));
	GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// Creation du renderer et du shader
	Renderer renderer;
	std::unique_ptr<Shader> shader = std::make_unique<Shader>("res/shaders/Basic.shader");
	shader->bind();
	shader->setUniform4F("u_Color", 1.0f, 1.0f, 1.0f, 1.0f);

	// Initialisation de la fenetre et du polygon
	fenetre = std::make_unique<Polygon>(0.0f, 0.0f, 1.0f);
	polygon = std::make_unique<Polygon>(1.0f, 0.0f, 0.0f);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	// Initialisation de IMGUI
	ImGui::CreateContext();
	ImGui_ImplGlfwGL3_Init(window, true);
	ImGui::StyleColorsDark();

	//Boucle de rendu
	while (!glfwWindowShouldClose(window))
	{
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetKeyCallback(window, key_callback);
		GL_CALL(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		renderer.clear();

		polygon->onRender(vp, shader.get());
		fenetre->onRender(vp, shader.get());

		// Creation du menu IMGUI
		ImGui_ImplGlfwGL3_NewFrame();
		ImGui::Begin("Menu");
		if (ImGui::CollapsingHeader("Instructions"))
		{
			ImGui::BulletText("E pour la creation du polygone.");
			ImGui::BulletText("F pour la creation de la fenetre.");
		}

		//TODO Trouver une autre solution pour afficher les sections
		if (ImGui::CollapsingHeader("Fenetre"))
		{
			fenetre->onImGuiRender();
		}

		if (ImGui::CollapsingHeader("Polygon"))
		{
			polygon->onImGuiRender();
		}
		ImGui::End();
		/*if (currentTest)
		{
			currentTest->OnUpdate(0.0f);
			currentTest->OnRender();
			ImGui::Begin("Test");
			if (currentTest != testMenu && ImGui::Button("<-"))
			{
				delete currentTest;
				currentTest = testMenu;
			}
			currentTest->OnImGuiRender();
			ImGui::End();
		}*/

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}


// Call back souris
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && polygonCreation)
	{
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(window, &xpos, &ypos);
		polygon->addPoint(xpos, ypos);
		//std::cout << "Cursor Position at (" << xpos << " : " << ypos << ")" << std::endl;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && fenetreCreation)
	{
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(window, &xpos, &ypos);
		fenetre->addPoint(xpos, ypos);
		//std::cout << "Cursor Position at (" << xpos << " : " << ypos << ")" << std::endl;
	}
}


// Callback clavier
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
		polygonCreation = true;
	else if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		polygonCreation = false;
	if (key == GLFW_KEY_F && action == GLFW_PRESS)
		fenetreCreation = true;
	else if (key == GLFW_KEY_F && action == GLFW_RELEASE)
		fenetreCreation = false;
}
