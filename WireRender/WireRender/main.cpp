//
//  main.cpp
//  WireRender
//
//  Created by Chuanheng He on 2016-10-29.
//  Student ID:250825743
//  Copyright © 2016 Chuanheng He. All rights reserved.
//


#include <iostream>
#include "basics.h"
#include <glm/gtx/rotate_vector.hpp>
#include "Camera.h"
using namespace std;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();

// Window dimensions
const GLuint WIDTH = 1200, HEIGHT = 800;

int start = 0;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX  =  WIDTH  / 2.0;
GLfloat lastY  =  HEIGHT / 2.0;
bool    keys[1024];

// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

// Light attributes

//This shader is just basic to get 3d into 2d screen
const GLchar* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"gl_Position = projection * view * model * vec4(position, 1.0f);\n"
"}\0";

const GLchar* vertexShaderSource2 = "#version 330 core\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 normal;\n"
"out vec3 Normal;\n"
"out vec3 FragPos;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"gl_Position = projection * view * model * vec4(position, 1.0f);\n"
"FragPos = vec3(model * vec4(position, 1.0f));\n"
"Normal = mat3(transpose(inverse(model))) * normal;\n"
"}\0";


//different fragment shaders for different purpose

const GLchar* fragmentShaderSource = "#version 330 core\n"
"out vec4 color;\n"
"uniform vec3 objectColor;\n"
"uniform vec3 lightColor;\n"
"void main()\n"
"{\n"
"color = vec4(lightColor * objectColor, 1.0f);\n"
"}\0";

//This one is applied to the object
const GLchar* fragmentShaderSource2 = "#version 330 core\n"
"struct Material {\n"
"vec3 ambient;\n"
"vec3 diffuse;\n"
"vec3 specular;\n"
"float shininess;\n"
"};\n"
"struct Light { \n"
"vec3 position;\n"
"vec3 ambient;\n"
"vec3 diffuse;\n"
"vec3 specular;\n"
"};\n"

"out vec4 color;\n"
"in vec3 Normal;\n"
"in vec3 FragPos;\n"
"uniform vec3 viewPos;\n"
"uniform Light light;\n"
"uniform Material material;\n"
"void main()\n"
"{\n"
"vec3 ambient = light.ambient * material.ambient;\n"
"vec3 norm = normalize(Normal);\n"
"vec3 lightDir = normalize(light.position - FragPos);\n"
"float diff = max(dot(norm, lightDir), 0.0);\n"
"vec3 diffuse = light.diffuse * (diff * material.diffuse);\n"
"vec3 viewDir = normalize(viewPos - FragPos);\n"
"vec3 reflectDir = reflect(-lightDir, norm);\n"
"float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
"vec3 specular = light.specular * (spec * material.specular);\n"
"vec3 result = ambient + diffuse + specular;\n"
"color = vec4(result, 1.0f);\n"
"}\0";

//This is the fragment shader for the light source
const GLchar* fragmentShaderSource3 = "#version 330 core\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = vec4(1.0f);\n"
"}\0";
//changing light source color
const GLchar* fragmentShaderSource4 = "#version 330 core\n"
"out vec4 color;\n"
"uniform vec4 currentColor;\n"
"void main()\n"
"{\n"
"color = currentColor;\n"
"}\0";




int main(int argc, const char * argv[]) {
    
    // setting up GLFW process
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "WireMesh", nullptr, nullptr);
    
    //Listen to the key call back
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    if(window == nullptr) {
        cout << "Failed to create a window" << endl;
        glfwTerminate();
        return -1;
    }
    
    glewExperimental = GL_TRUE;
    glfwMakeContextCurrent(window);
    
    // setting up the GLEW
    
    glewInit();
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK) {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    //enaple depth testing
    glEnable(GL_DEPTH_TEST);
    

    //Setting up the shaders that we use
    // compile the shader
    GLint success;
    GLchar infoLog[512];
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    //Next we attach the shader source code to the shader object and compile the shader:
    glShaderSource(vertexShader, 1, &vertexShaderSource2, NULL);
    glCompileShader(vertexShader);
    
    GLuint fragShader;
    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader,1,&fragmentShaderSource2,NULL);
    glCompileShader(fragShader);

    // Shader Program
    GLuint ourProgram;
    ourProgram = glCreateProgram();
    // attch the shader to program created
    glAttachShader(ourProgram,vertexShader);
    glAttachShader(ourProgram,fragShader);
    glLinkProgram(ourProgram);
    
    // check if the link is succesfful
    glGetProgramiv(ourProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(ourProgram, 512, NULL, infoLog);
    }
    
    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fragmentShaderSource4,NULL);
    glCompileShader(fragmentShader);
    // Shader Program for the lamp
    GLuint LampProgram;
    LampProgram = glCreateProgram();
    // attch the shader to program created
    glAttachShader(LampProgram,vertexShader);
    glAttachShader(LampProgram,fragmentShader);
    glLinkProgram(LampProgram);
    
    //This is the light source coordinates
    GLfloat lamp[] = {
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        
        -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        
        -0.5f, -0.5f, -0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
        
        -0.5f,  0.5f, -0.5f,
        0.5f,  0.5f, -0.5f,
        0.5f,  0.5f,  0.5f,
        0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };
    
    // reading the points into vector and also apply rotation
    vector<vector<vertex>> pointsdate = readProfiles();
    
    // The drawing index
    GLuint indices[] = {
        0, 1, 3,
        0, 2, 3
    };
    // triangulate step
    //*********************************
    int initialPointNum = pointsdate.size();
    int recNum = initialPointNum*ROTATENUM;
    GLuint VBOs[recNum],VAOs[recNum],EBOs[recNum];
    glGenVertexArrays(recNum,VAOs); // Generate multiple VAOs
    glGenBuffers(recNum,VBOs);
    glGenBuffers(recNum,EBOs);


// looping through the data points and build buffer objects of them..
    int counter = 0;
  for(int i = 0; i+1 < pointsdate.size(); i = i +1) {
    for(int j = 0; j < ROTATENUM ; j = j + 1) {
// get points and store them into vertice array as individual rectangle
// also calcualte the normal vectors and store them into vertices
        glm::vec3 firstPoint = pointsdate[i][j].position;
        glm::vec3 secondPoint = pointsdate[i+1][j].position;
        glm::vec3 thirdPoint = pointsdate[i][j+1].position;
        glm::vec3 fouthPoint = pointsdate[i+1][j+1].position;
        glm::vec3 normal = getNormal(firstPoint, secondPoint, thirdPoint);
        
        GLfloat vertices[] {
            firstPoint.x,firstPoint.y,firstPoint.z,normal.x,normal.y,normal.z,
            secondPoint.x,secondPoint.y,secondPoint.z,normal.x,normal.y,normal.z,
            thirdPoint.x,thirdPoint.y,thirdPoint.z,normal.x,normal.y,normal.z,
            fouthPoint.x,fouthPoint.y,fouthPoint.z,normal.x,normal.y,normal.z
        };
// Bind the buffer object
        glBindVertexArray(VAOs[counter]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[counter]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBOs[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
//      position attributes
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);
        // Normal attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
        glBindVertexArray(0);
        counter++;
    }
    }
    
    // setting up the VAO, VBO for the light source
    GLuint lightVAO,lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER,lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lamp), lamp, GL_STATIC_DRAW);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    // Set the vertex attributes 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    

    //Random colors fun
    random_device rd;
    mt19937 gen(rd());
    glm::vec3 lightColor = glm::vec3(1.0f,1.0f,1.0f);
    float r,g,b;
    float z = 1.0f;
    glm::vec3 lightPos(1.2f, 1.0f, 1.0f);
     GLfloat timepass = 0.0f;
    //Game loop
    while (!glfwWindowShouldClose(window))
    {
        
        // Calculate deltatime of current frame
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        // Check if any events have been activiated (key pressed, mouse moved etc.) and call corresponding response functions
        glfwPollEvents();
        do_movement();
        
        // Clear the colorbuffer
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Use cooresponding shader when setting uniforms/drawing objects
        glUseProgram(ourProgram);
        
        // rotating lightSource
        if(start) {
        float rotateDegree = glfwGetTime()*0.05f;
        lightPos = glm::rotateZ(lightPos, rotateDegree);
        GLint lightPosLoc = glGetUniformLocation(ourProgram, "lightPos");
        glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
        int timeStep1 = (int)glfwGetTime();
        if(timeStep1%3==0) {
            if(timeStep1!=(int)timepass) {
        r = (generate_canonical<float, 3>(gen)) ;
        g = (generate_canonical<float, 3>(gen));
        b = (generate_canonical<float, 3>(gen));
        z = (generate_canonical<float, 3>(gen));
        // Set lights properties
        lightColor = glm::vec3(r,g,b);
            timepass = glfwGetTime();
            }
            
        }
        }
        
        glm::vec3 diffuseColor = lightColor * glm::vec3(diffuseColorEffect); // Decrease the influence
        glm::vec3 ambientColor = diffuseColor * glm::vec3(ambientColorEffect); // Low influence
        
        //setting the lights attributes
        glUniform3f(glGetUniformLocation(ourProgram, "light.position"),  -lightPos.x, -lightPos.y, -lightPos.z);
        glUniform3f(glGetUniformLocation(ourProgram, "light.ambient"),  ambientColor.x, ambientColor.y, ambientColor.z);
        glUniform3f(glGetUniformLocation(ourProgram, "light.diffuse"),  diffuseColor.x, diffuseColor.y, diffuseColor.z);
        glUniform3f(glGetUniformLocation(ourProgram, "light.specular"), 1.0f, 1.0f, 1.0f);
        
        // getting the materials location
        GLint matAmbientLoc  = glGetUniformLocation(ourProgram, "material.ambient");
        GLint matDiffuseLoc  = glGetUniformLocation(ourProgram, "material.diffuse");
        GLint matSpecularLoc = glGetUniformLocation(ourProgram, "material.specular");
        GLint matShineLoc    = glGetUniformLocation(ourProgram, "material.shininess");
        
        //emerald vase
        //set the material properties
        glUniform3f(matAmbientLoc,  0.0215f, 0.1745f, 0.0215f);
        glUniform3f(matDiffuseLoc,   0.07568f, 0.61424f, 0.07568f);
        glUniform3f(matSpecularLoc,  0.633f, 0.727811f, 0.633f);
        glUniform1f(matShineLoc,    0.6f);

        //model, view, projection
        glm::mat4 view;
        view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
        // Get the uniform locations
        GLint modelLoc = glGetUniformLocation(ourProgram, "model");
        GLint viewLoc  = glGetUniformLocation(ourProgram,  "view");
        GLint projLoc  = glGetUniformLocation(ourProgram,  "projection");
        // Pass the matrices to the shader
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        
        // Draw the vase(or many vases?)
        
        for(int num = 0;num < recNum; num++) {
        glBindVertexArray(VAOs[num]);
        glm::mat4 model;
        model = glm::scale(model,glm::vec3(0.05f, 0.05f, 0.05f));
        model = glm::rotate(model, 270.0f, glm::vec3(1.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glDrawElements(GL_TRIANGLES, 6,GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        }
        
        // Also draw the lamp object, again binding the appropriate shader
        glUseProgram(LampProgram);
        // Get location objects for the matrices on the lamp shader (these could be different on a different shader)
        modelLoc = glGetUniformLocation(LampProgram, "model");
        viewLoc  = glGetUniformLocation(LampProgram, "view");
        projLoc  = glGetUniformLocation(LampProgram, "projection");
        
        glUniform4f(glGetUniformLocation(LampProgram, "currentColor"), lightColor.x,lightColor.y,lightColor.z,z);
        // Set matrices
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glm::mat4 model; model = glm::mat4();
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.25f)); // Make it a smaller cube
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        // Draw the light object (using light's vertex attributes)
        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 40);
        glBindVertexArray(0);

        // Swap the screen buffers
        glfwSwapBuffers(window);
        
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();
    return 0;
}


// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if(key== GLFW_KEY_E && action == GLFW_PRESS) {
        start = 1;
    }
    if(key== GLFW_KEY_R && action == GLFW_PRESS) {
        start = 0;
    }
    if (key >= 0 && key < 1024)
    {
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

// method to control the movement

void do_movement()
{
    // Camera controls
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

bool firstMouse = true;

// method for mouse control
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    GLfloat xoffset = xpos - lastX;
    GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left
    
    lastX = xpos;
    lastY = ypos;
    
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

