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

void SetUniformonOff(GLuint shader_id, const char* uniform_name,
    int uniform_value) {
    glUseProgram(shader_id);
    glUniform1i(glGetUniformLocation(shader_id, uniform_name), uniform_value);
}


void SetUniform2D(GLuint shader_id, const char* uniform_name,
    GLuint  uniform_value)
{
    glUseProgram(shader_id);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, uniform_value);

    // Set the texture unit index for the sampler uniform
    GLint textureUnitIndex = 1; // The same texture unit index used above
    glUniform1i(glGetUniformLocation(shader_id, uniform_name), textureUnitIndex);
}

void SetUniform2DGlossy(GLuint shader_id, const char* uniform_name,
    GLuint  uniform_value)
{
    glUseProgram(shader_id);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, uniform_value);

    // Set the texture unit index for the sampler uniform
    GLint textureUnitIndex = 2; // The same texture unit index used above
    glUniform1i(glGetUniformLocation(shader_id, uniform_name), textureUnitIndex);
}
GLuint loadTexture(const char* filename)
{
    // Step1 Create and bind textures
    GLuint textureId = 0;
    glGenTextures(1, &textureId);
    assert(textureId != 0);


    glBindTexture(GL_TEXTURE_2D, textureId);

    // Step2 Set filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

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

// creating the cube
void creatCube(mat4 modelMatrices, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type) {


    SetUniformMat4(shaderScene, "model_matrix", modelMatrices);

    SetUniformMat4(shaderShadow, "transform_in_light_space", lightProjMatrix * lightViewMatrix * modelMatrices);

    // Render shadow in 2 passes: 1- Render depth map, 2- Render scene
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
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture,
    GLuint tattooedTextureID) {

    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(position.x - 0.17, position.y - 0.5, position.z)) *
        rotate(mat4(1.0f), radians(320.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(0.1f, 0.5f, 0.1f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", tattooedTextureID);
    }
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
}

// drawing the upper arm
void drawUpperArm(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture,
    GLuint tattooedTextureID) {

    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y, position.z)) *
        scale(mat4(1.0f), vec3(0.1f, 0.5f, 0.1f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", tattooedTextureID);
    }
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
}


//drawing the rackets
void drawRacket(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture,
   GLuint greyGlossyTextureID, GLuint redGlossyTextureID,
    GLuint bleuGlossyTextureID) {


    //changing the color of the part of the racket red
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", redGlossyTextureID);
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
        SetUniform2D(shaderScene, "textureSampler", bleuGlossyTextureID);
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


    //changing the color of the net blueif 
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", greyGlossyTextureID);
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
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture
    , GLuint redTextureID, GLuint bleuTextureID, GLuint greenTextureID) {

    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 0.35f, position.z + 0.0f)) *
        scale(mat4(1.0f), vec3(0.02f, 0.5f, 0.02f));
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", redTextureID);
    }
    creatCube(modelMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    modelMatrices =
        translate(mat4(1.0f), vec3(position.x + 0.35f, position.y + 0.0f, position.z + 0.0f)) *
        scale(mat4(1.0f), vec3(0.5f, 0.02f, 0.02f));
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", bleuTextureID);
    }
    creatCube(modelMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 0.0f, position.z + 0.43f)) *
        scale(mat4(1.0f), vec3(0.02f, 0.02f, 0.5f));
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", greenTextureID);
    }
    creatCube(modelMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

}

// drawing everything at the same time
void drawFullModel(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture,
     GLuint greyGlossyTextureID, GLuint redGlossyTextureID, GLuint bleuGlossyTextureID, GLuint tattooedTextureID) {

    drawLowerArm(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture
        , tattooedTextureID);

    drawUpperArm(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture
        , tattooedTextureID);

    drawRacket(groupMatrix, position, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture, 
        greyGlossyTextureID, redGlossyTextureID, bleuGlossyTextureID);
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

// drawing the sky box
void drawSkybloack(vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture
    , GLuint skyTextureID) {
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", skyTextureID);
    }
    float scal = 140.0f;
    
    mat4 groundMatrices =  // mat4(1.0f);
        translate(mat4(1.0f), vec3(position.x, position.y-10.0f, position.z)) *
        scale(mat4(1.0f), vec3(scal, 0.1f, scal));


    SetUniformMat4(shaderScene, "model_matrix", groundMatrices);
    SetUniformMat4(shaderShadow, "transform_in_light_space", lightProjMatrix * lightViewMatrix * groundMatrices);



    //changing the color of the ball green 
    if (texture == 0) {
        //SetUniform2D(shaderScene, "textureSampler", clayTextureID);
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
        glBindVertexArray(activeVAO);
        glDrawArrays(GL_TRIANGLES, 0, activeVertices);
        // Unbind geometry
        glBindVertexArray(0);
    }

    ////// 2- Render scene: a- bind the default framebuffer and b- just render like
    ////// what we do normally
    {
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
        glBindVertexArray(activeVAO);
        glDrawArrays(GL_TRIANGLES, 0, activeVertices);
        // Unbind geometry
        glBindVertexArray(0);
    }


    groundMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y+150, position.z)) *
        rotate(mat4(1.0f), radians(270.0f), vec3(1.0f, 0.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, 0.1f));
    creatCube(groundMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type); 
    
    groundMatrices =
        translate(mat4(1.0f), vec3(position.x+100, position.y + 70, position.z)) *
        rotate(mat4(1.0f), radians(180.0f), vec3(1.0f, 0.0f, 0.0f)) *
        scale(mat4(1.0f), vec3( 0.1f,scal ,scal));
    creatCube(groundMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    groundMatrices =
        translate(mat4(1.0f), vec3(position.x - 100, position.y + 70, position.z)) *
        rotate(mat4(1.0f), radians(180.0f), vec3(1.0f, 0.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(0.1f, scal, scal));
    creatCube(groundMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    groundMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 70, position.z- 100)) *
        rotate(mat4(1.0f), radians(180.0f), vec3(1.0f, 0.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, 0.1f));
    creatCube(groundMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    groundMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 70, position.z + 100)) *
        rotate(mat4(1.0f), radians(180.0f), vec3(1.0f, 0.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, 0.1f));
    creatCube(groundMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

}


// drawing the Net and the Court
void drawNet(vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture
    , GLuint netTextureID, GLuint metalTextureID, GLuint ballGreenTextureID) {

    // draw the ground
    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", ballGreenTextureID);
    }
    
    mat4 netMatrices = 
        translate(mat4(1.0f), vec3(position.x, position.y-3.0f, position.z)) *
        rotate(mat4(1.0f), radians(90.0f), vec3(1.0f, 0.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(14.0f, 26.0f, 0.1f ));

    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", netTextureID);
    }
    // the lines on the court 
    // front -
    netMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y - 2.9f, position.z+19.5f)) *
        scale(mat4(1.0f), vec3(14.3f, 0.2f, 0.2f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    // back -
    netMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y - 2.9f, position.z - 19.5f)) *
        scale(mat4(1.0f), vec3(14.3f, 0.2f, 0.2f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    // left |
    netMatrices =
        translate(mat4(1.0f), vec3(position.x-10.5f, position.y - 2.9f, position.z)) *
        scale(mat4(1.0f), vec3(0.2f, 0.2f, 26.0f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // right |
    netMatrices =
        translate(mat4(1.0f), vec3(position.x + 10.5f, position.y - 2.9f, position.z)) *
        scale(mat4(1.0f), vec3(0.2f, 0.2f, 26.0f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // left close |
    netMatrices =
        translate(mat4(1.0f), vec3(position.x - 8.0f, position.y - 2.9f, position.z)) *
        scale(mat4(1.0f), vec3(0.2f, 0.2f, 26.0f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // right close |
    netMatrices =
        translate(mat4(1.0f), vec3(position.x + 8.0f, position.y - 2.9f, position.z)) *
        scale(mat4(1.0f), vec3(0.2f, 0.2f, 26.0f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // front close -
    netMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y - 2.9f, position.z + 11.5f)) *
        scale(mat4(1.0f), vec3(14.3f, 0.2f, 0.2f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    // back close -
    netMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y - 2.9f, position.z - 11.5f)) *
        scale(mat4(1.0f), vec3(14.3f, 0.2f, 0.2f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    // center |
    netMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y - 2.9f, position.z)) *
        scale(mat4(1.0f), vec3(0.2f, 0.2f, 15.5f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // - net
    for (int i = 0; i < 10; i++) {
        netMatrices =
            translate(mat4(1.0f), vec3(position.x, position.y - 2.5f + (0.25 * i), position.z)) *
            scale(mat4(1.0f), vec3(16.0, 0.05f, 0.05f));
        creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    }

    netMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y + 0.15f, position.z)) *
        scale(mat4(1.0f), vec3(16.0, 0.1f, 0.1f));
    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    // | net
    for (int i = 0; i < 70; i++) {
        netMatrices =
            translate(mat4(1.0f), vec3(position.x - 12 + (0.35 * i), position.y - 1.2f, position.z)) *
            scale(mat4(1.0f), vec3(0.05f, 1.8f, 0.05f));
        creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);
    }


    if (texture == 0) {
        SetUniform2D(shaderScene, "textureSampler", metalTextureID);
    }
    //poles
    netMatrices = 
        translate(mat4(1.0f), vec3(position.x-12.0f, position.y- 1.2f, position.z)) *
        scale(mat4(1.0f), vec3(0.3f, 2.5f, 0.3f));

    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    netMatrices =
        translate(mat4(1.0f), vec3(position.x + 12.0f, position.y - 1.2f, position.z)) *
        scale(mat4(1.0f), vec3(0.3f, 2.5f, 0.3f));

    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    netMatrices =
        translate(mat4(1.0f), vec3(position.x, position.y - 1.2f, position.z)) *
        scale(mat4(1.0f), vec3(0.3f, 2.5f, 0.3f));

    creatCube(netMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    
}


// drawing the letter A
void drawA(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture,
    GLuint aTextureID, vec3 color, float alphaA) {
    
    if (texture == 0) {
        SetUniformVec3(shaderScene, "object_color", color);
        SetUniform2D(shaderScene, "textureSampler", aTextureID);
    }

    vec3 realPos(position.x-2, position.y + 4.5f, position.z);


    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x, realPos.y + 0.2f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.6f, 0.12f, 0.12f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x, realPos.y + 1.0f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.62f, 0.12f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x - 0.54f, realPos.y, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 1.2f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x + 0.54f, realPos.y, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 1.2f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    SetUniformVec3(shaderScene, "object_color", vec3(1));
}


// drawing the letter R
void drawR(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture,
    GLuint rTextureID, vec3 color, float alphaA) {

    if (texture == 0) {
        SetUniformVec3(shaderScene, "object_color", color);
        SetUniform2D(shaderScene, "textureSampler", rTextureID);
    }

    vec3 realPos(position.x + 2, position.y + 4.5f, position.z);


    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x, realPos.y + 0.2f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.6f, 0.12f, 0.12f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x, realPos.y + 1.0f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.62f, 0.12f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x - 0.54f, realPos.y, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 1.2f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x + 0.54f, realPos.y - 0.37, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 0.7f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x + 0.54f, realPos.y + 0.6, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 0.4f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    SetUniformVec3(shaderScene, "object_color", vec3(1));
}

// drawing the letter U
void drawU(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture,
    GLuint uTextureID, vec3 color) {

    if (texture == 0) {
        SetUniformVec3(shaderScene, "object_color", color);
        SetUniform2D(shaderScene, "textureSampler", uTextureID);
    }

    vec3 realPos(position.x - 2, position.y + 4.5f, position.z);


    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x, realPos.y - 0.8f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.6f, 0.12f, 0.12f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x - 0.54f, realPos.y + 0.2f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 1.2f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x + 0.54f, realPos.y + 0.2f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 1.2f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    SetUniformVec3(shaderScene, "object_color", vec3(1));
}

// drawing the letter B
void drawB(mat4 groupMatrix, vec3 position, GLuint shaderScene, GLuint shaderShadow, mat4 lightProjMatrix, mat4 lightViewMatrix, mat4 viewMatrix
    , vec3 cameraPosition, vec3 cameraLookAt, vec3 cameraUp, GLuint depth_map_fbo, GLuint activeVAO, int activeVertices, int type, int texture,
    GLuint bTextureID, vec3 color) {

    if (texture == 0) {
        SetUniformVec3(shaderScene, "object_color", color);
        SetUniform2D(shaderScene, "textureSampler", bTextureID);
    }

    vec3 realPos(position.x + 2, position.y + 4.5f, position.z);

    mat4 modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x - 0.54f, realPos.y+0.1, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 1.1f, 0.12f));
    mat4 woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x, realPos.y + 0.15f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.6f, 0.12f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type); 
    
    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x, realPos.y + 1.0f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.62f, 0.12f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x, realPos.y - 0.8f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.62f, 0.12f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x + 0.54f, realPos.y - 0.33f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 0.52f, 0.12f)); 
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);

    modelMatrices =
        translate(mat4(1.0f), vec3(realPos.x + 0.54f, realPos.y + 0.55f, realPos.z)) *
        scale(mat4(1.0f), vec3(0.12f, 0.45f, 0.12f));
    woldMatrices = groupMatrix * modelMatrices;
    creatCube(woldMatrices, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
        , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type);


    SetUniformVec3(shaderScene, "object_color", vec3(1));
}



int main(int argc, char* argv[]) {

    GLuint WIDTH = 1024, HEIGHT = 768;
    if (!InitContext(WIDTH, HEIGHT)) return -1;

    //Background color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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
#else
    string cubePath = "Assets/Models/cube.obj";
#endif

    GLuint clayTextureID = loadTexture("Assets/textures/clayTexture.png");
    GLuint ballGreenTextureID = loadTexture("Assets/textures/ballGreen.jpg");
    GLuint bleuTextureID = loadTexture("Assets/textures/blue.jpg");
    GLuint glossyTextureID = loadTexture("Assets/textures/glossy.png");
    GLuint greyTextureID = loadTexture("Assets/textures/gray.jpg");
    GLuint redTextureID = loadTexture("Assets/textures/red.jpg");
    GLuint greenTextureID = loadTexture("Assets/textures/green.jpg");
    GLuint whiteTextureID = loadTexture("Assets/textures/white.jpg");
    GLuint redGlossyTextureID = loadTexture("Assets/textures/redGlossy.jpg");
    GLuint bleuGlossyTextureID = loadTexture("Assets/textures/blueGlossy.jpg");
    GLuint greyGlossyTextureID = loadTexture("Assets/textures/greyGlossy.jpg");
    GLuint tattooedTextureID = loadTexture("Assets/textures/tattooed.jpg");
    GLuint skyTextureID = loadTexture("Assets/textures/sky.jpg");
    GLuint netTextureID = loadTexture("Assets/textures/net.png");
    GLuint aTextureID = loadTexture("Assets/textures/a.jpg");
    GLuint rTextureID = loadTexture("Assets/textures/r.jpg");
    GLuint uTextureID = loadTexture("Assets/textures/u.jpg");
    GLuint bTextureID = loadTexture("Assets/textures/b.jpg");

    
    
    int cubeVertices;
    GLuint cubeVAO = setupModelVBO(cubePath, cubeVertices);

    int activeVertices = cubeVertices;
    GLuint activeVAO = cubeVAO;


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

    vec3 cameraPositionDefault(25.0f, 8.0f, 20.0f);
    vec3 cameraPosition1(0.0f, 3.5f, 25.0f);
    vec3 cameraPosition2(0.0f, 3.5f, -25.0f);
    vec3 cameraPositionCircle(0.0f, 3.5f, 10.0f);



    vec3 cameraPosition = cameraPositionDefault;
    vec3 cameraLookAt(0.0f, 0.0f, 0.0f);
    vec3 cameraUp(0.0f, 1.0f, 0.0f);

    // Other camera parameters
    float cameraSpeed = 5.0f;
    float cameraFastSpeed = 3 * cameraSpeed;
    vec3 modelPosition(0.0f, 1.0f, 10.0f);
    vec3 modelPosition2(0.0f, 1.0f, -10.0f);
    vec3 axiesPosition(6.0f, 2.5f, 3.0f);



    vec3 modelPositionDefault = modelPosition;
    vec3 modelPosition2Default = modelPosition2;

    float angleHorizontal = 0;
    float angleVertical = 0;
    float angleZ = 0;
    float angleHorizontal2 = 0;
    float angleVertical2 = 0;
    float angleZ2 = 0;
    float scal = 1.0f;
    float scal2 = 1.0f;

    // For spinning model
    int type = 0; // the type of model
    int light = 0; // the type of light
    int racket = 0; // what arm
    bool mov = true;
    int shadows = 0;
    int texture = 0;
    int nextCamera = 0;
    int ambient = 0;

    // Set projection matrix for shader, this won't change
    mat4 projectionMatrix =
        perspective(70.0f,                  // field of view in degrees
            WIDTH * 1.0f / HEIGHT,  // aspect ratio
            0.01f, 800.0f);         // near and far (near > 0)

    // Set initial view matrix on both shaders
    mat4 viewMatrix = lookAt(cameraPosition, cameraLookAt,  // center
        cameraUp);                      // up


    SetUniformMat4(shaderScene, "projection_matrix", projectionMatrix);

    SetUniformMat4(shaderScene, "view_matrix", viewMatrix);


    float lightAngleOuter = radians(30.0f);
    float lightAngleInner = radians(20.0f);
    // Set light cutoff angles on scene shader
    SetUniform1Value(shaderScene, "light_cutoff_inner", cos(lightAngleInner));
    SetUniform1Value(shaderScene, "light_cutoff_outer", cos(lightAngleOuter));

    // Set light color on scene shader
    SetUniformVec3(shaderScene, "light_color", vec3(1));
    SetUniformVec3(shaderScene, "object_color", vec3(1));


    // For frame time
    float lastFrameTime = glfwGetTime();

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
            SetUniformVec3(shaderScene, "light_color", vec3(1));
            lightPosition = vec3(0.0f, 0.0f,  30.0f);
        }
        else {
            // rotating light
            SetUniformVec3(shaderScene, "light_color", vec3(0.565f, 0.933f, 0.565f));
            lightPosition = vec3(cosf(theta),0.0f ,sinf(theta)) * 30.0f;

            viewMatrix = lookAt(lightPosition,  vec3(0), cameraUp);
            SetUniformMat4(shaderScene, "view_matrix", viewMatrix);
            //Set view position on scene shader
            SetUniformVec3(shaderScene, "view_position", lightPosition);
        }

        vec3 lightFocus(0.01);  // the point in 3D space the light "looks" at
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
        SetUniformonOff(shaderScene, "onOff", ambient);


        int width, height;
        // getting the new size of the window
        glfwGetFramebufferSize(window, &width, &height);

        // changing the dimension of when window size is changed 
        if (WIDTH != width || HEIGHT != height) {
            glViewport(0, 0, width, height);
        }
        WIDTH = width;
        HEIGHT = height;

        // draw sky box
        drawSkybloack(vec3(0.0f,0.0f,0.0f), shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture
            , skyTextureID);

        // draw axies
        drawAxies(axiesPosition, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture,
            redTextureID, bleuTextureID, greenTextureID);

        // draw net and court 
        drawNet(vec3(0.0f, 0.0f, 0.0f), shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture
            , whiteTextureID, greyGlossyTextureID, ballGreenTextureID);


        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        // reset everything
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {

            modelPosition = modelPositionDefault;
            modelPosition2 = modelPosition2Default;

            angleHorizontal = 0;
            angleVertical = 0;
            angleZ = 0;
            angleHorizontal2 = 0;
            angleVertical2 = 0;
            angleZ2 = 0;
            scal = 1.0f;
            scal2 = 1.0f;

            type = 0; 
            light = 0; 
            racket = 0;
            mov = true;
            shadows = 0;
            texture = 0;
            nextCamera = 0;
            ambient = 0;
        }

        // place model at random place
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {

            if (racket == 0) {
                modelPosition = getRandomGridPosition();
            }
            else {
                modelPosition2 = getRandomGridPosition();
            }
        }

        // move model up
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            if (racket == 0) {
                vec3 newPos(modelPosition.x, modelPosition.y + 0.1, modelPosition.z);
                modelPosition = newPos;
            }
            else {
                vec3 newPos(modelPosition2.x, modelPosition2.y + 0.1, modelPosition2.z);
                modelPosition2 = newPos;
            }
            mov = true;
        }
        // movdel model down
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            if (racket == 0) {
                vec3 newPos(modelPosition.x, modelPosition.y - 0.1, modelPosition.z);
                modelPosition = newPos;
            }
            else {
                vec3 newPos(modelPosition2.x, modelPosition2.y - 0.1, modelPosition2.z);
                modelPosition2 = newPos;
            }
            mov = true;
        }
        // move model right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            if (racket == 0) {
                vec3 newPos(modelPosition.x + 0.1, modelPosition.y, modelPosition.z);
                modelPosition = newPos;
            }
            else {
                vec3 newPos(modelPosition2.x + 0.1, modelPosition2.y, modelPosition2.z);
                modelPosition2 = newPos;
            }
            mov = true;
        }
        //move model left
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            if (racket == 0) {
                vec3 newPos(modelPosition.x - 0.1, modelPosition.y, modelPosition.z);
                modelPosition = newPos;
            }
            else {
                vec3 newPos(modelPosition2.x - 0.1, modelPosition2.y, modelPosition2.z);
                modelPosition2 = newPos;
            }
            mov = true;
        }


        // rotate on y-axis anti-clockwise
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            if (racket == 0) {
                angleHorizontal += 1;
            }
            else {
                angleHorizontal2 += 1;
            }
            mov = false;
        }
        // rotate on y-axis clockwise
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            if (racket == 0) {
                angleHorizontal -= 1;
            }
            else {
                angleHorizontal2 -= 1;
            }
            mov = false;
        }
        // rotate on z-axis anti-clockwise
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            if (racket == 0) {
                angleZ += 1;
            }
            else {
                angleZ2 += 1;
            }
            mov = false;
        }
        // rotate on z-axis clockwise
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            if (racket == 0) {
                angleZ -= 1;
            }
            else {
                angleZ2 -= 1;
            }
            mov = false;
        }
        // rotate on x-axis clockwise
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            if (racket == 0) {
                angleVertical -= 1;
            }
            else {
                angleVertical2 -= 1;
            }
            mov = false;
        }
        // rotate on x-axis anti-clockwise
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS) {
            if (racket == 0) {
                angleVertical += 1;
            }
            else {
                angleVertical2 += 1;
            }
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
            if (racket == 0) {
                scal += 0.01;
            }
            else {
                scal2 += 0.01;
            }
        }
        //make the model smaller
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            if (racket == 0) {
                scal -= 0.01;
            }
            else {
                scal2 -= 0.01;
            }
        }

        // slect which handle (upper or lower) arm
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            racket = 0;
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            racket = 1;
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

        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
            float differentTime = glfwGetTime() - lastclicked;
            if (differentTime > 1.0f) {
                lastclicked = glfwGetTime();
                if (ambient == 0) {
                    ambient = 1;
                }
                else {
                    ambient = 0;
                }
            }
        }

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            float differentTime = glfwGetTime() - lastclicked;
            if (differentTime > 1.0f) {
                lastclicked = glfwGetTime();
                nextCamera++;
                if (nextCamera > 2) {
                    nextCamera = 0;
                }
            }
        }

        

        mat4 groupMatrix = rotate(mat4(1.0f), radians(angleVertical), vec3(1.0f, 0.0f, 0.0f)) *
            scale(mat4(1.0f), vec3(scal));

        mat4 groupMatrix2 =
            rotate(mat4(1.0f), radians(angleHorizontal), vec3(0.0f, 1.0f, 0.0f)) *
            scale(mat4(1.0f), vec3(scal));

        mat4 groupMatrix3 =
            rotate(mat4(1.0f), radians(angleZ), vec3(0.0f, 0.0f, 1.0f)) *
            scale(mat4(1.0f), vec3(scal));

        mat4 total = mat4(1.0f);

        if (mov) {
            total = groupMatrix  * groupMatrix2 *  groupMatrix3;
        }
        else {

            total = groupMatrix  * groupMatrix2 * groupMatrix3;
        }
        
        for (int i = 0; i < 3; i++) {
            if (i == 2) {
                glDisable(GL_DEPTH_TEST);
            }
            vec3 newPos(modelPosition.x, modelPosition.y, modelPosition.z + (i * 0.3));
            float alphaA = 0.5f;

            drawA(total, newPos, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
                , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture
                , aTextureID, vec3(0.45f + (i* 0.15), 1.0f, 0.45f + (i * 0.15)), alphaA);

            drawR(total, newPos, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
                , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture
                , rTextureID, vec3(1.0f, 0.45f + (i * 0.15), 0.45f + (i * 0.15)), alphaA);

        }



        glEnable(GL_DEPTH_TEST);

        // drawing everything 
        drawFullModel(total, modelPosition, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture,
             greyGlossyTextureID, redGlossyTextureID
            , bleuGlossyTextureID, tattooedTextureID);

         groupMatrix = rotate(mat4(1.0f), radians(angleVertical2), vec3(1.0f, 0.0f, 0.0f)) *
            scale(mat4(1.0f), vec3(scal2));

         groupMatrix2 =
            rotate(mat4(1.0f), radians(angleHorizontal2), vec3(0.0f, 1.0f, 0.0f)) *
            scale(mat4(1.0f), vec3(scal2));

         groupMatrix3 =
            rotate(mat4(1.0f), radians(angleZ2), vec3(0.0f, 0.0f, 1.0f)) *
            scale(mat4(1.0f), vec3(scal2));

        if (mov) {
            total = groupMatrix * groupMatrix2 * groupMatrix3;
        }
        else {

            total = groupMatrix  * groupMatrix2 * groupMatrix3;
        }

        for (int i = 0; i < 3; i++) {
            if (i == 2) {
                glDisable(GL_DEPTH_TEST);
            }
            vec3 newPos(modelPosition2.x, modelPosition2.y, modelPosition2.z - (i * 0.3));

            drawU(total, newPos, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
                , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture
                , uTextureID, vec3(0.45f + (i * 0.15), 0.45f + (i * 0.15), 1.0f));

            drawB(total, newPos, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
                , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture
                , bTextureID, vec3(0.45f + (i * 0.25), 0.45f + (i * 0.25), 0.45f + (i * 0.15)));

        }
        

        glEnable(GL_DEPTH_TEST);
        // drawing everything 
        drawFullModel(total, modelPosition2, shaderScene, shaderShadow, lightProjMatrix, lightViewMatrix, viewMatrix
            , cameraPosition, cameraLookAt, cameraUp, depth_map_fbo, activeVAO, activeVertices, type, texture,
            greyGlossyTextureID, redGlossyTextureID
            , bleuGlossyTextureID, tattooedTextureID);

        if (nextCamera == 0) {

            viewMatrix = lookAt(cameraPositionDefault, cameraLookAt, cameraUp);
            SetUniformMat4(shaderScene, "view_matrix", viewMatrix);
            //Set view position on scene shader
            SetUniformVec3(shaderScene, "view_position", cameraPositionDefault);
        }
        else  if (nextCamera == 1) {

            viewMatrix = lookAt(cameraPosition1, cameraLookAt, cameraUp);
            SetUniformMat4(shaderScene, "view_matrix", viewMatrix);
            //Set view position on scene shader
            SetUniformVec3(shaderScene, "view_position", cameraPosition1);

        }
        else {
            viewMatrix = lookAt(cameraPosition2, cameraLookAt, cameraUp);
            SetUniformMat4(shaderScene, "view_matrix", viewMatrix);
            //Set view position on scene shader
            SetUniformVec3(shaderScene, "view_position", cameraPosition2);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
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