#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "shaders/shader.h"
#include "camera/camera.h"

#include <iostream>
#include <vector>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(char const* path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(15.2f, 5.0f, 15.0f);

//player settings
glm::vec3 playerSize = glm::vec3(0.6f, 0.0f, 0.6f);

enum class MaterialType {
    SolidColor,
    Flashing,
    Textured
};
//world object vector
struct WorldObject {
    glm::vec3 position;
    glm::vec3 scale;
    MaterialType material;
    unsigned int textureID = 0;
};

// collision object
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

AABB getAABB(const WorldObject& obj) {
    glm::vec3 half = obj.scale * 0.5f;
    return {
        obj.position - half,
        obj.position + half
    };
}

AABB getPlayerAABB() {
    glm::vec3 half = playerSize * 0.5f;
    return {
        camera.Position - half,
        camera.Position + half
    };
}

bool checkCollision(const AABB& a, const AABB& b) {
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
        (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
        (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

// background VAO/VBO
unsigned int quadVAO = 0;
unsigned int quadVBO;

//BG vertices
float quadVertices[] = {
    // positions    // texCoords
    -1.0f,  1.0f,   0.0f, 1.0f, // top-left
    -1.0f, -1.0f,   0.0f, 0.0f, // bottom-left
     1.0f, -1.0f,   1.0f, 0.0f, // bottom-right

    -1.0f,  1.0f,   0.0f, 1.0f, // top-left
     1.0f, -1.0f,   1.0f, 0.0f, // bottom-right
     1.0f,  1.0f,   1.0f, 1.0f  // top-right
};

std::vector<WorldObject> worldObjects = {
    {{0.0f, -1.0f, 0.0f }, {20.0f, 0.2f, 20.0f}, MaterialType::Flashing  }, // floor
    {{10.0f, 3.0f, 0.0f }, {0.2f, 8.0f, 10.0f }, MaterialType::Flashing  }, // left wall
    {{-10.0f, 3.0f, 0.0f}, {0.2f, 8.0f, 20.0f }, MaterialType::Flashing  }, // right wall
    {{0.0f, 3.0f, -10.0f}, {20.0f, 8.0f, 0.2f }, MaterialType::Flashing  }, // front wall
    {{ 0.0f, 3.0f, 10.0f}, {20.0f, 8.0f, 0.2f }, MaterialType::Flashing  }, // back wall
    {{ 0.0f, 7.0f, 0.0f }, {20.0f, 0.2f, 20.0f}, MaterialType::Flashing  }, // ceiling
    {{20.0f, -1.0f, 0.0f}, {20.0f, 0.2f, 20.0f}, MaterialType::SolidColor}, // floor
	{{30.0f, 3.0f, 0.0f }, {0.2f, 8.0f, 20.0f }, MaterialType::SolidColor}, // left wall
    {{20.0f, 3.0f,-10.0f}, {20.0f, 8.0f, 0.2f }, MaterialType::SolidColor}, // front wall
	{{20.0f, 7.0f, 0.0f }, {20.0f, 0.2f, 20.0f}, MaterialType::SolidColor}, // ceiling
    {{20.0f, -1.0f,20.0f}, {20.0f, 0.2f, 20.0f}, MaterialType::Textured  }, // floor
    {{10.0f, 3.0f, 20.0f}, {0.2f, 8.0f, 20.0f }, MaterialType::Textured  }, // right wall
    {{30.0f, 3.0f, 20.0f }, {0.2f, 8.0f, 20.0f },MaterialType::Textured  }, // left wall
    {{20.0f, 7.0f, 20.0f }, {20.0f, 0.2f, 20.0f},MaterialType::Textured  }, // ceiling
    {{20.0f, 3.0f, 20.0f }, {20.0f, 8.0f, 0.2f },MaterialType::Textured  }  // back wall
};

// shaders
// -------
Shader* renderShaders(WorldObject obj, Shader& lightingShader, Shader& basicLightingShader, Shader& lightingMap, Shader& light_casters, Shader& flashlight);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // shaders
    // -------
    Shader lightingShader("shaders/materials.vs", "shaders/materials.fs");
    Shader basicLightingShader("shaders/basic_lighting.vs", "shaders/basic_lighting.fs");
    Shader lightCubeShader("shaders/light_cube.vs", "shaders/light_cube.fs");
    Shader lightingMap("shaders/lighting_map.vs", "shaders/lighting_map.fs");
    Shader backgroundShader("shaders/bgShader.vs", "shaders/bgShader.fs");
	Shader lightCasters("shaders/light_casters.vs", "shaders/light_casters.fs");
    Shader flashlight("shaders/flashlight.vs", "shaders/flashlight.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };

    // first, configure the cube's VAO (and VBO)
    unsigned int VBO, cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    //texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    // we only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need (it's already bound, but we do it again for educational purposes)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // background VAO/VBO setup
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glBindVertexArray(quadVAO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);


    unsigned int diffuseMap = loadTexture("textures/backroomswall.jpeg");
    std::cout << "Loaded texture ID: " << diffuseMap << std::endl;
    if (diffuseMap == 0)
        std::cout << " Texture failed to load!" << std::endl;
    worldObjects.back().textureID = diffuseMap;

    if (diffuseMap == 0)
        std::cout << "texture failed to load";

    lightingMap.use();
	lightingMap.setInt("material.diffuse", 0);

    std::cout << "Texture ID: " << worldObjects.back().textureID << std::endl;

    unsigned int bgTexture = loadTexture("textures/liminalBG.jpg");

	std::cout << "BG Texture ID: " << bgTexture << std::endl;
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // input
        // -----
        // keep track of player AABB
        glm::vec3 oldPos = camera.Position;
        processInput(window);

        // update physics
        // -----
		camera.updatePhysics(deltaTime);

        // check collisions
        // ----------------
		AABB playerBox = getPlayerAABB();
        for (auto& obj : worldObjects) {
            if (checkCollision(playerBox, getAABB(obj))) {
                camera.Position = oldPos;
                break;
            }
        }

        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        backgroundShader.use();
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bgTexture);
        backgroundShader.setInt("bgTexture", 1);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        //glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);

        // be sure to activate shader when setting uniforms/drawing objects
	

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        lightingShader.use();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);

        basicLightingShader.use();
        basicLightingShader.setMat4("projection", projection);
        basicLightingShader.setMat4("view", view);

        lightingMap.use();
		lightingMap.setMat4("projection", projection);
		lightingMap.setMat4("view", view);

        lightCasters.use();
		lightCasters.setMat4("projection", projection);
		lightCasters.setMat4("view", view);

        flashlight.use();
		flashlight.setMat4("projection", projection);
		flashlight.setMat4("view", view);

        // custom view/projection transformations
        for (auto& obj : worldObjects) {
			Shader* activeShader = renderShaders(obj, lightingShader, basicLightingShader, lightingMap, lightCasters, flashlight);
            if (!activeShader) continue;

            if (activeShader) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, obj.position);
                model = glm::scale(model, obj.scale);
                activeShader->setMat4("model", model);

                if (obj.material == MaterialType::Textured && obj.textureID != 0) {
                    activeShader->use();
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, obj.textureID);
                    //activeShader->setInt("material.diffuse", 0);
                }

                glBindVertexArray(cubeVAO);
                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);


        // also draw the lamp object
        //lightCubeShader.use();
        //lightCubeShader.setMat4("projection", projection);
        //lightCubeShader.setMat4("view", view);
        //model = glm::mat4(1.0f);
        //model = glm::translate(model, lightPos);
        //model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        //lightCubeShader.setMat4("model", model);

        //glBindVertexArray(lightCubeVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &quadVBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
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
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.HandleJump(deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

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
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

Shader* renderShaders(WorldObject obj, Shader& lightingShader, Shader& basicLightingShader, Shader& lightingMap, Shader& light_casters, Shader& flashlight) {
    Shader* activeShader = nullptr;
    if (obj.material == MaterialType::Flashing) {
        activeShader = &lightingShader;
        activeShader->use();
        activeShader->setVec3("light.position", lightPos);
        activeShader->setVec3("viewPos", camera.Position);

        glm::vec3 lightColor;
        lightColor.x = sin(glfwGetTime() * 2.0f);
        lightColor.y = sin(glfwGetTime() * 0.7f);
        lightColor.z = sin(glfwGetTime() * 1.3f);

        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);

        activeShader->setVec3("light.ambient", ambientColor);
        activeShader->setVec3("light.diffuse", diffuseColor);
        activeShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        activeShader->setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
        activeShader->setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
        activeShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
        activeShader->setFloat("material.shininess", 32.0f);
        return activeShader;
	}
    else if (obj.material == MaterialType::SolidColor) {
        activeShader = &basicLightingShader;
        activeShader->use();
        activeShader->setVec3("lightPos", glm::vec3(lightPos.x, lightPos.y, lightPos.z - 10));
        activeShader->setVec3("lightColor", 0.5f, 0.5f, 0.5f);
        activeShader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
        activeShader->setVec3("viewPos", camera.Position);
        return activeShader;
    }
    else if (obj.material == MaterialType::Textured) {
        activeShader = &flashlight;

		activeShader->use();
        activeShader->setVec3("light.position", camera.Position);
		activeShader->setVec3("light.direction", camera.Front);
		activeShader->setFloat("light.cutOff", glm::cos(glm::radians(8.5f)));
		activeShader->setFloat("light.outerCutOff", glm::cos(glm::radians(12.0f)));
        activeShader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		activeShader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
		activeShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // light drop off over distance vals
        activeShader->setFloat("light.constant", 1.0f);
		activeShader->setFloat("light.linear", 0.12f);
		activeShader->setFloat("light.quadratic", 0.0064f);

		//activeShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		activeShader->setFloat("material.shininess", 32.0f);

        return activeShader;
    }
}

unsigned int loadTexture(char const* path) {
    unsigned int textureID;
    glGenTextures(1, &textureID);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        std::cout << "Loading texture: " << path << std::endl;
        std::cout << "Width: " << width << " Height: " << height << " Components: " << nrComponents << std::endl;
        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
    }
    return textureID;
}
