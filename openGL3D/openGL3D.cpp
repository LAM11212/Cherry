#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "shaders/shader.h"
#include "camera/camera.h"
#include "mesh.h"
#include "model.h"
#include "WorldObject.h"
#include "MaterialType.h"
#include "worldFactory.h"
#include "WorldEditor.h"


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
bool DEBUG = true;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

//player settings
glm::vec3 playerSize = glm::vec3(0.6f, 0.0f, 0.6f);


// collision object
struct AABB {
    glm::vec3 min;
    glm::vec3 max;
};

AABB getAABB(const WorldObject& obj) {
    glm::vec3 half = obj.transform.scale * 0.5f;
    return {
        obj.transform.position - half,
        obj.transform.position + half
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

// shaders
// -------
//Shader* renderShaders(WorldObject obj, Shader& lightingShader, Shader& basicLightingShader, Shader& lightingMap, Shader& light_casters, Shader& flashlight);

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

    if(!DEBUG)
        glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    if(!DEBUG)
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

    InitWorldShaders();
    WorldFactory worldFactory;
	worldFactory.loadWorld("world.json");
	std::vector<WorldObject>& worldObjects = worldFactory.getObjects();

    WorldEditor editor(window);
	editor.BindWorld(worldFactory);



    // shaders
    // -------
    Shader horrorShader("shaders/universal_lit.vs", "shaders/universal_lit.fs");
	Shader backgroundShader("shaders/bgShader.vs", "shaders/bgShader.fs");
	Shader gunShader("shaders/gun.vs", "shaders/gun.fs");
    Shader pyramidShader("shaders/pyramid.vs", "shaders/pyramid.fs");
	Shader worldLightingShader("shaders/light_casters.vs", "shaders/light_casters.fs");

    Model Gun("models/futuristic-sci-fi-glock/source/ASM - PBR Metallic Roughness/ASM - PBR Metallic Roughness.fbx");

    if (Gun.meshes.empty())
    {
        std::cout << "Model failed to load or has no meshes\n";
    }
    else
    {
        std::cout << "Loaded model with: " << Gun.meshes.size() << " meshes\n";
    }

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

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

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

    float pyramid[] = {
        // Positions           // Normals            // texCoords

        // Base (two triangles, facing down)
        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, 
         0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,

        -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f, 
         0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f, 
        -0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,

        // Front face
        -0.5f, -0.5f,  0.5f,   0.0f,  0.5f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f,  0.5f,  1.0f,  1.0f, 0.0f,
         0.0f,  0.5f,  0.0f,   0.0f,  0.5f,  1.0f,  0.5f, 1.0f,

         // Right face
          0.5f, -0.5f,  0.5f,   1.0f,  0.5f,  0.0f,  0.0f, 0.0f, 
          0.5f, -0.5f, -0.5f,   1.0f,  0.5f,  0.0f,  1.0f, 0.0f,
          0.0f,  0.5f,  0.0f,   1.0f,  0.5f,  0.0f,  0.5f, 1.0f,

          // Back face
           0.5f, -0.5f, -0.5f,   0.0f,  0.5f, -1.0f,  0.0f, 0.0f, 
          -0.5f, -0.5f, -0.5f,   0.0f,  0.5f, -1.0f,  1.0f, 0.0f,
           0.0f,  0.5f,  0.0f,   0.0f,  0.5f, -1.0f,  0.5f, 1.0f,

           // Left face
           -0.5f, -0.5f, -0.5f,  -1.0f,  0.5f,  0.0f,  0.0f, 0.0f,
           -0.5f, -0.5f,  0.5f,  -1.0f,  0.5f,  0.0f,  1.0f, 0.0f,
            0.0f,  0.5f,  0.0f,  -1.0f,  0.5f,  0.0f,  0.5f, 1.0f,
    };

    unsigned int pyramidVBO, pyramidVAO;
    glGenVertexArrays(1, &pyramidVAO);
	glGenBuffers(1, &pyramidVBO);

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid), pyramid, GL_STATIC_DRAW);

    glBindVertexArray(pyramidVAO);

    //position attrib
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attrib
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // texture attrib
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

    unsigned int pyramidTexture = loadTexture("textures/obama.jpg");

    unsigned int diffuseMap = loadTexture("textures/backroomswall.jpeg");
    std::cout << "Loaded texture ID: " << diffuseMap << std::endl;
    if (diffuseMap == 0)
        std::cout << " Texture failed to load!" << std::endl;
    worldObjects.back().textureID = diffuseMap;

    if (diffuseMap == 0)
        std::cout << "texture failed to load";

    //lightingMap.use();
	//lightingMap.setInt("material.diffuse", 0);

    std::cout << "Texture ID: " << worldObjects.back().textureID << std::endl;

    unsigned int bgTexture = loadTexture("textures/liminalBG.jpg");

    // GUN TEXTURE
	unsigned int gunTexture = loadTexture("models/futuristic-sci-fi-glock/textures/Stylized glock_Empty_BaseColor.png");
    gunShader.use();
    gunShader.setInt("texture_diffuse1", 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gunTexture);


    //Gun.Draw(gunShader);

	std::cout << "BG Texture ID: " << bgTexture << std::endl;

    // world ligting initialization, more in loop below
    /*worldLightingShader.use();
    worldLightingShader.setInt("material.diffuse", 0);
    worldLightingShader.setInt("material.specular", 1);*/

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
		camera.updatePhysics(deltaTime, DEBUG);

        // check collisions
        // ----------------
        if (!DEBUG)
        {
            AABB playerBox = getPlayerAABB();
            for (auto& obj : worldObjects) {
                if (checkCollision(playerBox, getAABB(obj))) {
                    camera.Position = oldPos;
                    break;
                }
            }
        }

        // render
        // ------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //glDisable(GL_DEPTH_TEST);

        editor.BeginFrame();
        editor.DrawUI();
        editor.Inspector();
        editor.ObjectMenu();

       /* backgroundShader.use();
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bgTexture);
        backgroundShader.setInt("bgTexture", 1);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);*/
        //glBindVertexArray(0);
        glEnable(GL_DEPTH_TEST);

        // be sure to activate shader when setting uniforms/drawing objects	

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

		gunShader.use();
		gunShader.setMat4("projection", projection);
		gunShader.setMat4("view", view);

  //      pyramidShader.use();
		//pyramidShader.setMat4("projection", projection);
		//pyramidShader.setMat4("view", view);
  //      pyramidShader.setInt("texture1", 0);
  //      glm::mat4 pyramidModel = glm::mat4(1.0f);
  //      //pyramidModel = glm::translate(pyramidModel, glm::vec3(-3.0f, 0.0f, -3.0f));
  //      pyramidModel = glm::rotate(pyramidModel, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
  //      pyramidModel = glm::scale(pyramidModel, glm::vec3(2.0f));
		//pyramidShader.setMat4("model", pyramidModel);
  //      pyramidShader.setVec3("light.position", lightPos);
		//pyramidShader.setVec3("light.ambient", 0.5f, 0.2f, 0.3f);
  //      pyramidShader.setVec3("light.diffuse", 0.5f, 0.5f, 1.0f);
		//pyramidShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
  //      pyramidShader.setFloat("material.shininess", 32.0f);
		//pyramidShader.setMat4("material.diffuse", glm::mat4(1.0f));
  //      glActiveTexture(GL_TEXTURE0);
  //      glBindTexture(GL_TEXTURE_2D, pyramidTexture);

		//glBindVertexArray(pyramidVAO);
  //      glDrawArrays(GL_TRIANGLES, 0, 18);

        //load our gun model and move to camera position
        gunShader.use();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(camera.Position));

        // apply camera rotation
        glm::mat4 rotation = glm::mat4(1.0f);
		rotation[0] = glm::vec4(camera.Right, 0.0f);
		rotation[1] = glm::vec4(camera.Up, 0.0f);
		rotation[2] = glm::vec4(-camera.Front, 0.0f);
        model *= rotation;

        // offset
        glm::vec3 offset = glm::vec3(0.3f, -0.3f, -0.7f);
        model = glm::translate(model, offset);

        //fix rotation
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, -1.0f, 0.0f));

        // scale down
        model = glm::scale(model, glm::vec3(0.0009f));

        gunShader.use();
		gunShader.setMat4("model", model);
		Gun.Draw(gunShader);

        // custom view/projection transformations
        for (auto& obj : worldObjects) {
			obj.Render(cubeVAO, projection, view, camera);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // world transformation
        //glm::mat4 lightmodel = glm::mat4(1.0f);
        //lightingShader.setMat4("model", lightmodel);

        //// also draw the lamp object
        //lightCubeShader.use();
        //lightCubeShader.setMat4("projection", projection);
        //lightCubeShader.setMat4("view", view);
        //model = glm::mat4(1.0f);
        //model = glm::translate(lightmodel, lightPos);
        //model = glm::scale(lightmodel, glm::vec3(0.2f)); // a smaller cube
        //lightCubeShader.setMat4("model", lightmodel);

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
    if (!DEBUG) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.HandleJump(deltaTime, DEBUG);
    }
    else if (DEBUG) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.DebugProcessKeyboard(FORWARD, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.DebugProcessKeyboard(BACKWARD, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.DebugProcessKeyboard(LEFT, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.DebugProcessKeyboard(RIGHT, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.DebugProcessKeyboard(ASCEND, deltaTime, DEBUG);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera.DebugProcessKeyboard(DESCEND, deltaTime, DEBUG);
    }
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

// this method is more or less useless as of where i am in the project; however i will keep this here just in case.
//Shader* renderShaders(WorldObject obj, Shader& lightingShader, Shader& basicLightingShader, Shader& lightingMap, Shader& light_casters, Shader& flashlight) {
//    Shader* activeShader = nullptr;
//    if (obj.material == MaterialType::Flashing) {
//        activeShader = &lightingShader;
//        activeShader->use();
//        activeShader->setVec3("light.position", lightPos);
//        activeShader->setVec3("viewPos", camera.Position);
//
//        glm::vec3 lightColor;
//        lightColor.x = sin(glfwGetTime() * 2.0f);
//        lightColor.y = sin(glfwGetTime() * 0.7f);
//        lightColor.z = sin(glfwGetTime() * 1.3f);
//
//        glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
//        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.2f);
//
//        activeShader->setVec3("light.ambient", ambientColor);
//        activeShader->setVec3("light.diffuse", diffuseColor);
//        activeShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
//
//        activeShader->setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
//        activeShader->setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
//        activeShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
//        activeShader->setFloat("material.shininess", 32.0f);
//        return activeShader;
//	}
//    else if (obj.material == MaterialType::SolidColor) {
//        activeShader = &basicLightingShader;
//        activeShader->use();
//        activeShader->setVec3("lightPos", glm::vec3(lightPos.x, lightPos.y, lightPos.z - 10));
//        activeShader->setVec3("lightColor", 0.5f, 0.5f, 0.5f);
//        activeShader->setVec3("objectColor", 1.0f, 0.5f, 0.31f);
//        activeShader->setVec3("viewPos", camera.Position);
//        return activeShader;
//    }
//    else if (obj.material == MaterialType::Textured) {
//        activeShader = &flashlight;
//
//		activeShader->use();
//        activeShader->setVec3("light.position", camera.Position);
//		activeShader->setVec3("light.direction", camera.Front);
//		activeShader->setFloat("light.cutOff", glm::cos(glm::radians(8.5f)));
//		activeShader->setFloat("light.outerCutOff", glm::cos(glm::radians(12.0f)));
//        activeShader->setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
//		activeShader->setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
//		activeShader->setVec3("light.specular", 1.0f, 1.0f, 1.0f);
//
//        // light drop off over distance vals
//        activeShader->setFloat("light.constant", 1.0f);
//		activeShader->setFloat("light.linear", 0.12f);
//		activeShader->setFloat("light.quadratic", 0.0064f);
//
//		//activeShader->setVec3("material.specular", 0.5f, 0.5f, 0.5f);
//		activeShader->setFloat("material.shininess", 32.0f);
//
//        return activeShader;
//    }
//}

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
