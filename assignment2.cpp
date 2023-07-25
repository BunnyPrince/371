//#include <algorithm>
//#include <iostream>
//#include <vector>
//
#define GLEW_STATIC 1  // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>  // Include GLEW - OpenGL Extension Wrangler
#include <GLFW/glfw3.h>  // GLFW provides a cross-platform interface for creating a graphical context,
// initializing OpenGL and binding inputs

#include <glm/glm.hpp>  // GLM is an optimized math library with syntax to similar to OpenGL Shading Language
#include <glm/gtc/matrix_transform.hpp>  // include this to create transformation matrices
#include <glm/gtc/type_ptr.hpp>

#include "OBJloader.h"    //For loading .obj files
#include "OBJloaderV2.h"  //For loading .obj files using a polygon list format
#include "shaderloader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;
using namespace std;

// window dimensions
const unsigned int DEPTH_MAP_TEXTURE_SIZE = 1024;
const float triangleVertices[] = {
   -0.6f, -0.34f, 0.0f, // Bottom-left 
     0.6f, -0.34f, 0.0f, // Bottom-right 
     0.0f,  0.5f, 0.0f  // Top 
};
vec3 computeCameraLookAt(double& lastMousePosX, double& lastMousePosY, float dt);

GLuint setupModelVBO(string path, int& vertexCount);

// Sets up a model using an Element Buffer Object to refer to vertex data
GLuint setupModelEBO(string path, int& vertexCount);

int LoadTexture(const char* filename,
    int& spriteWidth);

// shader variable setters
void SetUniformMat4(GLuint shader_id, const char* uniform_name,
    mat4 uniform_value) {
    glUseProgram(shader_id);
    glUniformMatrix4fv(glGetUniformLocation(shader_id, uniform_name), 1, GL_FALSE,
        &uniform_value[0][0]);
}

void SetUniformVec3(GLuint shader_id, const char* uniform_name,
    vec3 uniform_value) {
    glUseProgram(shader_id);
    glUniform3fv(glGetUniformLocation(shader_id, uniform_name), 1,
        value_ptr(uniform_value));
}




template <class T>
void SetUniform1Value(GLuint shader_id, const char* uniform_name,
    T uniform_value) {
    glUseProgram(shader_id);
    glUniform1i(glGetUniformLocation(shader_id, uniform_name), uniform_value);
    glUseProgram(0);
}
GLFWwindow* window = nullptr;
bool InitContext(GLint WIDTH, GLint HEIGHT);

GLuint LoadShaders(std::string vertex_shader_path,
    std::string fragment_shader_path) {
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_shader_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::string Line = "";
        while (getline(VertexShaderStream, Line)) VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else {
        printf(
            "Impossible to open %s. Are you in the right directory ? Don't forget "
            "to read the FAQ !\n",
            vertex_shader_path.c_str());
        getchar();
        exit(-1);
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_shader_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::string Line = "";
        while (getline(FragmentShaderStream, Line))
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_shader_path.c_str());
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, nullptr);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, nullptr,
            &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_shader_path.c_str());
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, nullptr);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, nullptr,
            &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr,
            &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}
//creating the big cube 


void creatCube(mat4 modelMatrices, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type) {


    SetUniformMat4(shaderScene, "model_matrix", modelMatrices);

    SetUniformMat4(shaderShadow, "transform_in_light_space", lightProjMatrix * lightViewMatrix * modelMatrices);

    // Set the view matrix for first person camera and send to both shaders
    viewMatrix = lookAt(cameraPosition, cameraPosition + cameraLookAt, cameraUp);
    SetUniformMat4(shaderScene, "view_matrix", viewMatrix);

    // Set view position on scene shader
    SetUniformVec3(shaderScene, "view_position", cameraPosition);




    // Render shadow in 2 passes: 1- Render depth map, 2- Render scene
    // 1- Render shadow map:
    // a- use program for shadows
    // b- resize window coordinates to fix depth map output size
    // c- bind depth map framebuffer to output the depth values
    {
        // Use proper shader
        glUseProgram(shaderShadow);
        // Use proper image output size
        glViewport(0, 0, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE);
        // Bind depth map texture as output framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
        // Bind geometry
        glBindVertexArray(activeVAO);
        // Draw geometry
        //glDrawElements(GL_TRIANGLES, activeVertices, GL_UNSIGNED_INT, 0);
        
        if (type == 0) {
            glDrawArrays(GL_TRIANGLES, 0, activeVertices);
        }
        if (type == 1) {
            glBindVertexArray(activeVertices);
            glDrawArrays(GL_LINE_STRIP, 0, activeVertices);
        }
        if (type == 2) {
            glBindVertexArray(activeVertices);
            glDrawArrays(GL_POINTS, 0, activeVertices);
        }
        
        // Unbind geometry
        glBindVertexArray(0);
    }

    ////// 2- Render scene: a- bind the default framebuffer and b- just render like
    ////// what we do normally
    {
        // Use proper shader
        glUseProgram(shaderScene);
        // Use proper image output size
        // Side note: we get the size from the framebuffer instead of using WIDTH
        // and HEIGHT because of a bug with highDPI displays
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        // Bind screen as output framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // Bind depth map texture: not needed, by default it is active
        // glActiveTexture(GL_TEXTURE0);
        // Bind geometry
        glBindVertexArray(activeVAO);
        // Draw geometry
        //glDrawElements(GL_TRIANGLES, activeVertices, GL_UNSIGNED_INT, 0);
        if (type == 0) {
            glDrawArrays(GL_TRIANGLES, 0, activeVertices);
        }
        if (type == 1) {
            glBindVertexArray(activeVertices);
            glDrawArrays(GL_LINE_STRIP, 0, activeVertices);
        }
        if (type == 2) {
            glBindVertexArray(activeVertices);
            glDrawArrays(GL_POINTS, 0, activeVertices);
        }
        // Unbind geometry
        glBindVertexArray(0);
    }
}

void drawLowerArm(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type) {

    mat4 modelMatrices =  
        glm::translate(mat4(1.0f), vec3(position.x-0.17, position.y-0.5, position.z)) *
        rotate(mat4(1.0f), radians(320.0f), vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(mat4(1.0f), vec3(0.1f,0.5f,0.1f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
}

void drawUpperArm(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type) {

    mat4 modelMatrices = 
        glm::translate(mat4(1.0f), vec3(position.x, position.y, position.z)) *
        glm::scale(mat4(1.0f), vec3(0.1f, 0.5f, 0.1f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
}

void drawRacket(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices,int type) {

    // the handle
    mat4 modelMatrices =  
        glm::translate(mat4(1.0f), vec3(position.x , position.y + 0.7f, position.z)) *
        glm::scale(mat4(1.0f), vec3(0.05f, 0.7f, 0.05f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // the left bar that connect the handle '\'
    modelMatrices = 
        glm::translate(mat4(1.0f), vec3(position.x-0.2, position.y + 1.4f, position.z)) *
        rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(mat4(1.0f), vec3(0.05f, 0.4f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // the right bar the connect the handle '/'
    modelMatrices =  
        glm::translate(mat4(1.0f), vec3(position.x + 0.2, position.y + 1.4f, position.z)) *
        rotate(glm::mat4(1.0f), glm::radians(315.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(mat4(1.0f), vec3(0.05f, 0.4f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // the bar that connect both '\ /'
     modelMatrices = 
        glm::translate(mat4(1.0f), vec3(position.x , position.y + 1.45f, position.z)) *
        glm::scale(mat4(1.0f), vec3(0.35f, 0.05f, 0.05f));
     woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // side bar on top of '\'
    modelMatrices =  
        glm::translate(mat4(1.0f), vec3(position.x-0.4, position.y + 2.0f, position.z)) *
        glm::scale(mat4(1.0f), vec3(0.05f, 0.6f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // side bar on top of '/'
    modelMatrices =
        glm::translate(mat4(1.0f), vec3(position.x + 0.4, position.y + 2.0f, position.z)) *
        glm::scale(mat4(1.0f), vec3(0.05f, 0.6f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // lower small '/' on left top 
    modelMatrices =
        glm::translate(mat4(1.0f), vec3(position.x - 0.32, position.y + 2.5f, position.z)) *
        rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // lower small '\' on right top 
    modelMatrices =
        glm::translate(mat4(1.0f), vec3(position.x + 0.32, position.y + 2.5f, position.z)) *
        rotate(glm::mat4(1.0f), glm::radians(315.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // upper small '/' on left top 
    modelMatrices =
        glm::translate(mat4(1.0f), vec3(position.x - 0.2, position.y + 2.6f, position.z)) *
        rotate(glm::mat4(1.0f), glm::radians(25.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // upper small '\' on right top 
    modelMatrices =
        glm::translate(mat4(1.0f), vec3(position.x + 0.2, position.y + 2.6f, position.z)) *
        rotate(glm::mat4(1.0f), glm::radians(335.0f), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // upper small '-'  
    modelMatrices =
        glm::translate(mat4(1.0f), vec3(position.x, position.y + 2.65f, position.z)) *
        glm::scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // | net 
    for (int i = 0; i < 14; i++) {

        modelMatrices =
            glm::translate(mat4(1.0f), vec3(position.x-0.3 +(0.05*i), position.y + 2.0f, position.z)) *
            glm::scale(mat4(1.0f), vec3(0.01f, 0.8f, 0.01f));
        woldMatrices = groupMatrix * modelMatrices;
        creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    }

    // - net
    for (int i = 0; i < 23; i++) {
        modelMatrices =
            glm::translate(mat4(1.0f), vec3(position.x, position.y + 1.48f + (0.05*i), position.z)) *
            glm::scale(mat4(1.0f), vec3(0.48f, 0.01f, 0.01f));
        woldMatrices = groupMatrix * modelMatrices;
        creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    }
}

void drawFullModel(mat4 groupMatrix,vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices,int type) {

    drawLowerArm(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    drawUpperArm(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    drawRacket(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
}

vec3 getRandomGridPosition()
{
    float minX = -100;
    float maxX = 100;
    float minZ = -100;
    float maxZ = 100;

    float x = minX + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));
    float z = minZ + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxZ - minZ)));

    return vec3(x, 0.0f, z);
}


int main(int argc, char* argv[]) {

    GLuint WIDTH = 1024, HEIGHT = 768;
    if (!InitContext(WIDTH, HEIGHT)) return -1;

    // Black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#if defined(PLATFORM_OSX)
    std::string shaderPathPrefix = "Shaders/";
#else
    std::string shaderPathPrefix = "Assets/Shaders/";
#endif

    GLuint shaderScene = loadSHADER(shaderPathPrefix + "scene_vertex.glsl",
        shaderPathPrefix + "scene_fragment.glsl");

    GLuint shaderShadow = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl",
        shaderPathPrefix + "shadow_fragment.glsl");


    GLuint shaderTexture = LoadShaders(shaderPathPrefix + "Texture.vertexshader",
        shaderPathPrefix + "Texture.fragmentshader");

    GLuint shaderColor = LoadShaders(shaderPathPrefix + "SolidColor.vertexshader",
        shaderPathPrefix + "SolidColor.fragmentshader");

    int textureWidth = 0;
    GLuint particleTexture = LoadTexture(
        "/Users/Janan/Comp371/assignment1/assignment1/assets/textures/Particle.png", textureWidth);

    // Setup models
#if defined(PLATFORM_OSX)
    string heraclesPath = "Models/heracles.obj";
    string cubePath = "Models/cube.obj";
    string ballPath = "Models/plane.obj";
#else
    string heraclesPath = "Assets/Models/heracles.obj";
    string cubePath = "Assets/Models/cube.obj";
    string ballPath = "Assets/Models/plane.obj";
#endif

    int cubeVertices;
    GLuint cubeVAO = setupModelVBO(cubePath, cubeVertices);

    int activeVertices = cubeVertices;
    GLuint activeVAO = cubeVAO;

    int ballVertices;
    GLuint ballVAO = setupModelVBO(ballPath, ballVertices);

    int activeBallVertices = ballVertices;
    GLuint activeBallVAO = ballVAO;


    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    // Bind the VAO and VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Setup texture and framebuffer for creating shadow map

    // Dimensions of the shadow texture, which should cover the viewport window
    // size and shouldn't be oversized and waste resources

    // Variable storing index to texture used for shadow mapping
    GLuint depth_map_texture;
    // Get the texture
    glGenTextures(1, &depth_map_texture);
    // Bind the texture so the next glTex calls affect it
    glBindTexture(GL_TEXTURE_2D, depth_map_texture);
    // Create the texture and specify it's attributes, including widthn height,
    // components (only depth is stored, no color information)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DEPTH_MAP_TEXTURE_SIZE,
        DEPTH_MAP_TEXTURE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    // Set texture sampler parameters.
    // The two calls below tell the texture sampler inside the shader how to
    // upsample and downsample the texture. Here we choose the nearest filtering
    // option, which means we just use the value of the closest pixel to the
    // chosen image coordinate.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // The two calls below tell the texture sampler inside the shader how it
    // should deal with texture coordinates outside of the [0, 1] range. Here we
    // decide to just tile the image.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Variable storing index to framebuffer used for shadow mapping
    GLuint depth_map_fbo;  // fbo: framebuffer object
    // Get the framebuffer
    glGenFramebuffers(1, &depth_map_fbo);
    // Bind the framebuffer so the next glFramebuffer calls affect it
    glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
    // Attach the depth map texture to the depth map framebuffer
    // glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
    // depth_map_texture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
        depth_map_texture, 0);
    glDrawBuffer(GL_NONE);  // disable rendering colors, only write depth values

    // Camera parameters for view transform
    vec3 cameraPosition(0.0f, 0.5f, 6.0f);
    vec3 cameraLookAt(0.0f, 0.0f, -1.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);

    // Other camera parameters
    float cameraSpeed = 5.0f;
    float cameraFastSpeed = 3 * cameraSpeed;
    vec3 modelPosition(0.0f, -1.0f, 0.0f);

    float angleHorizontal = 0;
    float angleVertical = 0;
    float scal = 1.0f;
    // For spinning model
    float spinningAngle = 0.0f;
    int type = 0; // the type of model
    int arm = 0;
    bool mov = true;

    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix =
        glm::perspective(70.0f,                  // field of view in degrees
            WIDTH * 1.0f / HEIGHT,  // aspect ratio
            0.01f, 800.0f);         // near and far (near > 0)

    // Set initial view matrix on both shaders
    mat4 viewMatrix = lookAt(cameraPosition,                 // eye
        cameraPosition + cameraLookAt,  // center
        cameraUp);                      // up

    // Set projection matrix on both shaders
    SetUniformMat4(shaderScene, "projection_matrix", projectionMatrix);

    // Set view matrix on both shaders
    SetUniformMat4(shaderScene, "view_matrix", viewMatrix);

    float lightAngleOuter = radians(30.0f);
    float lightAngleInner = radians(20.0f);
    // Set light cutoff angles on scene shader
    SetUniform1Value(shaderScene, "light_cutoff_inner", cos(lightAngleInner));
    SetUniform1Value(shaderScene, "light_cutoff_outer", cos(lightAngleOuter));

    // Set light color on scene shader
    SetUniformVec3(shaderScene, "light_color", vec3(1));

    // Set object color on scene shader
    SetUniformVec3(shaderScene, "object_color", vec3(1));

    // For frame time
    float lastFrameTime = glfwGetTime();
    int lastMouseLeftState = GLFW_RELEASE;
    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

    // Other OpenGL states to set once
    // Enable Backface culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // Entering Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Frame time calculation
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime = glfwGetTime();

        glUseProgram(shaderTexture);

        GLint colorLocation = glGetUniformLocation(shaderTexture, "color");
        // light parameters
        float phi = glfwGetTime() * 0.5f * 3.14f;
        vec3 lightPosition = vec3(0.0f, 30.0f, 0.0f); // the location of the light in 3D space: fixed position
        vec3(cosf(phi) * cosf(phi), sinf(phi),
            -cosf(phi) * sinf(phi)) *
            5.0f;  // variable position

        vec3 lightFocus(0, 0, -1);  // the point in 3D space the light "looks" at
        vec3 lightDirection = normalize(lightFocus - lightPosition);

        float lightNearPlane = 0.01f;
        float lightFarPlane = 400.0f;

        mat4 lightProjMatrix = frustum(-1.0f, 1.0f, -(float)DEPTH_MAP_TEXTURE_SIZE, 1.0f, lightNearPlane, lightFarPlane);
        //perspective(50.0f, (float)DEPTH_MAP_TEXTURE_SIZE / (float)DEPTH_MAP_TEXTURE_SIZE, lightNearPlane, lightFarPlane);
        mat4 lightViewMatrix = lookAt(lightPosition, lightFocus, vec3(0, 1, 0));

        SetUniformMat4(shaderScene, "light_proj_view_matrix", lightProjMatrix * lightViewMatrix);
        SetUniform1Value(shaderScene, "light_near_plane", lightNearPlane);
        SetUniform1Value(shaderScene, "light_far_plane", lightFarPlane);
        SetUniformVec3(shaderScene, "light_position", lightPosition);
        SetUniformVec3(shaderScene, "light_direction", lightDirection);



        int width, height;
        glfwGetFramebufferSize(window, &width, &height);


        if (WIDTH != width|| HEIGHT != height) {
            glViewport(0, 0, width, height);
        }

        WIDTH = width;
        HEIGHT = height;

        // Spinning model rotation animation
        spinningAngle += 45.0f * dt;  // This is equivalent to 45 degrees per second

        glm::mat4 modelMatrices[2];


        // Set model matrix and send to both shaders
       mat4 groundMatrices =  // mat4(1.0f);
            glm::translate(mat4(1.0f), vec3(0.0f, -3.0f, 0.0f)) *
            glm::scale(mat4(1.0f), vec3(100.0f,0.01f,100.0f));

        SetUniformMat4(shaderScene, "model_matrix", groundMatrices);

        SetUniformMat4(shaderShadow, "transform_in_light_space", lightProjMatrix * lightViewMatrix * groundMatrices);




        // Render shadow in 2 passes: 1- Render depth map, 2- Render scene
        // 1- Render shadow map:
        // a- use program for shadows
        // b- resize window coordinates to fix depth map output size
        // c- bind depth map framebuffer to output the depth values
        {
            // Use proper shader
            glUseProgram(shaderShadow);
            // Use proper image output size
            glViewport(0, 0, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE);
            // Bind depth map texture as output framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
            // Clear depth data on the framebuffer
            glClear(GL_DEPTH_BUFFER_BIT);
            // Bind geometry
            glBindVertexArray(activeVAO);
            // Draw geometry
            //glDrawElements(GL_TRIANGLES, activeVertices, GL_UNSIGNED_INT, 0);
            glDrawArrays(GL_TRIANGLES, 0, activeVertices);
            // Unbind geometry
            glBindVertexArray(0);
        }

        ////// 2- Render scene: a- bind the default framebuffer and b- just render like
        ////// what we do normally
        {
            // Use proper shader
            glUseProgram(shaderScene);
            // Use proper image output size
            // Side note: we get the size from the framebuffer instead of using WIDTH
            // and HEIGHT because of a bug with highDPI displays
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            // Bind screen as output framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Clear color and depth data on framebuffer
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Bind depth map texture: not needed, by default it is active
            // glActiveTexture(GL_TEXTURE0);
            // Bind geometry
            glBindVertexArray(activeVAO);
            // Draw geometry
            //glDrawElements(GL_TRIANGLES, activeVertices, GL_UNSIGNED_INT, 0);
            
            glDrawArrays(GL_TRIANGLES, 0, activeVertices);
            // Unbind geometry
            glBindVertexArray(0);
        }

        mat4 ballMatrices =  // mat4(1.0f);
            glm::translate(mat4(1.0f), vec3(-1.0f, 1.0f, 0.0f)) *
            glm::scale(mat4(1.0f), vec3(0.06f));

        SetUniformMat4(shaderScene, "model_matrix", ballMatrices);
        SetUniformVec3(shaderTexture, "v_color", vec3(1.0f, 0.0f, 0.0f));

        SetUniformMat4(shaderShadow, "transform_in_light_space", lightProjMatrix * lightViewMatrix * ballMatrices);


        // Render shadow in 2 passes: 1- Render depth map, 2- Render scene
        // 1- Render shadow map:
        // a- use program for shadows
        // b- resize window coordinates to fix depth map output size
        // c- bind depth map framebuffer to output the depth values
        {
            // Use proper shader
            glUseProgram(shaderShadow);
            // Use proper image output size
            glViewport(0, 0, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE);
            // Bind depth map texture as output framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
            // Bind geometry
            glBindVertexArray(activeBallVAO);
            // Draw geometry
            //glDrawElements(GL_TRIANGLES, activeVertices, GL_UNSIGNED_INT, 0);
            glDrawArrays(GL_TRIANGLES, 0, activeBallVertices);
            // Unbind geometry
            glBindVertexArray(0);
        }

        ////// 2- Render scene: a- bind the default framebuffer and b- just render like
        ////// what we do normally
        {
            // Use proper shader
            glUseProgram(shaderScene);
            // Use proper image output size
            // Side note: we get the size from the framebuffer instead of using WIDTH
            // and HEIGHT because of a bug with highDPI displays
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            glViewport(0, 0, width, height);
            // Bind screen as output framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // Bind depth map texture: not needed, by default it is active
            // glActiveTexture(GL_TEXTURE0);
            // Bind geometry
            glBindVertexArray(activeBallVAO);
            // Draw geometry
            //glDrawElements(GL_TRIANGLES, activeVertices, GL_UNSIGNED_INT, 0);


            glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f); // Red 
            glDrawArrays(GL_TRIANGLES, 0, activeBallVertices);
            // Unbind geometry
            glBindVertexArray(0);
        }
        
        // Everything below here is the solution for Lab02 - Moving camera exercise
        // We'll change this to be a first or third person camera
        bool fastCam = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        float currentCameraSpeed = (fastCam) ? cameraFastSpeed : cameraSpeed;

        cameraLookAt = computeCameraLookAt(lastMousePosX, lastMousePosY, dt);

        vec3 cameraSideVector = glm::cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));

        glm::normalize(cameraSideVector);

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            cameraPosition += cameraSideVector * dt * currentCameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            cameraPosition -= cameraSideVector * dt * currentCameraSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            cameraPosition += cameraLookAt * dt * currentCameraSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            cameraPosition -= cameraLookAt * dt * currentCameraSpeed;
        }

        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);


        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            modelPosition = getRandomGridPosition();
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ) {
            vec3 newPos(modelPosition.x, modelPosition.y + 0.1, modelPosition.z);
            modelPosition = newPos;
            mov = true;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ) {
            vec3 newPos(modelPosition.x, modelPosition.y - 0.1, modelPosition.z);
            modelPosition = newPos;
            mov = true;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            vec3 newPos(modelPosition.x + 0.1, modelPosition.y, modelPosition.z);
            modelPosition = newPos;
            mov = true;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            vec3 newPos(modelPosition.x - 0.1, modelPosition.y, modelPosition.z);
            modelPosition = newPos;
            mov = true;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            angleHorizontal += 1;
            mov = false;
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            angleHorizontal -= 1;
            mov = false;
        }

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            type = 0;
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            type = 1;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            type = 2;
        }


        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            angleVertical -= 1;
            mov = false;
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            angleVertical += 1;
            mov = false;
        }

        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            arm = 0;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            arm = 1;
        }


        mat4 translation = mat4(1.0f);
        mat4 translation2 = translate(mat4(1.0f), modelPosition);

        if (arm == 0) {
            vec3 position(modelPosition.x - 0.17, modelPosition.y - 0.5, modelPosition.z);
            translation = translate(mat4(1.0f), -position);
        }

        if (arm == 1) {
            vec3 position(modelPosition.x, modelPosition.y, modelPosition.z);
            translation = translate(mat4(1.0f), -position);
        }

        glm::mat4 groupMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angleVertical), glm::vec3(1.0f, 0.0f, 0.0f)) * 
            glm::scale(glm::mat4(1.0f), glm::vec3(1.0f,1.0f,1.0f));


        glm::mat4 groupMatrix2 = 
            glm::rotate(glm::mat4(1.0f), glm::radians(angleHorizontal), glm::vec3(0.0f, 1.0f, 0.0f)) *
            glm::scale(glm::mat4(1.0f), glm::vec3(scal, scal, scal));

        mat4 total = mat4(1.0f);

        if (mov) {
            total = groupMatrix * translation * groupMatrix2 * translation2;
        }
        else {

            total = groupMatrix * translation * groupMatrix2;
        }

        drawFullModel(total, modelPosition, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


        glfwSwapBuffers(window);
        glfwPollEvents();

        

        
    }

    glfwTerminate();

    return 0;
}

vec3 computeCameraLookAt(double& lastMousePosX, double& lastMousePosY, float dt) {
    // - Calculate mouse motion dx and dy
  // - Update camera horizontal and vertical angle
    double mousePosX, mousePosY;
    glfwGetCursorPos(window, &mousePosX, &mousePosY);

    double dx = mousePosX - lastMousePosX;
    double dy = mousePosY - lastMousePosY;

    lastMousePosX = mousePosX;
    lastMousePosY = mousePosY;

    // Convert to spherical coordinates
    const float cameraAngularSpeed = 60.0f;
    static float cameraHorizontalAngle = 90.0f;
    static float cameraVerticalAngle = 0.0f;
    cameraHorizontalAngle -= dx * cameraAngularSpeed * dt;
    cameraVerticalAngle -= dy * cameraAngularSpeed * dt;

    // Clamp vertical angle to [-85, 85] degrees
    cameraVerticalAngle =
        glm::max(-85.0f, glm::min(85.0f, cameraVerticalAngle));

    float theta = radians(cameraHorizontalAngle);
    float phi = radians(cameraVerticalAngle);

    return vec3(cosf(phi) * cosf(theta), sinf(phi), -cosf(phi) * sinf(theta));
}

GLuint setupModelVBO(string path, int& vertexCount) {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> colors{6, vec3(1.0f, 0.0f, 0.0f)};
    std::vector<glm::vec2> UVs;

    // read the vertex data from the model's OBJ file
    loadOBJ(path.c_str(), vertices, normals, UVs);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);  // Becomes active VAO
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and
    // attribute pointer(s).

    // Vertex VBO setup
    GLuint vertices_VBO;
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
        &vertices.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Normals VBO setup
    GLuint normals_VBO;
    glGenBuffers(1, &normals_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3),
        &normals.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(1);

    // UVs VBO setup
    GLuint uvs_VBO;
    glGenBuffers(1, &uvs_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(),
        GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(2);


    // Setup Colors

    GLuint color_VBO;
    glGenBuffers(1, &color_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, color_VBO);  // colors
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(vec3), colors.data(),
               GL_STATIC_DRAW);

  // 3rd attribute buffer : vertex color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent
    // strange bugs, as we are using multiple VAOs)
    vertexCount = vertices.size();
    return VAO;
}

GLuint setupModelEBO(string path, int& vertexCount) {
    vector<int> vertexIndices;
    // The contiguous sets of three indices of vertices, normals and UVs, used to
    // make a triangle
    vector<glm::vec3> vertices;
    vector<glm::vec3> normals;
    vector<glm::vec2> UVs;

    // read the vertices from the cube.obj file
    // We won't be needing the normals or UVs for this program
    loadOBJ2(path.c_str(), vertexIndices, vertices, normals, UVs);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);  // Becomes active VAO
    // Bind the Vertex Array Object first, then bind and set vertex buffer(s) and
    // attribute pointer(s).

    // Vertex VBO setup
    GLuint vertices_VBO;
    glGenBuffers(1, &vertices_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
        &vertices.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Normals VBO setup
    GLuint normals_VBO;
    glGenBuffers(1, &normals_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3),
        &normals.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(1);

    // UVs VBO setup
    GLuint uvs_VBO;
    glGenBuffers(1, &uvs_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(glm::vec2), &UVs.front(),
        GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(2);

    // EBO setup
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexIndices.size() * sizeof(int),
        &vertexIndices.front(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent
    // strange bugs), remember: do NOT unbind the EBO, keep it bound to this VAO
    vertexCount = vertexIndices.size();
    return VAO;
}


bool InitContext(GLint WIDTH, GLint HEIGHT) {
    // Initialize GLFW and OpenGL version
    glfwInit();

#if defined(PLATFORM_OSX)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // On windows, we set OpenGL version to 2.1, to support more hardware
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#endif

    // Create Window and rendering context using GLFW, resolution is 800x600
    window = glfwCreateWindow(WIDTH, HEIGHT, "Comp371 - Lab 08", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true;  // Needed for core profile
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to create GLEW" << std::endl;
        glfwTerminate();
        return false;
    }

    return true;
}


int LoadTexture(const char* filename,int& spriteWidth) {

    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    assert(textureId != 0);

    glBindTexture(GL_TEXTURE_2D, textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    int width, height, nrChannels;

    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Error::Texture could not load texture file:" << filename
            << std::endl;
        return 0;
    }

    spriteWidth = width;

    GLenum format = 0;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
        GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
}