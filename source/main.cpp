#include "litewq/mesh/TriMesh.h"
#include "litewq/utils/logging.h"
#include "litewq/platform/OpenGL/GLShader.h"
#include "litewq/camera/camera.h"
#include "litewq/utils/Loader.h"
#include "litewq/scent/scent.h"
#include "litewq/mesh/SkyBoxMesh.h"
#include "litewq/mesh/SkyBoxTexture.h"
#include "litewq/camera/Scene.h"
#include "litewq/math/BoundingBox.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "stb/stb_image.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <cmath>
#include <random>

using namespace litewq;

const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

int window_width = SCR_WIDTH;
int window_height = SCR_HEIGHT;

int current_width = SCR_WIDTH;
int current_height = SCR_HEIGHT;

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
	window_width = width;
	window_height = height;
}

Camera camera(glm::vec3(0.0f, 1.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
float deltaTime = 0.0f;
float lastFrame = 0.0f;
Scene scene;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, true);
		if (key == GLFW_KEY_F)
			camera.toggle_fly_mode();
		if (key == GLFW_KEY_F11 || (key == GLFW_KEY_ENTER && mods == GLFW_MOD_ALT))
		{
			static bool fullscreen = false;
			if (fullscreen)
			{
				glfwSetWindowMonitor(window, nullptr, 0, 0, window_width, window_height, 0);
				fullscreen = false;
				current_width = window_width;
				current_height = window_height;
			}
			else
			{
				GLFWmonitor *monitor = glfwGetPrimaryMonitor();
				const GLFWvidmode *mode = glfwGetVideoMode(monitor);
				glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
				fullscreen = true;
				current_width = mode->width;
				current_height = mode->height;
			}
		}
	}
}

void processInput(GLFWwindow *window)
{
    glm::vec3 old_pos = camera.get_position();
	int k = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) ? 2 : 1;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.process_keyboard(FORWARD, deltaTime * k);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.process_keyboard(BACKWARD, deltaTime * k);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.process_keyboard(LEFT, deltaTime * k);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.process_keyboard(RIGHT, deltaTime * k);

    glm::vec3 new_pos = camera.get_position();
    new_pos.y = 0.0f;
    Bounds3 bbox(old_pos, new_pos);
    if (scene.collision(bbox)) {
        camera.set_pos(old_pos);
    }
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	static bool firstMouse = true;
	static float lastX = SCR_WIDTH / 2.0f;
	static float lastY = SCR_HEIGHT / 2.0f;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.process_mouse_movement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
	camera.process_mouse_scroll(yoffset);
}

std::vector<unsigned int> VBOs;

unsigned int loadVAO(const float vertices[], int size)
{
	// Create vertex buffer
	unsigned int VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	// Set vertex attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
	glEnableVertexAttribArray(2); // texture coord

	VBOs.push_back(VBO);
	return VAO;
}

unsigned int loadTexture(const char *path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		LOG(FATAL) << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}



int main(int argc, char *argv[])
{
	// Initialize glfw
	if (!glfwInit())
	{
		LOG(FATAL) << "Failed to initialize glfw" << std::endl;
		return -1;
	}

	// Create window
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		LOG(FATAL) << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Set mouse callback
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Initialize glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		LOG(FATAL) << "Failed to initialize glad" << std::endl;
		return -1;
	}

	// Create shader
	GLShader shader(
		Loader::readFromRelative("shader/simple/vertex.glsl"), 
		Loader::readFromRelative("shader/simple/opaque_frag.glsl")
	);

    GLShader scent_shader(
        Loader::readFromRelative("shader/simple/vertex.glsl"),
        Loader::readFromRelative("shader/simple/scent_frag.glsl")
    );

	GLShader phong(
		Loader::readFromRelative("shader/bling-phong/lighting_map_vertex.glsl"), 
		Loader::readFromRelative("shader/bling-phong/lighting_map_frag.glsl")
	);

    GLShader shadow(
        Loader::readFromRelative("shader/shadow/shadowmap_vertex.glsl"),
        Loader::readFromRelative("shader/shadow/shadowmap_frag.glsl")
    );

    GLShader skybox_shader(
        Loader::readFromRelative("shader/skybox/vertex.glsl"),
        Loader::readFromRelative("shader/skybox/frag.glsl")
    );

    GLShader depth_shader(
        Loader::readFromRelative("shader/shadow/depth_vertex.glsl"),
        Loader::readFromRelative("shader/shadow/depth_frag.glsl")
    );

    GLShader debug_depth(
        Loader::readFromRelative("shader/shadow/depth_debug_vertex.glsl"),
        Loader::readFromRelative("shader/shadow/depth_debug_frag.glsl")
    );


	// another square
	float vertices2[] = {
		// positions        // normals      // texture coords
		0.5f, 0.0f, 0.5f,  0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		-0.5f, 0.0f, 0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

	unsigned int VAO2 = loadVAO(vertices2, sizeof(vertices2));

	// Create texture
	unsigned int texContainer = loadTexture("assets/tex/container.jpg");
	unsigned int texGrass = loadTexture("assets/tex/grass.jpg");

	// Choose texture unit
	shader.Bind();
	shader.updateUniformInt("texture1", 0);

	// Create scent
	std::default_random_engine generator(time(NULL));
    Scent scent(generator, scent_shader, 0.1);
    scent.initGL();

    auto wolf =
            TriMesh::from_obj(Loader::getAssetPath("model/wolf/wolf.obj"));
    auto wolf_raw = static_cast<TriMesh *>(wolf.get());
    wolf_raw->initGL();
    wolf_raw->updateModel(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.5f)));
    wolf_raw->buildBVH();

    auto tree =
            TriMesh::from_obj(Loader::getAssetPath("model/tree/Tree1.obj"));
    auto tree_raw = static_cast<TriMesh *>(tree.get());
    tree_raw->initGL();
    tree_raw->updateModel(glm::scale(glm::mat4(1.0f), glm::vec3(.5f, .5f, .5f)));
    // tree_raw->buildBVH();

    /* Skybox forest */
    auto skybox = litewq::SkyBoxMesh::build();
    litewq::SkyBoxTexture skybox_tex;
//     skybox_tex.LoadTexture({
//             litewq::Loader::getAssetPath("tex/Plants/posx.jpg"),
//             litewq::Loader::getAssetPath("tex/Plants/negx.jpg"),
//             litewq::Loader::getAssetPath("tex/Plants/posy.jpg"),
//             litewq::Loader::getAssetPath("tex/Plants/negy.jpg"),
//             litewq::Loader::getAssetPath("tex/Plants/posz.jpg"),
//             litewq::Loader::getAssetPath("tex/Plants/negz.jpg"),
//     });
//     skybox_tex.LoadTexture({
//         litewq::Loader::getAssetPath("tex/skybox_demo/right.jpg"),
//         litewq::Loader::getAssetPath("tex/skybox_demo/left.jpg"),
//         litewq::Loader::getAssetPath("tex/skybox_demo/top.jpg"),
//         litewq::Loader::getAssetPath("tex/skybox_demo/bottom.jpg"),
//         litewq::Loader::getAssetPath("tex/skybox_demo/front.jpg"),
//         litewq::Loader::getAssetPath("tex/skybox_demo/back.jpg"),
//     });
//
//     skybox->initGL();

    scene.addObject(tree_raw);
    scene.addObject(wolf_raw);

    /* Shadow mapping, depth frame buffer and stored in texture. */
    unsigned int DepthMapFBO, DepthMap;
    constexpr unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    glGenFramebuffers(1, &DepthMapFBO);

    glGenTextures(1, &DepthMap);
    glBindTexture(GL_TEXTURE_2D, DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    /* if you choose repeat wrapping method, shadow will repeat outside shadow port*/
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glm::vec4 BorderColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(BorderColor));
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // configure shader
    shadow.Bind();
    shadow.updateUniformInt("shadowMap", 1);

    // Render loop
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!glfwWindowShouldClose(window))
	{
		// Input
		processInput(window);

		// Calculate delta time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Setup
        glm::vec3 light_pos = glm::vec3(0.f, 4.f, 3.f);
        glm::mat4 light_projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1.0f, 7.5f);
        glm::mat4 light_view = glm::lookAt(light_pos, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 world2light = light_projection * light_view;

        depth_shader.Bind();
        depth_shader.updateUniformMat4("lightSpaceMatrix", world2light);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::mat4(1.0f);

		// Draw 21x21 tiles around camera, manually assign material
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texGrass);
		glBindVertexArray(VAO2);
		glm::vec3 cameraPos = camera.get_position();
		for (int i = -10; i <= 10; i++)
			for (int j = -10; j <= 10; j++)
			{
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3(i * 1.0f + floor(cameraPos.x), 0.0f, j * 1.0f + floor(cameraPos.z)));
				depth_shader.updateUniformMat4("model", model);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}

        /* always put wolf in front of camera with some distance */
//        glm::vec3 view_dir = camera.get_view_dir();
//        glm::vec3 init_view_dir = glm::vec3(0.f, 0.f, -1.f);
//        glm::vec3 ground_up_vec = glm::vec3(0.f, 1.f, 0.f);
//        glm::vec3 init_ground_up_vec = glm::vec3(0.f, 1.f, 0.f);
//
//        glm::vec3 head_dir = glm::cross(glm::cross(ground_up_vec, view_dir), ground_up_vec);
//        glm::vec3 wolf_pos = camera.get_position() + head_dir * 0.1f;
//
//        glm::mat4 wolf_model2world = glm::lookAt(wolf_pos, wolf_pos + head_dir, ground_up_vec);

        scene.render();


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width, window_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* skybox */
//        skybox_shader.Bind();
//        skybox_shader.updateUniformMat4("view", glm::mat4(glm::mat3(view)));
//        skybox_shader.updateUniformMat4("projection", projection);
//        skybox_tex.BindTexture();
//        skybox->render();

        // Draw scent
//        depth_shader.Bind();
//        scent.render(cameraPos, view, projection, glm::vec4(0, 0, current_width, current_height));

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, DepthMap);
        glm::mat4 view = camera.get_view_matrix();
        glm::mat4 projection = glm::perspective(camera.get_zoom(), (float)window_width / (float)window_height, 0.1f, 100.0f);
        shadow.Bind();
        shadow.updateUniformFloat3("light.pos", light_pos);
        shadow.updateUniformFloat3("light.Ia", glm::vec3(0.3f, 0.3f, 0.3f));
        shadow.updateUniformFloat3("light.Id", glm::vec3(1.0f, 1.0f, 1.0f));
        shadow.updateUniformFloat3("light.Is", glm::vec3(0.2f, 0.2f, 0.2f));
        shadow.updateUniformMat4("lightSpaceMatrix", world2light);
        shadow.updateUniformFloat3("view_pos", camera.get_position());
        shadow.updateUniformMat4("view", view);
        shadow.updateUniformMat4("projection", projection);


        // Draw 21x21 tiles around camera, manually assign material
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texGrass);
        glBindVertexArray(VAO2);
        shadow.updateUniformFloat3("material.Ks", glm::vec3(.5f, .5f, .5f));
        shadow.updateUniformFloat("material.highlight_decay", 200.f);
        cameraPos = camera.get_position();
        for (int i = -10; i <= 10; i++)
            for (int j = -10; j <= 10; j++)
            {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i * 1.0f + floor(cameraPos.x), 0.0f, j * 1.0f + floor(cameraPos.z)));
                shadow.updateUniformMat4("model", model);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }
        scene.render();

        /* render depth */
//        debug_depth.Bind();
//        debug_depth.updateUniformFloat("near_plane", 1.0f);
//        debug_depth.updateUniformFloat("far_plane", 7.5f);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, DepthMap);
//        /*  render depth information in [-1,1] NDC */
//        renderQuad();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Clean up
	glDeleteVertexArrays(1, &VAO2);
	glDeleteBuffers(2, VBOs.data());
	glDeleteTextures(1, &texContainer);
	glDeleteTextures(1, &texGrass);
	glfwTerminate();
	return 0;
}
