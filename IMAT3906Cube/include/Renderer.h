#pragma once

#ifndef RENDERER_H
#define RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Shader.h"
#include "Camera.h"
#include "normalMapper.h"
#include "SkyBox.h"

class Renderer {
public:
	Renderer(unsigned int sW, unsigned int sH);
	void renderScene(Shader& cubeShader, Shader& floorShader, Shader& skyBoxShader, Camera camera);
	void renderLights(Shader& lightShader, Camera camera);
	void renderQuad(Shader& shader, unsigned int& textureObj);
	void renderQuad(Shader& shader, unsigned int& textureObj, unsigned int& textureObj2);
	void renderCubes(Shader& shader);
	void renderFloor(Shader& shader);

	SkyBox skybox;
private:
	void drawLight(Shader& shader);
	void loadTextureFiles();
	unsigned int loadTexture(char const* path);
	void createCube();
	void createFloor();
	void createQuad();
	unsigned int cubeVAO, floorVAO, cubeVBO, cubeEBO, floorVBO, floorEBO, quadVAO, quadVBO, quadEBO, VAO, VBO, EBO, lightVAO, lightVBO, lightEBO;
	unsigned int screenW, screenH;
	unsigned int cubeDiffuse, cubeSpec, cubeNormal, cubeDisMap, cubeBump;
	unsigned int floorDiffuse, floorSpec, floorNormal, floorDisMap, floorBump;

	const float floorSize = 5.0f;
	const float floorLevel = -2.0f;
	const glm::vec3 cubeColor = glm::vec3(1.0, 0.4, 0.4);
	const glm::vec3 floorColor = glm::vec3(0.1, 0.3, 0.3);
};
#endif