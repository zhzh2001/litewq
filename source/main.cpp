#include "litewq/mesh/TriMesh.h"
#include "litewq/utils/logging.h"
#include "litewq/surface/WavefrontOBJ.h"
#include "litewq/platform/OpenGL/GLShader.h"
#include "litewq/camera/camera.h"
#include "litewq/utils/Loader.h"
#include "litewq/scent/scent.h"

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
	int k = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) ? 2 : 1;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.process_keyboard(FORWARD, deltaTime * k);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.process_keyboard(BACKWARD, deltaTime * k);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.process_keyboard(LEFT, deltaTime * k);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.process_keyboard(RIGHT, deltaTime * k);
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

void renderQuad() {
    static unsigned int quadVAO = 0;
    static unsigned int quadVBO;
    if (quadVAO == 0) {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f};
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) (3 * sizeof(float)));
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
            Loader::readFromRelative("shader/bling-phong/lighting_map_vertex.glsl"),
            Loader::readFromRelative("shader/ssao/geom_frag.glsl"));

    GLShader phong(
            Loader::readFromRelative("shader/bling-phong/lighting_map_vertex.glsl"),
            Loader::readFromRelative("shader/ssao/geom_frag.glsl"));

    GLShader ssao(
            Loader::readFromRelative("shader/ssao/ssao_vertex.glsl"),
            Loader::readFromRelative("shader/ssao/ssao_frag.glsl"));

    ssao.Bind();
    ssao.updateUniformInt("gPosition", 0);
    ssao.updateUniformInt("gNormal", 1);
    ssao.updateUniformInt("texNoise", 2);

    GLShader ssao_blur(
            Loader::readFromRelative("shader/ssao/ssao_vertex.glsl"),
            Loader::readFromRelative("shader/ssao/blur_frag.glsl"));

    GLShader lighting(
            Loader::readFromRelative("shader/ssao/ssao_vertex.glsl"),
            Loader::readFromRelative("shader/ssao/lighting_frag.glsl"));

    lighting.Bind();
    lighting.updateUniformInt("gPosition", 0);
    lighting.updateUniformInt("gNormal", 1);
    lighting.updateUniformInt("gAlbedo", 2);
    lighting.updateUniformInt("ssao", 3);

    // another square
    float vertices2[] = {
            // positions        // normals      // texture coords
            0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

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

    auto tree =
            TriMesh::from_obj(Loader::getAssetPath("model/tree/Tree1.obj"));
    auto tree_raw = static_cast<TriMesh *>(tree.get());
    tree_raw->initGL();
    tree_raw->shader = &phong;

    // auto wolf =
    //         TriMesh::from_obj(Loader::getAssetPath("model/wolf/wolf.obj"));
    // auto wolf_raw = static_cast<TriMesh *>(wolf.get());
    // wolf_raw->initGL();
    // wolf_raw->shader = &phong;

    /* Shadow mapping, depth frame buffer and stored in texture. */
    unsigned int DepthMapFBO, DepthMap;
    glGenFramebuffers(1, &DepthMapFBO);

    glGenTextures(1, &DepthMap);
    glBindTexture(GL_TEXTURE_2D, DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 window_width, window_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glm::vec4 BorderColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(BorderColor));
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, DepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create G buffer
    GLuint gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Create 3 textures for G buffer
    GLuint gPosition, gNormal, gAlbedoSpec;
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, window_width, window_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    // Create and attach depth buffer (renderbuffer)
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // Also check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG(FATAL) << "Framebuffer not complete!";

    // SSAO framebuffer & texture
    GLuint ssaoFBO, ssaoBlurFBO;
    glGenFramebuffers(1, &ssaoFBO);
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    GLuint ssaoColorBuffer, ssaoColorBufferBlur;
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG(FATAL) << "SSAO Framebuffer not complete!";
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, window_width, window_height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        LOG(FATAL) << "SSAO Blur Framebuffer not complete!";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Generate sample kernel
    std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
    std::vector<glm::vec3> ssaoKernel;
    for (GLuint i = 0; i < 64; ++i) {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0;
        scale = Lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    // Generate noise texture
    std::vector<glm::vec3> ssaoNoise;
    for (GLuint i = 0; i < 16; i++) {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        ssaoNoise.push_back(noise);
    }
    GLuint noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Render loop
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // 1. Geometry pass
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup
        phong.Bind();
        glm::vec3 light_pos = glm::vec3(0.f, 80.f, 0.f);
        glm::mat4 light_projection = glm::ortho(-10.f, 10.f, -10.f, 10.f, 1.5f, 7.5f);
        glm::mat4 light_view = glm::lookAt(light_pos, glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
        glm::mat4 world2light = light_projection * light_view;
        phong.updateUniformFloat3("light.pos", glm::vec3(0.0f, 100.0f, 0.0f));
        phong.updateUniformFloat3("light.Ia", glm::vec3(0.5f, 0.5f, 0.5f));
        phong.updateUniformFloat3("light.Id", glm::vec3(1.0f, 1.0f, 1.0f));
        phong.updateUniformFloat3("light.Is", glm::vec3(0.1f, 0.1f, 0.1f));
        phong.updateUniformFloat3("view_pos", camera.get_position());

        glm::mat4 view = camera.get_view_matrix();
        glm::mat4 projection = glm::perspective(camera.get_zoom(), (float) window_width / (float) window_height, 0.1f, 100.0f);
        phong.updateUniformMat4("view", view);
        phong.updateUniformMat4("projection", projection);

        glm::mat4 model = glm::mat4(1.0f);

        // Draw 21x21 tiles around camera, manually assign material
        glBindTexture(GL_TEXTURE_2D, texGrass);
        glBindVertexArray(VAO2);
        phong.updateUniformFloat3("material.Ks", glm::vec3(.5f, .5f, .5f));
        phong.updateUniformFloat("material.highlight_decay", 200.f);
        glm::vec3 cameraPos = camera.get_position();
        for (int i = -10; i <= 10; i++)
            for (int j = -10; j <= 10; j++) {
                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(i * 1.0f + floor(cameraPos.x), 0.0f, j * 1.0f + floor(cameraPos.z)));
                phong.updateUniformMat4("model", model);
                glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
            }


        /* always put wolf in front of camera with some distance */
        glm::vec3 view_dir = camera.get_view_dir();
        glm::vec3 init_view_dir = glm::vec3(0.f, 0.f, -1.f);
        glm::vec3 ground_up_vec = glm::vec3(0.f, 1.f, 0.f);
        glm::vec3 init_ground_up_vec = glm::vec3(0.f, 1.f, 0.f);

        glm::vec3 head_dir = glm::cross(glm::cross(ground_up_vec, view_dir), ground_up_vec);
        glm::vec3 wolf_pos = camera.get_position() + head_dir * 0.1f;

        glm::mat4 wolf_model2world = glm::lookAt(wolf_pos, wolf_pos + head_dir, ground_up_vec);
        phong.updateUniformMat4("model", glm::mat4(1.0f));
        // wolf_raw->render();

        phong.updateUniformMat4("model", glm::scale(glm::mat4(1.0f), glm::vec3(.5f, .5f, .5f)));
        tree_raw->render();

        // Draw scent
        scent_shader.Bind();
        scent_shader.updateUniformMat4("view", view);
        scent_shader.updateUniformMat4("projection", projection);
        scent.render(cameraPos, view, projection, glm::vec4(0, 0, current_width, current_height));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. Create SSAO texture
        // we render it directly to the screen for now
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        ssao.Bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, noiseTexture);
        ssao.updateUniformMat4("projection", projection);
        for (int i = 0; i < 64; i++)
            ssao.updateUniformFloat3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. Blur SSAO texture to remove noise
        glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
        glClear(GL_COLOR_BUFFER_BIT);
        ssao_blur.Bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
        renderQuad();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 4. Lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lighting.Bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glActiveTexture(GL_TEXTURE3);// add extra SSAO texture to lighting pass
        glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
        lighting.updateUniformFloat3("viewPos", camera.get_position());
        renderQuad();

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
