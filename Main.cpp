#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <math.h>

#include "LoadShader.h"
#include "QuadModel.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
using std::make_unique;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

//
const unsigned int SCR_WIDTH = 2400;
const unsigned int SCR_HEIGHT = 1350;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// UI
glm::mat4 mouseRotation{ 1.0f };
glm::mat4 scrollModel{ 1.0f };


int main()
{
    // glfw: initialize and configure
    // MIND THE VERSION!!!
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Quadrilateral Mesh Simplification", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    //GLFW callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //IMGui init    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");


    // configure global opengl state
    // Depth testing and face culling is off by default in OpenGL
    //z buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    //Load Models (Quad meshes)
    std::vector<modelPtr> models; //modelPtr = std::unique_ptr<Model>
    models.emplace_back(make_unique<QuadModel>(".\\models\\quadDavid.obj"));
    models.emplace_back(make_unique<QuadModel>(".\\models\\gargoyle.obj"));
    models.emplace_back(make_unique<QuadModel>(".\\models\\horse.obj"));

    //swap ownership between model pointers
    modelPtr currentModel;
    unsigned int currentModelId = 0;
    currentModel.swap(models[currentModelId]);
    

    //Load Shaders
    std::unique_ptr<GLuint> randomShader = make_unique<GLuint>(loadShader(".\\shaders\\randomQuads.vert",".\\shaders\\randomQuads.frag", ".\\shaders\\randomQuads.geom"));
    std::unique_ptr<GLuint> diffuseShader = make_unique<GLuint>(loadShader(".\\shaders\\diffuseQuad.vert",".\\shaders\\diffuseQuad.frag", ".\\shaders\\diffuseQuad.geom"));
    std::unique_ptr<GLuint> primitiveIDShader = make_unique<GLuint>(loadShader(".\\shaders\\randomQuads.vert", ".\\shaders\\byPrimitiveID.frag", ".\\shaders\\randomQuads.geom"));
    std::unique_ptr<GLuint> elementEdgesShader = make_unique<GLuint>(loadShader(".\\shaders\\elementEdges.vert", ".\\shaders\\elementEdges.frag", ".\\shaders\\elementEdges.geom"));


    //GLuint* currentShader = randomShader.get();
    std::unique_ptr<GLuint> currentShader;
    currentShader.swap(randomShader);


    //UI
    vec3 background{ 0.95f };
    float modelSize = 1.0f;
    float lightDegrees = 0.0f;
    int renderModeSelect = 0;
    bool drawEdges = false;
    float lineWidth = 2.0f;
    //view
    glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 viewDir = glm::vec3(0.0f, 0.0f, -1.0f);
    //light settings
    glm::vec3 lightPosInit = glm::vec3(-1.0f, 1.0f, 1.5f);
    glm::vec3 lightPos = lightPosInit;
    glm::vec3 lightDiffuse = glm::vec3(1, 1, 1);
    glm::vec3 lightSpecular = glm::vec3(1, 1, 1); 
    glm::vec3 lineColor = glm::vec3(1.0);

    //QMS
    GLfloat LoD = 1.0;

    //render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        //yDegrees += 1;
        //yDegrees =int(yDegrees)%360;

        glClearColor(background.x, background.y, background.z, 0.0); //background
        glLineWidth(lineWidth);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        switch (renderModeSelect)
        {
        case 0:
            currentShader.swap(randomShader);
            break;
        case 1:
            currentShader.swap(diffuseShader);
            break;
        case 2:
            currentShader.swap(primitiveIDShader);
            break;
        }

        //IMGui new frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //IMGui window
        ImGui::Begin("Quadrilateral Mesh Simplification");

        //ImGui::Checkbox("Line Drawing with Apparent Ridges", &mesh);

        ImGui::Text("Click + drag to move model. Scroll to zoom.");
        ImGui::Text("Ctrl + click + drag to move light source.");
        
        //choose shader
    
        ImGui::Columns(2, nullptr, false);
        ImGui::RadioButton("Random Color by Element", &renderModeSelect, 0);
        ImGui::RadioButton("Diffuse", &renderModeSelect, 1);
        ImGui::RadioButton("Prmitive ID", &renderModeSelect, 2);

        ImGui::NextColumn();
        ImGui::Checkbox("Draw Element Edges", &drawEdges);
        ImGuiColorEditFlags misc_flags = (0 | ImGuiColorEditFlags_NoDragDrop | 0 | ImGuiColorEditFlags_NoOptions);
        ImGui::ColorEdit3("Line Color", (float*)&lineColor, misc_flags);

        ImGui::Columns(1);
        //chose model
        const char* listboxItems[] = { "David", "Dog", "Horse", "Cow" };
        static int currentlistboxItem = 0;
        ImGui::ListBox("Model", &currentlistboxItem, listboxItems, IM_ARRAYSIZE(listboxItems), 3);

        currentModel.swap(models[currentModelId]); //swap back to original ownership
        currentModelId = currentlistboxItem;
        currentModel.swap(models[currentModelId]); //get new model

        ImGui::SliderFloat("LoD", &LoD, 0.0f, 1.0f);


        ImGui::ColorEdit3("Background Color", (float*)&background, misc_flags);
        //ImGui::SliderFloat("Rotate Global Light Source", &lightDegrees, 0.0f, 360.0f);   
        //ImGui::SliderFloat("Brightness", &diffuse, 0.0f, 2.0f);
        ImGui::End();

        switch (renderModeSelect)
		{ 
            case 0:
                currentShader.swap(randomShader);
				break;
            case 1:
				currentShader.swap(diffuseShader);
                break;
            case 2:
                currentShader.swap(primitiveIDShader);
                break;
        }
        glUseProgram(*currentShader);

        glUniform1i(glGetUniformLocation(*currentShader, "numElements"), currentModel->numFaces);
        /*
        switch (renderModeSelect)
        {
        case 0:
            break;
        case 1:
            break;
        case 2:
            glUniform1i(glGetUniformLocation(*currentShader, "numElements"), currentModel->numFaces);
            break;
        }
        */
        //Uniforms
        glm::mat4 lightRotate = glm::rotate(glm::mat4(1), glm::radians(lightDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
        lightPos = glm::vec3(lightRotate * glm::vec4(lightPosInit, 0.0f));

        //opengl matrice transforms are applied from the right side. (last first)
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -1.0f));
        model = glm::scale(model, glm::vec3(modelSize, modelSize, modelSize));
        //model = glm::rotate(model, glm::radians(yDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
        //model = glm::rotate(model, glm::radians(xDegrees), glm::vec3(1.0f, 0.0f, 0.0f));

        model = model * mouseRotation;
        model *= scrollModel;
        model = glm::scale(model, glm::vec3(currentModel->modelScaleFactor));
        model = glm::translate(model, (-1.0f * currentModel->center));
        glm::mat4 view = glm::lookAt(viewPos, viewDir, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);


        glUniformMatrix4fv(glGetUniformLocation(*currentShader, "model"), 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(*currentShader, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(*currentShader, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniform3f(glGetUniformLocation(*currentShader, "viewPosition"), viewPos.x, viewPos.y, viewPos.z);

        glUniform3f(glGetUniformLocation(*currentShader, "lightPosition"), lightPos.x, lightPos.y, lightPos.z);

        currentModel->render(*currentShader);

        if (drawEdges) {
            glUniformMatrix4fv(glGetUniformLocation(*elementEdgesShader, "model"), 1, GL_FALSE, &model[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(*elementEdgesShader, "view"), 1, GL_FALSE, &view[0][0]);
            glUniformMatrix4fv(glGetUniformLocation(*elementEdgesShader, "projection"), 1, GL_FALSE, &projection[0][0]);
            glUniform3f(glGetUniformLocation(*elementEdgesShader, "lineColor"), lineColor.x, lineColor.y, lineColor.z);
            currentModel->render(*elementEdgesShader);
        }

        glUseProgram(0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Delete ImGUI instances
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}


void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    //return if using IMGUI
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    //static variables persists across all function calls and doesn't go out of range.
    static double last_xpos = xpos;
    static double last_ypos = ypos;
    double dx = xpos - last_xpos;
    double dy = ypos - last_ypos;
    last_xpos = xpos;
    last_ypos = ypos;

    glm::vec3 cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS)
    {
        //rotation
        glm::quat rotY = glm::angleAxis(0.0025f * static_cast<float>(dy), cameraRight);
        glm::quat rotX = glm::angleAxis(0.0025f * static_cast<float>(dx), cameraUp);
        glm::quat rotation = rotX * rotY;

        //!!! mouseRotation *= glm::mat4_cast(rotation) ; -> mouseRotation = mouseRotation*glm::mat4_cast(rotation); which will cause rotations to apply in model space!!!
        mouseRotation = glm::mat4_cast(rotation) * mouseRotation;
        //Next time I'll add a camera class.
    }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }
    float scale_factor = 1.0f + yoffset * 0.1f;
    scrollModel = glm::scale(scrollModel, glm::vec3(scale_factor));

};