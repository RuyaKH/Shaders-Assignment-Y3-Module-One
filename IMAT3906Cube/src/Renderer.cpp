#include "Renderer.h"

Renderer::Renderer(unsigned int sW, unsigned int sH)
{
	//create cube and floor
	createCube();
	createFloor();
	createQuad();
	loadTextureFiles();
	screenW = sW;
	screenH = sH;
	skybox.createSkyBox();
}

void Renderer::renderScene(Shader& shader, Shader& floorShader, Shader& skyBoxShader, Camera camera)
{
	//MVP
	//set MVP
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenW / (float)screenH, 0.1f, 1000.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);

	skyBoxShader.use();
	skyBoxShader.setMat4("projection", projection);
	skyBoxShader.setMat4("view", glm::mat4(glm::mat3(camera.GetViewMatrix())));
	skybox.renderSkyBox(skyBoxShader);

	floorShader.use();
	floorShader.setMat4("projection", projection);
	floorShader.setMat4("view", view);
	floorShader.setMat4("model", model);
	floorShader.setVec3("viewPos", camera.Position);
	renderFloor(floorShader);

	shader.use();
	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	shader.setMat4("model", model);
	shader.setVec3("viewPos", camera.Position);
	renderCubes(shader);
}

void Renderer::renderLights(Shader& lightShader, Camera camera)
{
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)screenW / (float)screenH, 0.1f, 1000.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1.0f);

	lightShader.use();
	lightShader.setMat4("projection", projection);
	lightShader.setMat4("view", view);

	lightShader.setVec3("lightColor", glm::vec3(1.0, 1.0, 0.0));
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-3.0, -1.0, 0.5));
	model = glm::scale(model, glm::vec3(0.5f)); // a smaller cube
	lightShader.setMat4("model", model);
	drawLight(lightShader);

	lightShader.setVec3("lightColor", glm::vec3(5.0, 0.0, 0.0));
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2.0, 2.0, 4.0));
	model = glm::scale(model, glm::vec3(0.5f)); // a smaller cube
	lightShader.setMat4("model", model);
	drawLight(lightShader);

	lightShader.setVec3("lightColor", glm::vec3(0.0, 5.0, 5.0));
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0, 3.0, 4.0));
	model = glm::scale(model, glm::vec3(0.5f)); // a smaller cube
	lightShader.setMat4("model", model);
	drawLight(lightShader);

	
}

void Renderer::renderCubes(Shader& shader)
{
	//bind vao
	//draw
	//shader.setVec3("objectCol", cubeColor);
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cubeDiffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, cubeSpec);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, cubeNormal);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, cubeDisMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, cubeBump);

	glBindVertexArray(cubeVAO);  // bind and draw cube
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 0.0, 3.0));
	shader.setMat4("model", model);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0, 0.0, -5.0));
	model = glm::rotate(model, (float)(glfwGetTime() * 5.0), glm::vec3(2.0, 2.0, 2.0));
	model = glm::scale(model, glm::vec3(2.0));
	shader.setMat4("model", model);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void Renderer::renderFloor(Shader& shader)
{
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, floorDiffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, floorSpec);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, floorNormal);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, floorDisMap);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, floorBump);

	//bind vao
	//draw
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	//shader.setVec3("objectCol", floorColor);
	glBindVertexArray(floorVAO);  // bind and draw floor
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Renderer::drawLight(Shader& shader)
{
	shader.use();
	glBindVertexArray(cubeVAO);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


}

void Renderer::renderQuad(Shader& shader, unsigned int& textureObj)
{
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureObj);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Renderer::renderQuad(Shader& shader, unsigned int& textureObj, unsigned int& textureObj2)
{
	shader.use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureObj);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureObj2);
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

void Renderer::createCube()
{
	// cube data
	float cubeVertices[] = {
		//back
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,// 0 
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f, //1
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,

		//front
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,//4
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
		-0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,

		//left
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,//8
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

		//right
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,//12
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,

		//bottom
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,//16
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,

		//top	
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, //20
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,

	};

	unsigned int cubeIndices[] = {
		1,2,3,
		1,3,0,

		5,6,7,
		5,7,4,

		11,8,9,
		11,9,10,

		15,12,13,
		15,13,14,

		19,18,17,
		19,17,16,

		23,22,21,
		23,21,20
	};

	normalMapper normMap;
	normMap.calculateTanAndBitan(cubeVertices, 192, cubeIndices, 36);
	std::vector<float> updatedCubeVertices = normMap.getUpdatedVertexData();

	// Create VAO
	// Cube
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);

	glBindVertexArray(cubeVAO);
	// fill VBO with vertex data
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, updatedCubeVertices.size() * sizeof(GLfloat), updatedCubeVertices.data() , GL_STATIC_DRAW);
	// fill EBO with index data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// uv attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// tan attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// bitan attribute
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);
}

void Renderer::createFloor()
{
	float floorVertices[] = {
		-floorSize, floorLevel, -floorSize, 0.0, 1.0, 0.0, 0.0f, 0.0f,
		floorSize, floorLevel, -floorSize, 0.0, 1.0, 0.0, 1.0f, 0.0f,
		floorSize, floorLevel, floorSize, 0.0, 1.0, 0.0, 1.0f, 1.0f,
		-floorSize, floorLevel, floorSize, 0.0, 1.0, 0.0, 0.0f, 1.0f
	};

	unsigned int floorIndices[] = {
		3,2,1,
		3,1,0
	};

	normalMapper normMap;
	normMap.calculateTanAndBitan(floorVertices, 32, floorIndices, 6);
	std::vector<float> updatedFloorVertices = normMap.getUpdatedVertexData();

	//Floor
	glGenVertexArrays(1, &floorVAO);
	glGenBuffers(1, &floorVBO);
	glGenBuffers(1, &floorEBO);

	glBindVertexArray(floorVAO);

	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	glBufferData(GL_ARRAY_BUFFER, updatedFloorVertices.size() * sizeof(GLfloat), updatedFloorVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//uv
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	// tan
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);
	// bitan
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
	glEnableVertexAttribArray(4);
}

void Renderer::createQuad()
{
	float quadVertices[] = {
		//positions			// texture Coords
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f
	};
	//setup plane VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
}

void Renderer::loadTextureFiles()
{
	cubeDiffuse = loadTexture("..\\Texture\\woodCrate\\diffuse.jpg");
	cubeSpec = loadTexture("..\\Texture\\woodCrate\\specular.jpg");
	cubeNormal = loadTexture("..\\Texture\\woodCrate\\normal.jpg");
	cubeDisMap = loadTexture("..\\Texture\\woodCrate\\height.png");
	//cubeBump = loadTexture("..\\Texture\\woodCrate\\ambientOcclusion.jpg");

	floorDiffuse = loadTexture("..\\Texture\\metalCorrugated\\diffuse.jpg");
	floorSpec = loadTexture("..\\Texture\\metalCorrugated\\metallic.jpg");
	floorNormal = loadTexture("..\\Texture\\metalCorrugated\\normal.jpg");
	floorDisMap = loadTexture("..\\Texture\\metalCorrugated\\height.png");
	floorBump = loadTexture("..\\Texture\\metalCorrugated\\roughness.jpg");
}

unsigned int Renderer::loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //s = x axis
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //t = y axis, r if 3D
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
		std::cout << "loaded texture at path: " << path << " width " << width << " id " << textureID << std::endl;
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}