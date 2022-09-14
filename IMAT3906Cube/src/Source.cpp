#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>


#include "Shader.h"
#include "Camera.h"
#include "Renderer.h"

#include <string>
#include <iostream>
#include <numeric>


// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;
const unsigned int SHADOW_WIDTH = 1024;
const unsigned int SHADOW_HEIGHT = 1024;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void updatePerFrameUniforms(Shader& cubeShader, Shader& floorShader, Shader& postProcess, Camera camera, bool DL, bool PL, bool SL, bool toneMap, bool invert, bool grey, bool bloom, bool RL);
void setUniforms(Shader& shader);
void setFBOdepth();
void setFBOColourAndDepth();
void setFBOblur();
void setFBOdepthMap();

// camera
Camera camera(glm::vec3(0,0,9));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool DL = true;
bool PL = false;
bool SL = false;
bool RL = false;
bool depth = false;
bool bright = false;
bool toneMap = false;
bool invert = false;
bool grey = false;
bool bloom = false;
bool gamma = false;
float bloomBrightness;

//arrays
//shader
unsigned int floorVBO, cubeVBO, floorEBO, cubeEBO, cubeVAO, floorVAO;
//framebuffer
unsigned int myFBO, myFBOdepth, FBO_blur, colAttachment, depthAttachment, FBO_ColourAndDepth, colourAttachment[2], blurredTexture, depthMapFBO, depthMap;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//light position
glm::vec3 lightDir = glm::vec3(0.1, -1.0, 0.0);

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IMAT3907", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	Renderer renderer(SCR_WIDTH, SCR_HEIGHT);
	Shader cubeShader("..\\shaders\\plainVert.vs", "..\\shaders\\plainFrag.fs");
	Shader floorShader("..\\shaders\\floorVert.vs", "..\\shaders\\floorFrag.fs");
	Shader lightShader("..\\shaders\\lightShader.vs", "..\\shaders\\lightShader.fs");
	Shader postProcess("..\\shaders\\postProcess.vs", "..\\shaders\\postProcess.fs");
	Shader depthProcess("..\\shaders\\postProcess.vs", "..\\shaders\\dPP.fs");
	Shader blurShader("..\\shaders\\postProcess.vs", "..\\shaders\\blurShader.fs");
	Shader bloomShader("..\\shaders\\postProcess.vs", "..\\shaders\\bloomShader.fs");
	Shader shadowMapShader("..\\shaders\\depthShader.vs", "..\\shaders\\depthShader.fs");
	Shader skyBoxShader("..\\shaders\\SB.vs", "..\\shaders\\SB.fs");

	int maxColourAttachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColourAttachments);
	std::cout << "max colour attachements: " << maxColourAttachments << std::endl;

	skyBoxShader.use();
	skyBoxShader.setInt("skybox", 0);

	setUniforms(cubeShader);
	setUniforms(floorShader);
	cubeShader.setFloat("PXscale", 0.01);
	floorShader.setFloat("PXscale", 0.015);

	postProcess.use();
	postProcess.setInt("image", 0);
	postProcess.setInt("hdrBuffer", FBO_ColourAndDepth);
	postProcess.setFloat("exposure", 1.0);

	depthProcess.use();
	depthProcess.setInt("image", 0);

	blurShader.use();
	blurShader.setInt("image", 0);

	bloomShader.use();
	bloomShader.setInt("image", 0);
	bloomShader.setInt("bloomBlur", 1);
	bloomShader.setFloat("exposure", 1.5);

	//setFBOdepth();
	setFBOColourAndDepth();
	setFBOblur();
	setFBOdepthMap();

	float orthoSize = 10;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		//float time = (float)(currentFrame * 0.5);
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processInput(window);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		////MRT to HDR FBO pass
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_ColourAndDepth); //at location 0, bright parts at location 1
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		updatePerFrameUniforms(cubeShader, floorShader, postProcess, camera, DL, SL, PL, toneMap, invert, grey, gamma, RL);
		renderer.renderScene(cubeShader, floorShader, skyBoxShader, camera);
		renderer.renderLights(lightShader, camera);

		//blur attachment pass
		glBindFramebuffer(GL_FRAMEBUFFER, FBO_blur);
		glDisable(GL_DEPTH_TEST);
		blurShader.use();
		blurShader.setInt("horz", 1);
		renderer.renderQuad(blurShader, colourAttachment[1]); // 1 is bright part
		blurShader.setInt("horz", 0);
		renderer.renderQuad(blurShader, blurredTexture);

		//pass for depth
		//glBindFramebuffer(GL_FRAMEBUFFER, myFBOdepth);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//updatePerFrameUniforms(cubeShader, floorShader, postProcess, camera, DL, SL, PL, toneMap, invert, grey, gamma, RL);
		//renderer.renderScene(cubeShader, floorShader, skyBoxShader, camera);
		//renderer.renderLights(lightShader, camera);

		//shadow mapping
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		
		glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, -orthoSize, 2*orthoSize);
		glm::mat4 lightView = glm::lookAt(lightDir*glm::vec3(-1.0), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 lightSpaceMatrix = lightProjection * lightView;
		shadowMapShader.use();
		shadowMapShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		renderer.renderCubes(shadowMapShader);
		renderer.renderFloor(shadowMapShader);

		//render to screen 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glDisable(GL_DEPTH_TEST);
		//2nd pass with normal shader and perspec proj, also need to add depthMap
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cubeShader.use();
		cubeShader.setInt("depthMap", 5);
		cubeShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		floorShader.use();
		floorShader.setInt("depthMap", 5);
		floorShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		if (depth == false) {
			if (bloom == false)
			{
				if (bright == false) renderer.renderQuad(postProcess, colourAttachment[0]);
				else if (bright == true) renderer.renderQuad(postProcess, colourAttachment[1]);
			}
			else if (bloom == true)
			{
				renderer.renderQuad(bloomShader, colourAttachment[0], blurredTexture);
			}
			if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
				renderer.renderQuad(postProcess, blurredTexture);
		}
		else if (depth == true) {
			renderer.renderQuad(depthProcess, depthMap);
			//renderer.renderQuad(depthProcess, depthAttachment);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.ProcessKeyboard(MOVEUP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(MOVEDOWN, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		if (DL == true) DL = false;
		else DL = true;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		if (PL == true) PL = false;
		else PL = true;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		if (SL == true) SL = false;
		else SL = true;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		if (RL == true) RL = false;
		else RL = true;
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
		if (toneMap == false) toneMap = true;
		else toneMap = false;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
		if (bright == false) bright = true;
		else bright = false;
	}
	if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
		if (invert == false) invert = true;
		else invert = false;
	}
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) {
		if (grey == false) grey = true;
		else grey = false;
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
		if (gamma == false) gamma = true;
		else gamma = false;
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		if (bloom == false) bloom = true;
		else bloom = false;
	}
	if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
		if (depth == false) depth = true;
		else depth = false;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void setUniforms(Shader& shader)
{
	shader.use();
	float bloomBrightness = 0.85f;

	//rim lighting
	shader.setVec3("rimColor", glm::vec3(1.0));
	shader.setFloat("rimPower", 20.0);

	//directional light
	//glm::vec3 lightDir = glm::vec3(0.0, -1.0, 0.0);
	glm::vec3 lightColor = glm::vec3(0.8, 0.8, 0.8);
	shader.setVec3("lightCol", lightColor);
	shader.setVec3("lightDir", lightDir);

	shader.setInt("diffuseTexture", 0);
	shader.setInt("specularTexture", 1);
	shader.setInt("normalMap", 2);
	shader.setInt("disMap", 3);
	shader.setInt("bumpMap", 4);
	shader.setFloat("bloomBrightness", bloomBrightness);

	//point light
	float Kc = 1.0;
	float Kl = 0.22f;
	float Ke = 0.2f;

	glm::vec3 pLightPos[] = {
		glm::vec3(2.0, 3.0, 4.0),
		glm::vec3(-2.0, 2.0, 4.0),
		glm::vec3(-3.0, -1.0, 0.5)
	};

	glm::vec3 pLightCol[] = {
		glm::vec3(0.0, 3.0, 3.0),
		glm::vec3(2.0, 0.0, 0.0),
		glm::vec3(2.0, 2.0, 0.0)
	};


	//point light one
	shader.setVec3("pLight[0].position", pLightPos[0]);
	shader.setVec3("pLight[0].color", pLightCol[0]);
	shader.setFloat("pLight[0].Kc", Kc);
	shader.setFloat("pLight[0].Kl", Kl);
	shader.setFloat("pLight[0].Ke", Ke);

	//point light two
	shader.setVec3("pLight[1].position", pLightPos[1]);
	shader.setVec3("pLight[1].color", pLightCol[1]);
	shader.setFloat("pLight[1].Kc", Kc);
	shader.setFloat("pLight[1].Kl", Kl);
	shader.setFloat("pLight[1].Ke", Ke);

	//point light 3
	shader.setVec3("pLight[2].position", pLightPos[2]);
	shader.setVec3("pLight[2].color", pLightCol[2]);
	shader.setFloat("pLight[2].Kc", Kc);
	shader.setFloat("pLight[2].Kl", Kl);
	shader.setFloat("pLight[2].Ke", Ke);

	//spot light
	shader.setVec3("sLight.color", glm::vec3(0.0f, 2.0f, 0.0f));
	shader.setFloat("sLight.Kc", 1.0f);
	shader.setFloat("sLight.Kl", 0.027f);
	shader.setFloat("sLight.Ke", 0.0028f);
	shader.setFloat("sLight.innerRad", glm::cos(glm::radians(12.5f)));
	shader.setFloat("sLight.outerRad", glm::cos(glm::radians(17.5f)));


}	

void updatePerFrameUniforms(Shader& cubeShader, Shader& floorShader, Shader& postProcess, Camera camera, bool DL, bool PL, bool SL, bool toneMap, bool invert, bool grey, bool gamma, bool RL) {
	floorShader.use();
	floorShader.setVec3("sLight.position", camera.Position);
	floorShader.setVec3("sLight.direction", camera.Front);
	floorShader.setBool("DL", DL);
	floorShader.setBool("PL", PL);
	floorShader.setBool("SL", SL);
	floorShader.setBool("RL", RL);
	
	cubeShader.use();
	cubeShader.setVec3("sLight.position", camera.Position);
	cubeShader.setVec3("sLight.direction", camera.Front);
	cubeShader.setBool("DL", DL);
	cubeShader.setBool("PL", PL);
	cubeShader.setBool("SL", SL);
	cubeShader.setBool("RL", RL);

	postProcess.use();
	postProcess.setBool("toneMap", toneMap);
	postProcess.setBool("invert", invert);
	postProcess.setBool("grey", grey);
	postProcess.setBool("gamma", gamma);
}

void setFBOColourAndDepth() {
	glGenFramebuffers(1, &FBO_ColourAndDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_ColourAndDepth);
	glGenTextures(2, colourAttachment);
	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colourAttachment[i]);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colourAttachment[i], 0);
	}

	//depth
	glGenTextures(1, &depthAttachment);
	glBindTexture(GL_TEXTURE_2D, depthAttachment);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment, 0);

	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setFBOblur() {
	glGenFramebuffers(1, &FBO_blur);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO_blur);
	glGenTextures(2, &blurredTexture);
	glBindTexture(GL_TEXTURE_2D, blurredTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurredTexture, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	//check if complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
}

void setFBOdepth()
{
	glGenFramebuffers(1, &myFBOdepth);
	glBindFramebuffer(GL_FRAMEBUFFER, myFBOdepth);
	glGenTextures(1, &depthAttachment);
	glBindTexture(GL_TEXTURE_2D, depthAttachment);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setFBOdepthMap()
{
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
