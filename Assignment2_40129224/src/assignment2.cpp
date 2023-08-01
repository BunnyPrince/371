// alot of part in this assignment was reused from  labs 

#define GLEW_STATIC 1  // This allows linking with Static Library on Windows, without DLL
#include <GL/glew.h>  
#include <GLFW/glfw3.h>  
#include <glm/glm.hpp>  
#include <glm/gtc/matrix_transform.hpp> 
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


void SetUniform2D(GLuint shader_id, const char* uniform_name,
    GLuint  uniform_value)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, uniform_value);

    // Set the texture unit index for the sampler uniform
    GLint textureUnitIndex = 0; // The same texture unit index used above
    glUniform1i(glGetUniformLocation(shader_id, "uniform_name"), textureUnitIndex);
}
GLuint loadTexture(const char* filename)
{
    // Step1 Create and bind textures
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    assert(textureId != 0);


    glBindTexture(GL_TEXTURE_2D, textureId);

    // Step2 Set filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Step3 Load Textures with dimension data
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
        return 0;
    }

    // Step4 Upload the texture to the PU
    GLenum format = 0;
    if (nrChannels == 1)
        format = GL_RED;
    else if (nrChannels == 3)
        format = GL_RGB;
    else if (nrChannels == 4)
        format = GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
        0, format, GL_UNSIGNED_BYTE, data);

    // Step5 Free resources
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureId;
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

// drawing the lower arm 
void drawLowerArm(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type,  int texture) {

    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(position.x - 0.17, position.y - 0.5, position.z)) *
        rotate(mat4(1.0f), radians(320.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(0.1f, 0.5f, 0.1f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    if (texture == 0) {
        vec3 objectColor(0.2f, 0.2f, 0.2f);
        SetUniformVec3(shaderScene, "object_color", objectColor);
    }
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
}

// drawing the upper arm
void drawUpperArm(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture) {

    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y, position.z)) *
        scale(mat4(1.0f), vec3(0.1f, 0.5f, 0.1f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    if (texture == 0) {
        vec3 objectColor(0.3f, 0.3f, 0.3f);
        SetUniformVec3(shaderScene, "object_color", objectColor);
    }
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
}


//drawing the rackets
void drawRacket(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture) {
    

    //changing the color of the part of the racket red
    if (texture == 0) {
        vec3 objectColor(1.0f, 0.0f, 0.0f);
        SetUniformVec3(shaderScene, "object_color", objectColor);
    }

    // the handle
    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 0.7f, position.z)) *
        scale(mat4(1.0f), vec3(0.05f, 0.7f, 0.05f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // the bar that connect both '\ /'
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 1.45f, position.z)) *
        scale(mat4(1.0f), vec3(0.35f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // side bar on top of '\'
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x - 0.4, position.y + 2.0f, position.z)) *
        scale(mat4(1.0f), vec3(0.05f, 0.6f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // side bar on top of '/'
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x + 0.4, position.y + 2.0f, position.z)) *
        scale(mat4(1.0f), vec3(0.05f, 0.6f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // upper small '/' on left top 
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x - 0.2, position.y + 2.6f, position.z)) *
        rotate(mat4(1.0f), radians(25.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // upper small '\' on right top 
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x + 0.2, position.y + 2.6f, position.z)) *
        rotate(mat4(1.0f), radians(335.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    //changing the color of the part of the racket white
    if (texture == 0) {
        vec3 objectColor2(1.0f, 1.0f, 1.0f);
        SetUniformVec3(shaderScene, "object_color", objectColor2);
    }
    // the left bar that connect the handle '\'
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x - 0.2, position.y + 1.4f, position.z)) *
        rotate(mat4(1.0f), radians(45.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(0.05f, 0.4f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // the right bar the connect the handle '/'
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x + 0.2, position.y + 1.4f, position.z)) *
        rotate(mat4(1.0f), radians(315.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(0.05f, 0.4f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // lower small '/' on left top 
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x - 0.32, position.y + 2.5f, position.z)) *
        rotate(mat4(1.0f), radians(45.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // lower small '\' on right top 
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x + 0.32, position.y + 2.5f, position.z)) *
        rotate(mat4(1.0f), radians(315.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // upper small '-'  
    modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 2.65f, position.z)) *
        scale(mat4(1.0f), vec3(0.15f, 0.05f, 0.05f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    //changing the color of the net blue
    if (texture == 0) {
        vec3 objectColor3(0.0f, 0.0f, 1.0f);
        SetUniformVec3(shaderScene, "object_color", objectColor3);
    }

    // | net 
    for (int i = 0; i < 14; i++) {

        modelMatrices =
            translate(mat4(1.0f), vec3(position.x - 0.3 + (0.05 * i), position.y + 2.0f, position.z)) *
            scale(mat4(1.0f), vec3(0.01f, 0.8f, 0.01f));
        woldMatrices = groupMatrix * modelMatrices;
        creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    }

    // - net
    for (int i = 0; i < 23; i++) {
        modelMatrices =
            translate(mat4(1.0f), vec3(position.x, position.y + 1.48f + (0.05 * i), position.z)) *
            scale(mat4(1.0f), vec3(0.48f, 0.01f, 0.01f));
        woldMatrices = groupMatrix * modelMatrices;
        creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    }
}

void drawAxies(vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture) {

    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 0.35f, position.z + 0.0f)) *
        scale(mat4(1.0f), vec3(0.02f, 0.5f, 0.02f));
    if (texture == 0) {
        vec3 objectColor(1.0f, 0.0f, 0.0f);
        SetUniformVec3(shaderScene, "object_color", objectColor);
    }
    creatCube(modelMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


     modelMatrices =
        translate(mat4(1.0f), vec3(position.x + 0.35f, position.y + 0.0f, position.z + 0.0f)) *
        scale(mat4(1.0f), vec3(0.5f, 0.02f, 0.02f));
    if (texture == 0) {
        vec3 objectColor(0.0f, 0.0f, 1.0f);
        SetUniformVec3(shaderScene, "object_color", objectColor);
    }
    creatCube(modelMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

     modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 0.0f, position.z + 0.45f)) *
        scale(mat4(1.0f), vec3(0.02f, 0.02f, 0.5f));
    if (texture == 0) {
        vec3 objectColor(0.0f, 1.0f, 0.0f);
        SetUniformVec3(shaderScene, "object_color", objectColor);
    }
    creatCube(modelMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

}

// drawing everything at the same time
void drawFullModel(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture) {

    drawLowerArm(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture);

    drawUpperArm(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture);

    drawRacket(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture);
}

// getting random position to move
vec3 getRandomGridPosition()
{
    float minX = -50;
    float maxX = 50;
    float minZ = -50;
    float maxZ = 50;

    float x = minX + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));
    float z = minZ + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxZ - minZ)));

    return vec3(x, -1.0f, z);
}


int main(int argc, char* argv[]) {

    GLuint WIDTH = 1024, HEIGHT = 768;
    if (!InitContext(WIDTH, HEIGHT)) return -1;

    //Background color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

#if defined(PLATFORM_OSX)
    std::string shaderPathPrefix = "Shaders/";
#else
    std::string shaderPathPrefix = "Assets/Shaders/";
#endif

    GLuint shaderScene = loadSHADER(shaderPathPrefix + "scene_vertex.glsl",
        shaderPathPrefix + "scene_fragment.glsl");

    GLuint shaderShadow = loadSHADER(shaderPathPrefix + "shadow_vertex.glsl",
        shaderPathPrefix + "shadow_fragment.glsl");


    // Setup models
#if defined(PLATFORM_OSX)
    string cubePath = "Models/cube.obj";
    string ballPath = "Models/plane.obj";
#else
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
    vec3 ballPosition(-1.0f, 1.0f, 0.0f);
    vec3 axiesPosition(1.0f, 0.5f, 0.0f);


    vec3 modelPositionB = modelPosition;
    vec3 ballPositionB = ballPosition;

    float angleHorizontal = 0;
    float angleVertical = 0;
    float scal = 1.0f;

    float scalBall = 0.0f;
    // For spinning model
    float spinningAngle = 0.0f;
    int type = 0; // the type of model
    int light = 0; // the type of light
    int arm = 0; // what arm
    bool mov = true;
    int shadows = 0;
    int texture = 0;

    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix =
        perspective(70.0f,                  // field of view in degrees
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

    float lastclicked = glfwGetTime();

    // Other OpenGL states to set once
    // Enable Backface culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // Entering Main Loop
    while (!glfwWindowShouldClose(window)) {
        // Frame time calculation
        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime = glfwGetTime();

        // put the original color
        if (texture == 1) {
            SetUniformVec3(shaderScene, "object_color", vec3(1));
        }
        // light parameters
        float theta = glfwGetTime() * 0.5f * 3.14f;
        vec3 lightPosition = vec3(0.0f);
        if (light == 0) {
            // still light
            lightPosition = vec3(0.0f, 30.0f, 0.0f);
        }
        else {
            // rotating light
            lightPosition = vec3(0.0f, cosf(theta), sinf(theta)) * 5.0f;
        }

        vec3 lightFocus( 0.01);  // the point in 3D space the light "looks" at
        vec3 lightDirection = normalize(lightFocus - lightPosition);
        float lightNearPlane = 0.01f;
        float lightFarPlane = 100.0f;


        mat4 lightProjMatrix = mat4(1.0);

            if (shadows == 0) {
                // still light
                lightProjMatrix =
                    perspective(50.0f, (float)DEPTH_MAP_TEXTURE_SIZE / (float)DEPTH_MAP_TEXTURE_SIZE, lightNearPlane, lightFarPlane);
            }
            else {
                // rotating light
                lightProjMatrix = frustum(-1.0f, 1.0f, -(float)DEPTH_MAP_TEXTURE_SIZE, 1.0f, lightNearPlane, lightFarPlane);
            }
           
        mat4 lightViewMatrix = lookAt(lightPosition, lightFocus, vec3(0, 1, 0));

        // giving all the light info 
        SetUniformMat4(shaderScene, "light_proj_view_matrix", lightProjMatrix * lightViewMatrix);
        SetUniform1Value(shaderScene, "light_near_plane", lightNearPlane);
        SetUniform1Value(shaderScene, "light_far_plane", lightFarPlane);
        SetUniformVec3(shaderScene, "light_position", lightPosition);
        SetUniformVec3(shaderScene, "light_direction", lightDirection);


        int width, height;
        // getting the new size of the window
        glfwGetFramebufferSize(window, &width, &height);

        // changing the dimension of when window size is changed 
        if (WIDTH != width || HEIGHT != height) {
            glViewport(0, 0, width, height);
        }
        WIDTH = width;
        HEIGHT = height;


        // Set ball matrix and send to both shaders
        mat4 ballMatrices =  // mat4(1.0f);
            translate(mat4(1.0f), ballPosition) *
            rotate(mat4(1.0f), radians(angleHorizontal), vec3(0.0f, 1.0f, 0.0f)) *
            rotate(mat4(1.0f), radians(angleVertical), vec3(1.0f, 0.0f, 0.0f)) *
            scale(mat4(1.0f), vec3(0.06f + scalBall));

        SetUniformMat4(shaderScene, "model_matrix", ballMatrices);
        SetUniformMat4(shaderShadow, "transform_in_light_space", lightProjMatrix * lightViewMatrix * ballMatrices);
        //changing the color of the ball green 
        if (texture == 0) {
            vec3 objectColor2(0.0f, 1.0f, 0.0f);
            SetUniformVec3(shaderScene, "object_color", objectColor2);
        }

        // Render shadow in 2 passes: 1- Render depth map, 2- Render scene
        {
            // Use proper shader
            glUseProgram(shaderShadow);
            // Use proper image output size
            glViewport(0, 0, DEPTH_MAP_TEXTURE_SIZE, DEPTH_MAP_TEXTURE_SIZE);
            // Bind depth map texture as output framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
            // Clear depth data on the framebuffer
            glClear(GL_DEPTH_BUFFER_BIT);
            // Bind and draw geometry
            glBindVertexArray(activeBallVAO);
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
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // Bind and draw geometry
            glBindVertexArray(activeBallVAO);
            glDrawArrays(GL_TRIANGLES, 0, activeBallVertices);
            // Unbind geometry
            glBindVertexArray(0);
        }


        drawAxies(axiesPosition, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture);

        // moving the camera with mouse
        cameraLookAt = computeCameraLookAt(lastMousePosX, lastMousePosY, dt);
        vec3 cameraSideVector = cross(cameraLookAt, vec3(0.0f, 1.0f, 0.0f));
        normalize(cameraSideVector);

        // move camera left and right
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            cameraPosition += cameraSideVector * dt * cameraSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            cameraPosition -= cameraSideVector * dt * cameraSpeed;
        }

        // zoom
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            cameraPosition += cameraLookAt * dt * cameraSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            cameraPosition -= cameraLookAt * dt * cameraSpeed;
        }


        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        

        // place model at random place
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            modelPosition = getRandomGridPosition();
            vec3 newPos(modelPosition.x - 1.0, ballPosition.y, modelPosition.z);
            ballPosition = newPos;
        }

        // move model up
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            vec3 newPos(modelPosition.x, modelPosition.y + 0.1, modelPosition.z);
            modelPosition = newPos;
            mov = true;
        }
        // movdel model down
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            vec3 newPos(modelPosition.x, modelPosition.y - 0.1, modelPosition.z);
            modelPosition = newPos;
            mov = true;
        }
        // move model right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            vec3 newPos(modelPosition.x + 0.1, modelPosition.y, modelPosition.z);
            modelPosition = newPos;
            mov = true;
        }
        //move model left
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            vec3 newPos(modelPosition.x - 0.1, modelPosition.y, modelPosition.z);
            modelPosition = newPos;
            mov = true;
        }


        // rotate on y-axis anti-clockwise
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            angleHorizontal += 1;
            mov = false;
        }
        // rotate on y-axis clockwise
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            angleHorizontal -= 1;
            mov = false;
        }
        // rotate on x-axis clockwise
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            angleVertical -= 1;
            mov = false;
        }
        // rotate on x-axis anti-clockwise
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
            angleVertical += 1;
            mov = false;
        }


        // change the type of the models to fill
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            type = 0;
        }
        // change the type of the models to line
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            type = 1;
        }
        // change the type of the models to point
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            type = 2;
        }


        // make the model bigger
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
            scal += 0.01;
            scalBall += 0.001;
        }
        //make the model smaller
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            scal -= 0.01;
            scalBall -= 0.001;
        }

        // slect which handle (upper or lower) arm
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            arm = 0;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            arm = 1;
        }

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {

            float differentTime = glfwGetTime() - lastclicked;
            if (differentTime > 1.0f) {
                    lastclicked = glfwGetTime();
                if (light == 0) {
                    light = 1;
                }
                else {
                    light = 0;
                }
            }
            
        }

        //Remove color texture
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            float differentTime = glfwGetTime() - lastclicked;
            if (differentTime > 1.0f) {
                lastclicked = glfwGetTime();
                if (texture == 0) {
                    texture = 1;
                }
                else {
                    texture = 0;
                }
            }

        }
        
        // put the model to the original position
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
            float differentTime = glfwGetTime() - lastclicked;
            if (differentTime > 1.0f) {
                lastclicked = glfwGetTime();
                if (shadows == 0) {
                    shadows = 1;
                }
                else {
                    shadows = 0;
                }
            }
        }

        // put the model to the original position
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            modelPosition = modelPositionB;
            ballPosition = ballPositionB;
        }

        mat4 translation = mat4(1.0f);
        mat4 translation2 = translate(mat4(1.0f), modelPosition);
        // upper arm 
        if (arm == 0) {
            vec3 position(modelPosition.x - 0.17, modelPosition.y - 0.5, modelPosition.z);
            translation = translate(mat4(1.0f), -position);
        }
        // lower arm
        if (arm == 1) {
            vec3 position(modelPosition.x, modelPosition.y, modelPosition.z);
            translation = translate(mat4(1.0f), -position);
        }

        mat4 groupMatrix = rotate(mat4(1.0f), radians(angleVertical), vec3(1.0f, 0.0f, 0.0f)) *
            scale(mat4(1.0f), vec3(1.0f, 1.0f, 1.0f));

        mat4 groupMatrix2 =
            rotate(mat4(1.0f), radians(angleHorizontal), vec3(0.0f, 1.0f, 0.0f)) *
            scale(mat4(1.0f), vec3(scal));

        mat4 total = mat4(1.0f);

        if (mov) {
            total = groupMatrix * translation * groupMatrix2 * translation2;
        }
        else {

            total = groupMatrix * translation * groupMatrix2;
        }

        // drawing everything 
        drawFullModel(total, modelPosition, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture);


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
    std::vector<vec3> vertices;
    std::vector<vec3> normals;
    std::vector<vec2> UVs;

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
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec3),
        &vertices.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Normals VBO setup
    GLuint normals_VBO;
    glGenBuffers(1, &normals_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, normals_VBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(vec3),
        &normals.front(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(1);

    // UVs VBO setup
    GLuint uvs_VBO;
    glGenBuffers(1, &uvs_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvs_VBO);
    glBufferData(GL_ARRAY_BUFFER, UVs.size() * sizeof(vec2), &UVs.front(),
        GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat),
        (GLvoid*)0);
    glEnableVertexAttribArray(2);


    // 3rd attribute buffer : vertex color
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent
    // strange bugs, as we are using multiple VAOs)
    vertexCount = vertices.size();
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

