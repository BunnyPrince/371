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
const int GRID_WIDTH = 78;
const int GRID_HEIGHT = 36;
const float GRID_CELL_SIZE = 0.2f;

const int NET_WIDTH = 84;
const int NET_HEIGHT = 18;
const float NET_CELL_SIZE = 0.1f;



//axis line
const float LINE_LENGTH = 1.0f;
GLuint shaderProgram;

// placement of the object created at the center
vec3 modelPosition(0.0f, 0.0f, 0.0f);


//class Letters {
//    public: {
//        Letters() {
//
//        }
//
//      }
//    public:{
//
//
//    }
//}

// Define the vertices of the unit cube 
//using grid width to create the cube vertices
float cubeVertices[] = {
        // Front face
        -(GRID_WIDTH/2), 0.0f,(GRID_WIDTH / 2),  // Bottom-left
       (GRID_WIDTH / 2), 0.0f, (GRID_WIDTH / 2),   // Bottom-right
       (GRID_WIDTH / 2), (GRID_WIDTH / 2),(GRID_WIDTH / 2),    // Top-right
        -(GRID_WIDTH / 2),(GRID_WIDTH / 2), (GRID_WIDTH / 2),   // Top-left
    
        // Back face
        -(GRID_WIDTH / 2), 0.0f, -(GRID_WIDTH / 2), // Bottom-left
        (GRID_WIDTH / 2), 0.0f, -(GRID_WIDTH / 2),  // Bottom-right
        (GRID_WIDTH / 2), (GRID_WIDTH / 2), -(GRID_WIDTH / 2),   // Top-right
        -(GRID_WIDTH / 2), (GRID_WIDTH / 2), -(GRID_WIDTH / 2),  // Top-left
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

// Vertex shader source code
const char* vertexShaderSource = R"(
    #version 330 core

    uniform mat4 projection;
    uniform mat4 view;
    layout (location = 0) in vec3 position;

    void main()
    {
        gl_Position = projection * view * vec4(position, 1.0);
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
    float gridVertices[((GRID_WIDTH + 1) * 2 +(GRID_HEIGHT+1)*2) * 3];
    int index = 0;

    // this loop is to make the  vertical line 
    for (int i = 0; i <= GRID_WIDTH; ++i)
    {
        float xPos = (i - GRID_WIDTH / 2) * GRID_CELL_SIZE;

        // Vertical lines
        gridVertices[index++] = -GRID_HEIGHT / 2 * GRID_CELL_SIZE;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = xPos;

        gridVertices[index++] = GRID_HEIGHT / 2 * GRID_CELL_SIZE;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = xPos;
        
    }

    // this loop is to make the  horizontal line 
    for (int i = 0; i <= GRID_HEIGHT; ++i)
    {
        float xPos = (i - GRID_HEIGHT / 2) * GRID_CELL_SIZE;

        // Horizontal lines
        gridVertices[index++] = xPos;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = -GRID_WIDTH / 2 * GRID_CELL_SIZE;

        gridVertices[index++] = xPos;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = GRID_WIDTH / 2 * GRID_CELL_SIZE;

    }

    // Set the vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW);

    // Set the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    //set the line width bigger then default
    glLineWidth(2);
    // Draw the grid lines
    glDrawArrays(GL_LINES, 0, (GRID_WIDTH + 1) * 4);

    // Unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Delete the VBO and VAO to release resources
    glDeleteBuffers(1, &gridVBO);
    glDeleteVertexArrays(1, &gridVAO);
}


void drawNet() {
    // Create the vertex buffer object (VBO) and vertex array object (VAO) for the grid
    GLuint gridVBO, gridVAO;
    glGenBuffers(1, &gridVBO);
    glGenVertexArrays(1, &gridVAO);

    // Bind the VAO and VBO
    glBindVertexArray(gridVAO);
    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);

    // Generate the grid vertices 
    // since we have x,y and z is multiplide by 3
    //since we only one haft of the hight we don't multiply the height by 2
    float gridVertices[((NET_WIDTH + 1) * 2 + (NET_HEIGHT + 2)) * 3];
    int index = 0;

    // this loop is to make the  vertical line 
    for (int i = 0; i <= NET_HEIGHT; ++i)
    {
        float yPos = (i - NET_HEIGHT / 2) * NET_CELL_SIZE;

        if (yPos >= 0) {
            // Vertical lines
            gridVertices[index++] = -NET_WIDTH / 2 * NET_CELL_SIZE;
            gridVertices[index++] = yPos;
            gridVertices[index++] = 0.0f;

            gridVertices[index++] = NET_WIDTH / 2 * NET_CELL_SIZE;
            gridVertices[index++] = yPos;
            gridVertices[index++] = 0.0f;
        }
    }

    // this loop is to make the  horizontal line 
    for (int i = 0; i <= NET_WIDTH; ++i)
    {
        float xPos = (i - NET_WIDTH / 2) * NET_CELL_SIZE;

        // Horizontal lines
        gridVertices[index++] = xPos;
        gridVertices[index++] = 0.0f;
        gridVertices[index++] = 0.0f;

        gridVertices[index++] = xPos;
        gridVertices[index++] = NET_HEIGHT / 2 * NET_CELL_SIZE;
        gridVertices[index++] = 0.0f;

    }

    // Set the vertex data
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW);

    // Set the vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // Draw the grid lines
    glDrawArrays(GL_LINES, 0, (NET_WIDTH + 1) * 4);

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
         startX + 4, startY + 1, startZ + 2,
        endX + 4, endY + 1, endZ + 2
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
void drawUnitCube(const float cube[], vec3 position)
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

    // Draw the cube using indexed vertices
    glDrawElements(GL_TRIANGLES, sizeof(cubeIndices) / sizeof(unsigned int), GL_UNSIGNED_INT, nullptr);

    // Unbind the VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Delete the VBO, VAO, and EBO to release resources
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeEBO);
}

void createVertieces(const float x, const float y, const float z, const float sizeX, const float sizeY, const float sizeZ) {
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
    drawUnitCube(vetices, position);

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
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkShaderLinking(shaderProgram);

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

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Projection matrix
    mat4 projection = perspective(radians(45.0f), static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 1000.0f);

    // Compute the view matrix
    mat4 view = lookAt(cameraPosition, cameraTarget, cameraUp);

    // Variables to be used later in tutorial
    float angle = 0;
    float rotationSpeed = 180.0f;  // 180 degrees per second
    float lastFrameTime = glfwGetTime();

    vec3 eye(0.0f, 3.0f, 12.0f);
    vec3 center(0.0f, 0.0f, 0.0f);
    float movementSpeed = 0.5f;
    
    // Set the projection matrix uniform
    GLint projectionLocation = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, value_ptr(projection));

    // Set the view matrix uniform
    GLint viewLocation = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));


    // the position of the rackets
    float places[4][3] = {
            {-2.5, 0, 6},
            {2.5, 0, 6},
            {2.5, 0, -6},
            {-2.5, 0, -6}
    };


    // Main loop
    while (!glfwWindowShouldClose(window))
    {

        float dt = glfwGetTime() - lastFrameTime;
        lastFrameTime += dt;
        // Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader program
        glUseProgram(shaderProgram);

        
        //used lab 2 to see how the glfw key works
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
            vec3 pos(places[0][0], places[0][1] + 3, places[0][2] + 5);
            vec3 cen(places[0][0], places[0][1], places[0][2]);

            glm::mat4 view = glm::lookAt(pos,  // eye
                cen,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        }
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
            vec3 pos(places[1][0], places[1][1]+3, places[1][2]+5);
            vec3 cen(places[1][0], places[1][1], places[1][2]);

            glm::mat4 view = glm::lookAt(pos,  // eye
                cen,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        }
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
            vec3 pos(places[2][0], places[2][1] + 3, places[2][2] - 5);
            vec3 cen(places[2][0], places[2][1], places[2][2]);

            glm::mat4 view = glm::lookAt(pos,  // eye
                cen,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        }
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
            vec3 pos(places[3][0], places[3][1] + 3, places[3][2] - 5);
            vec3 cen(places[3][0], places[3][1], places[3][2]);

            glm::mat4 view = glm::lookAt(pos,  // eye
                cen,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            angle = rotationSpeed * dt;

            vec3 direction(0.0f, 1.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));
            
            glm::mat4 view = glm::lookAt(eye,  // eye
                                                center,  // center
                                                glm::vec3( 0.0f, 1.0f, 0.0f) );// up
            
            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
            std::cout << 's' << std::endl;
        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            angle = -rotationSpeed * dt;

            vec3 direction(0.0f, 1.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));

            glm::mat4 view = glm::lookAt(eye,  // eye
                center,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
            std::cout << 's' << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            angle = rotationSpeed * dt;

            vec3 direction(1.0f,0.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));

            glm::mat4 view = glm::lookAt(eye,  // eye
                center,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
            std::cout << 's' << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            angle = -rotationSpeed * dt;

            vec3 direction(1.0f, 0.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));

            glm::mat4 view = glm::lookAt(eye,  // eye
                center,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
            std::cout << 's' << std::endl;
        }


        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            angle = rotationSpeed * dt;

            vec3 direction(0.0f, 1.0f, 0.0f);
            eye = eye * cos(glm::radians(angle)) + cross(direction, eye) * sin(glm::radians(angle));
            center = center * cos(glm::radians(angle)) + cross(direction, center) * sin(glm::radians(angle));

            glm::mat4 view = glm::lookAt(eye,  // eye
                center,  // center
                glm::vec3(0.0f, 1.0f, 0.0f));// up

            GLuint viewLocation = glGetUniformLocation(shaderProgram, "view");
            glUniformMatrix4fv(viewLocation, 1, GL_FALSE, value_ptr(view));
            std::cout << 's' << std::endl;
        }



        createVertieces(0.0f, 2.5f, 6.0f, 0.15f, 2.0f, 0.15f);





        // Draw the coordinate axes
        GLint colorLocation = glGetUniformLocation(shaderProgram, "color");
        // Set the color uniform to yellow (R=1.0, G=1.0, B=0.0)
        glUniform3f(colorLocation, 1.0f, 1.0f, 0.0f); // Yellow color
        // Draw the grid
        drawGrid();

        glUniform3f(colorLocation, 0.0f, 0.0f, 0.0f); // Black
        drawNet();

        // Draw the X-axis line (red)  height 0.5
        glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f); // Red 
        drawLine(0.0f, 0.0f, 0.0f, LINE_LENGTH / 2.0f, 0.0f, 0.0f, 0.5f);

        // Draw the Y-axis line (green)  height 1.0
        glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f); // Green 
        drawLine(0.0f, 0.0f, 0.0f, 0.0f, LINE_LENGTH / 2.0f, 0.0f, 1.0f);

        // Draw the Z-axis line (blue) height 1.5
        glUniform3f(colorLocation, 0.0f, 0.0f, 1.0f); // Blue 
        drawLine(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, LINE_LENGTH / 2.0f, 1.5f);

        //Draw the Cube (sky blue) at the middle of the screen
        glUniform3f(colorLocation, 0.5f, 0.8f, 0.9f); // Sky Blue
        drawUnitCube(cubeVertices, modelPosition);


        int x = 0;
        int y = 0;
        int z = 0;

        for (int i = 0; i < 4; i++) {
            x = places[i][0];
            y = places[i][1];
            z = places[i][2];

            glUniform3f(colorLocation, 1.0f, 0.0f, 0.0f);  //Red 
            createVertieces(x, y+0.5f, z, 0.1f, 0.5f, 0.1f);

            glUniform3f(colorLocation, 1.0f, 0.3f, 0.0f); // Light Red
            createVertieces(x, y+1.2f, z, 0.45f, 0.1f, 0.1f);

            createVertieces(x+0.17f, y+1.0f, z, 0.1f, 0.35f, 0.1f);

            createVertieces(x-0.17f, y+1.0f, z, 0.1f, 0.35f, 0.1f);

            createVertieces(x, y+0.8f, z, 0.45f, 0.1f, 0.1f);

            glUniform3f(colorLocation, 0.0f, 1.0f, 0.0f); // Green
            createVertieces(x, y+1.0f, z, 0.35f, 0.45f, 0.05f);

        }

        

        //ARUBORIBARAN ==> taking A, R, U, B
        float col = 0.1;
        float dis = 0.2;

        //A
        x = places[0][0];
        y = places[0][1];
        z = places[0][2];
        for (int i = 0; i < 3; i++){ 
            glUniform3f(colorLocation, 1.0f - (col * i), (col * i) + 0.65f, 1.0f - (col * i));//Pink
            createVertieces(x + 0.3f, y + 1.8f, (dis * i) + z, 0.1f, 0.8f, 0.1f);
            createVertieces(x - 0.3f, y + 1.8f, (dis * i) + z, 0.1f, 0.8f, 0.1f);
            createVertieces(x + 0.0f, y + 2.25f, (dis * i) + z, 0.7f, 0.1f, 0.1f);
            createVertieces(x + 0.0f, y + 1.95f, (dis * i) + z, 0.7f, 0.1f, 0.1f);
        }

        //R
        x = places[1][0];
        y = places[1][1];
        z = places[1][2];
        for (int i = 0; i < 3; i++) {
            glUniform3f(colorLocation, (col* i) + 0.5f , 0.0f- (col * i), 0.5f - (col * i));//Purple
            createVertieces(x-0.3f, y+1.8f, (dis * i) + z, 0.1f, 0.8f, 0.1f);
            createVertieces(x, y+2.25f, (dis * i) + z, 0.7f, 0.1f, 0.1f);
            createVertieces(x, y+1.95f, (dis * i) + z, 0.53f, 0.1f, 0.1f);
            createVertieces(x+0.3f, y+2.15f, (dis * i) + z, 0.1f, 0.3f, 0.1f);
            createVertieces(x+0.3f, y+1.65f, (dis * i) + z, 0.1f, 0.5f, 0.1f);
        }

        //U
        x = places[2][0];
        y = places[2][1];
        z = places[2][2];
        for (int i = 0; i < 3; i++) {
            glUniform3f(colorLocation, (col* i) + 0.4f, (col* i) + 0.4f, (col* i) + 1.0f); //Light Purple
            createVertieces(x + 0.3f, y + 1.8f, z - (dis * i), 0.1f, 0.8f, 0.1f);
            createVertieces(x - 0.3f, y + 1.8f, z - (dis * i), 0.1f, 0.8f, 0.1f);
            createVertieces(x + 0.0f, y + 1.45f, z - (dis * i), 0.7f, 0.1f, 0.1f);
        }

        //B
        x = places[3][0];
        y = places[3][1];
        z = places[3][2];
        for (int i = 0; i < 3; i++) {
            glUniform3f(colorLocation, (col * i) + 0.3f, (col * i) + 0.3f, (col * i) + 0.3f); //Grey
            createVertieces(x + 0.3f, y + 1.8f, z - (dis * i), 0.1f, 0.8f, 0.1f);
            createVertieces(x, y + 2.25f, z - (dis * i), 0.7f, 0.1f, 0.1f);
            createVertieces(x, y + 1.85f, z - (dis * i), 0.53f, 0.1f, 0.1f);
            createVertieces(x, y + 1.45f, z - (dis * i), 0.7f, 0.1f, 0.1f);
            createVertieces(x - 0.3f, y + 2.1f, z - (dis * i), 0.1f, 0.38f, 0.1f);
            createVertieces(x - 0.3f, y + 1.6f, z - (dis * i), 0.1f, 0.38f, 0.1f);
        }

        //The tennis court
        glUniform3f(colorLocation, 0.0f, 0.5f, 0.0f); // Dark Green
        createVertieces(0.0f, 0.02f, 0.0f, 7.2f, 0.0f,15.6f );

        //the tennis court lines
        glUniform3f(colorLocation, 1.0f, 1.0f, 1.0f); // White
        createVertieces(0.0f, 0.025f, 7.8f, 7.2f, 0.0f, 0.2f);
        createVertieces(0.0f, 0.025f, -7.8f, 7.2f, 0.0f, 0.2f);
        createVertieces(3.6f, 0.025f, 0.0f, 0.1, 0.0f, 15.6f);
        createVertieces(-3.6f, 0.025f, 0.0f, 0.1, 0.0f, 15.6f);
        createVertieces(2.8f, 0.025f, 0.0f, 0.1, 0.0f, 15.6f);
        createVertieces(-2.8f, 0.025f, 0.0f, 0.1, 0.0f, 15.6f);
        createVertieces(0.0f, 0.025f, 0.0f, 0.07f, 0.0f, 7.8f);
        createVertieces(0.0f, 0.025f, 3.9f, 5.6f, 0.0f, 0.07f);
        createVertieces(0.0f, 0.025f, -3.9f, 5.6f, 0.0f, 0.07f);

        //The tennis net details
        createVertieces(0.0f, 0.9f, 0.0f, 8.4f, 0.05f, 0.05f);
        glUniform3f(colorLocation, 0.2f, 0.2f, 0.2f); // Dark Grey
        createVertieces(4.2f, 0.0f, 0.0f, 0.15f, 2.0f, 0.15f);
        createVertieces(-4.2f, 0.0f, 0.0f, 0.15f, 2.0f, 0.15f);





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