#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
using namespace glm;
using namespace std;

// Window dimensions
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// Grid dimensions
const int GRID_SIZE = 100;
const float GRID_CELL_SIZE = 0.2f;


//axis line
const float LINE_LENGTH = 2.0f;
GLuint shaderProgram;

// placement of the object created at the center
vec3 modelPosition(0.0f, 0.0f, 0.0f);



// Define the vertices of the unit cube 
//using grid width to create the cube vertices
float cubeVertices[] = {
    // Front face
    -0.1f, 0.0f, 0.1f,  // Bottom-left
    0.1f, 0.0f, 0.1f,   // Bottom-right
    0.1f, 0.5f, 0.1f,    // Top-right
    -0.1f, 0.5f, 0.1f,   // Top-left

    // Back face
    -0.1f, 0.0f, -0.1f, // Bottom-left
    0.1f, 0.0f, -0.1f,  // Bottom-right
    0.1f, 0.5f, -0.1f,   // Top-right
    -0.1f, 0.5f, -0.1f,  // Top-left
};



// Define the indices for the cube's triangles
unsigned int cubeIndices[] = {
    // Front face
    0, 1, 2, // Triangle 1
    2, 3, 0, // Triangle 2

    // Right face
    1, 5, 6, // Triangle 1
    6, 2, 1, // Triangle 2

    // Back face
    7, 6, 5, // Triangle 1
    5, 4, 7, // Triangle 2

    // Left face
    4, 0, 3, // Triangle 1
    3, 7, 4, // Triangle 2

    // Top face
    3, 2, 6, // Triangle 1
    6, 7, 3, // Triangle 2

    // Bottom face
    4, 5, 1, // Triangle 1
    1, 0, 4  // Triangle 2
};


/*
const char* getVertexShaderSource()
{
    // For now, you use a string for your shader code, in the assignment, shaders will be stored in .glsl files
    return
                "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;"
                "layout (location = 1) in vec3 aColor;"
                ""
                "uniform mat4 worldMatrix;"
                "uniform mat4 viewMatrix = mat4(1.0);"  // default value for view matrix (identity)
                "uniform mat4 projectionMatrix = mat4(1.0);"
                ""
                "out vec3 vertexColor;"
                "void main()"
                "{"
                "   vertexColor = aColor;"
                "   mat4 modelViewProjection = projectionMatrix * viewMatrix * worldMatrix;"
                "   gl_Position = modelViewProjection * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
                "}";
}


const char* getFragmentShaderSource()
{
    return
                "#version 330 core\n"
                "in vec3 vertexColor;"
                "out vec4 FragColor;"
                "void main()"
                "{"
                "   FragColor = vec4(vertexColor.r, vertexColor.g, vertexColor.b, 1.0f);"
                "}";
}


*/





// Vertex shader source code
const char* vertexShaderSource = R"(
    #version 330 core

    uniform mat4 projection= mat4(1.0);;
    uniform mat4 view = mat4(1.0);
uniform mat4 worldMatrix;
    layout (location = 0) in vec3 position;

    void main()
    {
mat4 modelViewProjection = projection * view * worldMatrix;
        gl_Position = modelViewProjection * vec4(position.x, position.y, position.z, 1.0);
    }
)";

// Fragment shader source code
const char* fragmentShaderSource = R"(
    #version 330 core

    uniform vec3 color;
    out vec4 FragColor;

    void main()
    {
        FragColor = vec4(color, 1.0);
    }
)";

void checkShaderCompilation(GLuint shader)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        cerr << "Shader compilation failed:\n" << infoLog << endl;
    }
}

void checkShaderLinking(GLuint program)
{
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        cerr << "Shader linking failed:\n" << infoLog << endl;
    }
}


void drawGrid()
{
    // Create the vertex buffer object (VBO) and vertex array object (VAO) for the grid
    GLuint gridVBO, gridVAO;
    glGenBuffers(1, &gridVBO);
    glGenVertexArrays(1, &gridVAO);

    // Bind the VAO and VBO
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);

    // Generate the grid vertices 
    // since we have x,y and z is multiplide by 3
    // we have 76x2 + 36x2 + +2 the last '+2' is the missing vertex since it starts with 0
    // Generate the grid vertices
    float gridVertices[(GRID_SIZE + 1) * 4 * 3];
    int index = 0;

    for (int i = 0; i <= GRID_SIZE; ++i)
    {
        float xPos = (i - GRID_SIZE / 2) * GRID_CELL_SIZE;

        // Horizontal lines
        gridVertices[index++] = -GRID_SIZE / 2 * GRID_CELL_SIZE;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = xPos;

        gridVertices[index++] = GRID_SIZE / 2 * GRID_CELL_SIZE;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = xPos;

        // Vertical lines
        gridVertices[index++] = xPos;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = -GRID_SIZE / 2 * GRID_CELL_SIZE;

        gridVertices[index++] = xPos;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = GRID_SIZE / 2 * GRID_CELL_SIZE;
    }

    // Set the vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW);

    // Set the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    //set the line width bigger then default
    glLineWidth(2);
    // Draw the grid lines
    glDrawArrays(GL_LINES, 0, (GRID_SIZE + 1) * 4);

    // Unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Delete the VBO and VAO to release resources
    glDeleteBuffers(1, &gridVBO);
    glDeleteVertexArrays(1, &gridVAO);
}



void drawLine(float startX, float startY, float startZ, float endX, float endY, float endZ, float height)
{
    // Create the vertex buffer object (VBO) and vertex array object (VAO) for the line
    GLuint lineVBO, lineVAO;
    glGenBuffers(1, &lineVBO);
    glGenVertexArrays(1, &lineVAO);

    // Bind the VAO and VBO
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);

    // creating the line vertices and added the right coner of the screen
    float vertices[] = {
         startX + 2, startY + 6, startZ + 2,
        endX + 2, endY + 6, endZ + 2
    };

    // Set the vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Set the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    //creating the line at the origin
    mat4 lineTranslation = translate(mat4(1.0f), vec3(startX, startY, startZ));
    mat4 lineDirection = translate(mat4(1.0f), vec3(endX - startX, endY - startY, endZ - startZ));
    mat4 modelMatrix = lineTranslation * lineDirection;

    // Apply the translation to move the line to the middle of the screen
    mat4 screenTranslation = translate(mat4(1.0f), vec3(-0.5f, height, -0.5f));

    modelMatrix = screenTranslation * modelMatrix;

    // Set the model matrix uniform in the shader
    GLint modelLocation = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, value_ptr(modelMatrix));
    glEnable(GL_LINE_SMOOTH);
    //change the default line width
    glLineWidth(5);
    // Draw the line
    glDrawArrays(GL_LINES, 0, 2);
    glDisable(GL_LINE_SMOOTH);

    // Delete the VBO and VAO to release resources
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &lineVAO);
}

//creating the big cube 
void drawUnitCube(const float cube[], vec3 position, int type)
{
    // Create the vertex buffer object (VBO) and vertex array object (VAO) for the cube
    GLuint cubeVBO, cubeVAO, cubeEBO;
    glGenBuffers(1, &cubeVBO);
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeEBO);

    // Bind the VAO and VBO
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);


    // Set the vertex data
    //since we need the size of the array and we are just having a pointer 
    //use the vertices created as a base and pass the element seperatly
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cube, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    // Set the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), modelPosition);

    if (type == 1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (type == 2) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (type == 3) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Draw the cube using indexed vertices
    glDrawElements(GL_POLYGON, sizeof(cubeIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

    // Unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Delete the VBO, VAO, and EBO to release resources
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeEBO);
}

void createVertieces(const float x, const float y, const float z, const float sizeX, const float sizeY, const float sizeZ, int type) {
    float vetices[]{

        // Front face
        x - (sizeX / 2), y - (sizeY / 2),(sizeZ / 2) + z,  // Bottom-left
       (sizeX / 2) + x, y - (sizeY / 2), (sizeZ / 2) + z,   // Bottom-right
       (sizeX / 2) + x, (sizeY / 2) + y,(sizeZ / 2) + z,    // Top-right
         x - (sizeX / 2),(sizeY / 2) + y, (sizeZ / 2) + z,   // Top-left

         // Back face
          x - (sizeX / 2), y - (sizeY / 2), z - (sizeZ / 2), // Bottom-left
         (sizeX / 2) + x, y - (sizeY / 2),z - (sizeZ / 2),  // Bottom-right
         (sizeX / 2) + x, (sizeY / 2) + y, z - (sizeZ / 2),   // Top-right
        x - (sizeX / 2),(sizeY / 2) + y, z - (sizeZ / 2)  // Top-left

    };

    vec3  position(x, y, z);
    drawUnitCube(vetices, position, type);

}

vec3 getRandomGridPosition()
{
    float minX = -GRID_SIZE / 2 * GRID_CELL_SIZE;
    float maxX = GRID_SIZE / 2 * GRID_CELL_SIZE;
    float minZ = -GRID_SIZE / 2 * GRID_CELL_SIZE;
    float maxZ = GRID_SIZE / 2 * GRID_CELL_SIZE;

    float x = minX + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxX - minX)));
    float z = minZ + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (maxZ - minZ)));

    return vec3(x, 2.0f, z);
}

int createVertexBufferObject()
{
    // Cube model
    vec3 vertexArray[] = {  // position,                            color
        vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f), //left - red
        vec3(-0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(-0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 0.0f),
        vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 0.0f),

        vec3(0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f), // far - blue
        vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
        vec3(-0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),

        vec3(0.5f, 0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
        vec3(0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),
        vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 0.0f, 1.0f),

        vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f), // bottom - turquoise
        vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),
        vec3(0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),

        vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f),
        vec3(-0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 1.0f),
        vec3(-0.5f,-0.5f,-0.5f), vec3(0.0f, 1.0f, 1.0f),

        vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f), // near - green
        vec3(-0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
        vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),
        vec3(0.5f,-0.5f, 0.5f), vec3(0.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f), // right - purple
        vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),
        vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),

        vec3(0.5f,-0.5f,-0.5f), vec3(1.0f, 0.0f, 1.0f),
        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f),
        vec3(0.5f,-0.5f, 0.5f), vec3(1.0f, 0.0f, 1.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f), // top - yellow
        vec3(0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),
        vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),

        vec3(0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f),
        vec3(-0.5f, 0.5f,-0.5f), vec3(1.0f, 1.0f, 0.0f),
        vec3(-0.5f, 0.5f, 0.5f), vec3(1.0f, 1.0f, 0.0f)
    };


    // Create a vertex array
    GLuint vertexArrayObject;
    glGenVertexArrays(1, &vertexArrayObject);
    glBindVertexArray(vertexArrayObject);


    // Upload Vertex Buffer to the GPU, keep a reference to it (vertexBufferObject)
    GLuint vertexBufferObject;
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexArray), vertexArray, GL_STATIC_DRAW);

    glVertexAttribPointer(0,                   // attribute 0 matches aPos in Vertex Shader
        3,                   // size
        GL_FLOAT,            // type
        GL_FALSE,            // normalized?
        2 * sizeof(vec3), // stride - each vertex contain 2 vec3 (position, color)
        (void*)0             // array buffer offset
    );
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1,                            // attribute 1 matches aColor in Vertex Shader
        3,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(vec3),
        (void*)sizeof(vec3)      // color is offseted a vec3 (comes after position)
    );
    glEnableVertexAttribArray(1);


    return vertexBufferObject;
}

void drawUpperArm(int colorLocation, int worldMatrixLocation, vec3 cubePosition, float rotation, float scal, int type) {
    glUniform3f(colorLocation, 0.4f, 0.2f, 0.0f);  //Red
    mat4 pillarWorldMatrix = translate(mat4(1.0f), vec3(cubePosition.x, cubePosition.y, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 1.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &pillarWorldMatrix[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.5f, 10.0f, 1.5f, type);

}

void drawLowerArm(int colorLocation, int worldMatrixLocation, vec3 cubePosition, float rotation, float scal, int type) {

    glUniform3f(colorLocation, 0.4f, 0.2f, 0.0f); // Light Red
    mat4 pillarWorldMatrix = translate(mat4(1.0f), vec3(cubePosition.x - 0.2f, cubePosition.y - 0.7f, cubePosition.z + 0.25f)) * rotate(mat4(1.0f), radians(rotation + 60.0f), vec3(0.0f, 1.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &pillarWorldMatrix[0][0]);

    createVertieces(0.0f, 0.0f, 0.0f, 9.0f, 1.5f, 1.5f, type);
}

void drawRacket(int colorLocation, int worldMatrixLocation, vec3 cubePosition, float rotation, float scal, int type) {

    glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f); // Red
    mat4 drawRacket0 = translate(mat4(1.0f), vec3(cubePosition.x, cubePosition.y + 1.0f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 1.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket0[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 11.0f, 1.0f, type);

    mat4 drawRacket4 = translate(mat4(1.0f), vec3(cubePosition.x, cubePosition.y + 1.8f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 1.0f, 0.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket4[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 6.0f, 1.0f, 1.0f, type);

    mat4 drawRacket5 = translate(mat4(1.0f), vec3(cubePosition.x + 0.52f, cubePosition.y + 2.5f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket5[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 11.0f, 1.0f, type);

    mat4 drawRacket6 = translate(mat4(1.0f), vec3(cubePosition.x - 0.52f, cubePosition.y + 2.5f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket6[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 11.0f, 1.0f, type);

    mat4 drawRacket9 = translate(mat4(1.0f), vec3(cubePosition.x - 0.2f, cubePosition.y + 3.25f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation + 290.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket9[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 3.0f, 1.0f, type);

    mat4 drawRacket10 = translate(mat4(1.0f), vec3(cubePosition.x + 0.2f, cubePosition.y + 3.25f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation + 70.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket10[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 3.0f, 1.0f, type);



    glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f);
    mat4 drawRacket1 = translate(mat4(1.0f), vec3(cubePosition.x - 0.28f, cubePosition.y + 1.75f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation + 50.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket1[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 7.0f, 1.0f, type);

    mat4 drawRacket2 = translate(mat4(1.0f), vec3(cubePosition.x + 0.28f, cubePosition.y + 1.75f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation + 310.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket2[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 7.0f, 1.0f, type);

    mat4 drawRacket7 = translate(mat4(1.0f), vec3(cubePosition.x - 0.44f, cubePosition.y + 3.1f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation + 310.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket7[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 3.0f, 1.0f, type);

    mat4 drawRacket8 = translate(mat4(1.0f), vec3(cubePosition.x + 0.44f, cubePosition.y + 3.1f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation + 50.0f), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket8[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 1.0f, 3.0f, 1.0f, type);

    mat4 drawRacket11 = translate(mat4(1.0f), vec3(cubePosition.x, cubePosition.y + 3.28f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 0.0f, 1.0f)) *
        scale(mat4(1.0f), vec3(scal, scal, scal));
    glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &drawRacket11[0][0]);
    createVertieces(0.0f, 0.0f, 0.0f, 3.0f, 1.0f, 1.0f, type);




    for (int i = 0; i < 5; i++) {
        glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f);
        mat4 net = translate(mat4(1.0f), vec3(cubePosition.x - (0.46 - (i * 0.1)), cubePosition.y + 2.5f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 0.0f, 1.0f)) *
            scale(mat4(1.0f), vec3(scal, scal, scal));
        glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &net[0][0]);
        createVertieces(0.0f, 0.0f, 0.0f, 0.25f, 14.1f, 0.25f, type);


        mat4 net2 = translate(mat4(1.0f), vec3(cubePosition.x + (0.46 - (i * 0.1)), cubePosition.y + 2.5f, cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 0.0f, 1.0f)) *
            scale(mat4(1.0f), vec3(scal, scal, scal));
        glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &net2[0][0]);
        createVertieces(0.0f, 0.0f, 0.0f, 0.25f, 14.1f, 0.25f, type);

    }

    for (int i = 0; i < 12; i++) {
        glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f);
        mat4 net3 = translate(mat4(1.0f), vec3(cubePosition.x, cubePosition.y + (2.0 + (i * 0.1)), cubePosition.z)) * rotate(mat4(1.0f), radians(rotation), vec3(0.0f, 0.0f, 1.0f)) *
            scale(mat4(1.0f), vec3(scal, scal, scal));
        glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &net3[0][0]);
        createVertieces(0.0f, 0.0f, 0.0f, 9.0f, 0.25f, 0.25f, type);

    }



}

void creatObject(int color, int worldMatrixLocation, vec3 cubePosition, float rotation, float scal, int type) {

    drawUpperArm(color, worldMatrixLocation, cubePosition, rotation, scal, type);
    drawLowerArm(color, worldMatrixLocation, cubePosition, rotation, scal, type);
    drawRacket(color, worldMatrixLocation, cubePosition, rotation, scal, type);

}




int main()
{
    if (!glfwInit())
    {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Grid", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        glfwTerminate();
        return -1;
    }

    // Define the vertex and fragment shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    checkShaderCompilation(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    checkShaderCompilation(fragmentShader);

    // Create the shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkShaderLinking(shaderProgram);

    //GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");

    // Create the vertex buffer object (VBO) and vertex array object (VAO)
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    // Bind the VAO and VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Draw the coordinate axes
    glUseProgram(shaderProgram);

    // Initialize camera variables
    vec3 cameraPosition(0.0f, 5.0f, 12.0f);  // Camera position at the center of the grid
    vec3 cameraTarget(0.0f, 0.0f, 0.0f);   // Camera target along the negative z-axis
    vec3 cameraUp(0.0f, 2.0f, 0.0f);        // Up vector
    float cameraHorizontalAngle = 90.0f;
    float cameraVerticalAngle = 0.0f;

    // Enable Backface culling
    glEnable(GL_CULL_FACE);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Projection matrix
    mat4 projection = perspective(radians(45.0f), static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 10000.0f);

    // Compute the view matrix
    mat4 view = lookAt(cameraPosition, cameraTarget, cameraUp);

    // Variables to be used later in tutorial
    float angle = 0;
    float rotationSpeed = 180.0f;  // 180 degrees per second
    float lastFrameTime = glfwGetTime();

    vec3 eye(0.0f, 3.0f, 10.0f);
    vec3 center(0.0f, 0.0f, 0.0f);
    float movementSpeed = 0.5f;

    // Set the projection matrix uniform
    GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, value_ptr(projection));

    // Set the view matrix uniform
    GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));


    float spinningCubeAngle = 0.0f;
    // Define and upload geometry to the GPU here ...
    int vao = createVertexBufferObject();

    vec3 cubePosition(0.0f, 0.0f, 2.0f);
    float rotation = 00.0f;
    float scal = 0.1f;
    float direction = 0.0f;
    int type = 3;

    float cameraSpeed = 1.0f;

    double lastMousePosX, lastMousePosY;
    glfwGetCursorPos(window, &lastMousePosX, &lastMousePosY);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {

        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;
        // Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw geometry
        // Draw geometry
        glBindVertexArray(vao);

        // Draw ground
        mat4 groundWorldMatrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, 0.0f)) * scale(mat4(1.0f), vec3(1.0f, 0.02f, 1.0f));
        GLuint worldMatrixLocation = glGetUniformLocation(shaderProgram, "worldMatrix");
        glUniformMatrix4fv(worldMatrixLocation, 1, GL_FALSE, &groundWorldMatrix[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, 36); // 36 vertices, starting at index 0




        GLint colorLocation = glGetUniformLocation(shaderProgram, "color");
        glUniform3f(colorLocation, 1.0f, 1.0f, 0.0f); // Yellow color
        // Draw the grid
        drawGrid();



        // Draw the X-axis line (red)  height 0.5
        glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f); // Red 
        drawLine(0.0f, 0.0f, 0.0f, LINE_LENGTH / 2.0f, 0.0f, 0.0f, 0.5f);

        // Draw the Y-axis line (green)  height 1.0
        glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f); // Green 
        drawLine(0.0f, 50.0f, 0.0f, 0.0f, LINE_LENGTH / 2.0f, 0.0f, 1.0f);

        // Draw the Z-axis line (blue) height 1.5
        glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f); // Blue 
        drawLine(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, LINE_LENGTH / 2.0f, 1.5f);



        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            angle = rotationSpeed * dt;

            vec3 direction(0.0f, 1.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));

            glm::mat4 view = glm::lookAt(eye,  // eye
                center,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        }

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            angle = -rotationSpeed * dt;

            vec3 direction(0.0f, 1.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));

            glm::mat4 view = glm::lookAt(eye,  // eye
                center,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            angle = rotationSpeed * dt;

            vec3 direction(1.0f, 0.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));

            glm::mat4 view = glm::lookAt(eye,  // eye
                center,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            angle = -rotationSpeed * dt;

            vec3 direction(1.0f, 0.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));

            glm::mat4 view = glm::lookAt(eye,  // eye
                center,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        }



        // Draw the coordinate axes
                // Set the color uniform to yellow (R=1.0, G=1.0, B=0.0)

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            cubePosition = getRandomGridPosition();
        }

        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
            direction += 0.0001f;
            vec3 newPos(cubePosition.x + 0.001f, cubePosition.y + 0.001f, cubePosition.z);
            cubePosition = newPos;
            scal += 0.00001f;
        }
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
            direction -= 0.0001f;
            vec3 newPos(cubePosition.x + 0.001f, cubePosition.y - 0.001f, cubePosition.z);
            cubePosition = newPos;
            scal -= 0.00001f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            vec3 newPos(cubePosition.x - 0.01f, cubePosition.y, cubePosition.z);
            cubePosition = newPos;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            vec3 newPos(cubePosition.x + 0.01f, cubePosition.y, cubePosition.z);
            cubePosition = newPos;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            vec3 newPos(cubePosition.x, cubePosition.y + 0.01f, cubePosition.z);
            cubePosition = newPos;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            vec3 newPos(cubePosition.x, cubePosition.y - 0.01f, cubePosition.z);
            cubePosition = newPos;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            rotation += 10;
            std::cout << cubePosition.x << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            type = 1;
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
            type = 2;
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            type = 3;
        }


        creatObject(colorLocation, worldMatrixLocation, cubePosition, rotation, scal, type);



        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    // Terminate GLFW
    glfwTerminate();

    return 0;
}