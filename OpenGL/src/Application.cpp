#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include "Renderer.h"
#include "PolygonManager.h"

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

        PolygonManager::get()->sutherland_ogdmann();
        PolygonManager::get()->on_render(vp, shader.get());
        PolygonManager::get()->compute_bounding_box();
        PolygonManager::get()->on_render_fill(vp, shader.get());
        
		// Creation du menu IMGUI
		ImGui_ImplGlfwGL3_NewFrame();
		ImGui::Begin("Menu");
		ImGui::Text("Instructions:");
		ImGui::BeginChild("Instructions", ImVec2(0, 50), true);
		ImGui::BulletText("E pour la creation du polygone.");
		ImGui::BulletText("F pour la creation de la fenetre.");
		ImGui::EndChild();
		ImGui::Text("Polygon:");
		ImGui::BeginChild("Polygon", ImVec2(0, 150), true);
        PolygonManager::get()->on_im_gui_render_polygons();
		ImGui::EndChild();
		ImGui::Text("Fenetre:");
		ImGui::BeginChild("Fenetre", ImVec2(0, 150), true);
        PolygonManager::get()->on_im_gui_render_windows();
		ImGui::EndChild();
		ImGui::End();


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
		glfwGetCursorPos(window, &xpos, &ypos);
        PolygonManager::get()->get_current_polygon()->addPoint(xpos, ypos);
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && fenetreCreation)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
        PolygonManager::get()->get_current_window()->addPoint(xpos, ypos);
	}
}


// Callback clavier
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
    {
        std::cout << "PRESS E\n";
        polygonCreation = true;
        PolygonManager::get()->add_polygon();
    }
     
    else if (key == GLFW_KEY_E && action == GLFW_RELEASE)
    {
        std::cout << "RELEASE E\n";
        polygonCreation = false;

        std::cout << "BUT QHY ?\n";

        if (PolygonManager::get()->get_current_polygon() != nullptr)
            

        if (PolygonManager::get()->get_current_polygon() != nullptr)
            if (PolygonManager::get()->get_current_polygon()->size() < 3)
                PolygonManager::get()->delete_current_polygon();

        std::cout << "DIE ?\n";
    }
	
    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        std::cout << "PRESS F\n";
        fenetreCreation = true;
        PolygonManager::get()->add_window();
    }
    
    else if (key == GLFW_KEY_F && action == GLFW_RELEASE)
    {
        std::cout << "RELEASE F\n";
        fenetreCreation = false;

        if (PolygonManager::get()->get_current_window()->size() < 3)
            PolygonManager::get()->delete_current_window();

        if (PolygonManager::get()->get_current_window())
            PolygonManager::get()->get_current_window()->ear_clipping(PolygonManager::get()->get_triangles());
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
    {
        if (PolygonManager::get()->get_current_polygon() != nullptr)
            PolygonManager::get()->get_current_polygon()->subdivise();
    }
}
