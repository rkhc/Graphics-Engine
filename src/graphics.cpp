#include "graphics.h"
#include <iostream>
#include <glad/glad.h> 
#include <GLFW/glfw3.h>

//will update again
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

Graphics::Shader::Shader(std::string const& vShader, std::string const& fShader, SHADER_PURPOSE const p) : vString(vShader), fString(fShader), shaderPurpose(p), vertexShader(0), fragmentShader(0), shaderProgram(0)
{
    
}

Graphics::Graphics(int const w, int const h) : width(w), height(h), window(nullptr), VAO_left(0), VAO_right(0), VBO_left(0), VBO_right(0), EBO(0),
vertices_left{ -0.5f,0.5f, 0.0f, -1.0f, -0.5f, 0.0f, 0.0f, -0.5f, 0.0f }, vertices_right{ 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.0f, 1.0f, -0.5f, 0.0f }
{
    shaders.push_back(Shader("#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0", "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0);\n"
        "}\0", SHADER_PURPOSE::COLOR_SHADER));

    shaders.push_back(Shader("#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0",
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0);\n"
        "}\0", SHADER_PURPOSE::COLOR_SHADER2));
}

//destructor
Graphics::~Graphics()
{

}

int Graphics::initialize()
{
    //initialize and configure glfw using opengl 3.3 core profile
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Faild to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    //make this the main context
    glfwMakeContextCurrent(window);

    //initialize glad (helps find which opengl specifications we can use)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //let opengl know the size of the rendering window
    //first 2 parameters sets the drawing location from the bottom left of the window
    //next two parameters is the pixel width and height of rendeinrg window
    glViewport(0, 0, width, height);

    //once window is resized, we need to let opengl know as well
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    int result = 0;
    //initialize vertex and fragment shader
    result = initializeShaders();

    if (result == -1)
        return -1;

    if (shaders.size() != SHADER_PURPOSE::SIZE)
    {
        std::cout << "Invalid number of shaders provided" << std::endl;
        return -1;
    }

    initializeVertices();

    return 0;
}

void Graphics::update()
{

    //render loop untill told to close
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        //render frame
        //set state to be cleared to this color each frame
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //uses state to retrieve the clearing color to
        glClear(GL_COLOR_BUFFER_BIT);

        //sets the currect active shader program
        glUseProgram(shaders[SHADER_PURPOSE::COLOR_SHADER].shaderProgram);
        glBindVertexArray(VAO_left);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe drawing
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //full drawing
        glDrawArrays(GL_TRIANGLES, 0, 6); //replaced to draw from element buffer object
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(VAO_right);
        //glDrawArrays(GL_TRIANGLES, 0, 6); //replaced to draw from element buffer object
        glUseProgram(shaders[SHADER_PURPOSE::COLOR_SHADER2].shaderProgram);
        glBindVertexArray(VAO_right);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        //second buffer is to render next frame, once complete swap with front buffer
        glfwSwapBuffers(window);
        //input, mouse, window state update checks
        glfwPollEvents();
    }
}

void Graphics::release()
{
    glfwTerminate();
    for (size_t i = 0; i < shaders.size(); ++i)
    {
        glDeleteShader(shaders[i].vertexShader);
        glDeleteShader(shaders[i].fragmentShader);
    }
}


void Graphics::processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int Graphics::compileShader(Shader& obj)
{
    int result = 0;

    if (obj.vString == "" || obj.fString == "")
    {
        std::cout << "Shader information not provided" << std::endl;
        return -1;
    }
    
    //compile vertex shader
    result = compileIndividualShader(obj, SHADER_TYPE::VERTEX_SHADER);
    
    if (result == -1)
        return -1;

    //compile fragment shader
    result = compileIndividualShader(obj, SHADER_TYPE::FRAGMENT_SHADER);

    if (result == -1)
        return -1;

    //link both vertex and fragment to shader program
    result = linkShaderProgram(obj);
    
    if (result == -1)
        return -1;

    return 0;
}

int Graphics::compileIndividualShader(Shader& obj, SHADER_TYPE const type)
{
    unsigned int* shader = nullptr;
    char* charString = nullptr;

    if (type == SHADER_TYPE::VERTEX_SHADER)
    {
        shader = &obj.vertexShader;
        charString = const_cast<char*>(obj.vString.c_str());
    }
    else if (type == SHADER_TYPE::FRAGMENT_SHADER)
    {
        shader = &obj.fragmentShader;
        charString = const_cast<char*>(obj.fString.c_str());
    }

    if (!shader || !charString)
    {
        std::cout << "Could not compile shader" << std::endl;
        return -1;
    }

    //create a vertex shader object to compile the vertex shader
    if(type == SHADER_TYPE::VERTEX_SHADER)
        obj.vertexShader = glCreateShader(GL_VERTEX_SHADER);
    else if (type == SHADER_TYPE::FRAGMENT_SHADER)
        obj.vertexShader = glCreateShader(GL_FRAGMENT_SHADER);

    //attach and compile the vertex shader
    glShaderSource(obj.vertexShader, 1, &charString, NULL);
    glCompileShader(obj.vertexShader);

    //error checking the vertex shader
    int success = 0;
    char infoLog[512];
    glGetShaderiv(obj.vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(obj.vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::";
        if (type == SHADER_TYPE::VERTEX_SHADER)
            std::cout << "VERTEX";
        else if (type == SHADER_TYPE::FRAGMENT_SHADER)
            std::cout << "FRAGMENT";
        std::cout << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        return -1;
    }

    return 0;
}

int Graphics::linkShaderProgram(Shader& obj)
{
    int success = 0;
    char infoLog[512];

    //we have the vertex and fragment shader objects. now we need to link them to a shader program
    obj.shaderProgram = glCreateProgram();

    glAttachShader(obj.shaderProgram, obj.vertexShader);
    glAttachShader(obj.shaderProgram, obj.fragmentShader);
    glLinkProgram(obj.shaderProgram);

    memset(infoLog, 0, 512);
    glGetProgramiv(obj.shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(obj.shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::LINKED_FAILED\n" << infoLog << std::endl;
        return -1;
    }

    return 0;
}

int Graphics::initializeShaders()
{
    int result = 0;
    for (size_t i = 0; i < shaders.size(); ++i)
    {
        result = compileShader(shaders[i]);

        if (result == -1)
            return -1;
    }

    return 0;
}

int Graphics::createShader(unsigned int& shaderProgram, unsigned int& vertexShader, unsigned int& fragmentShader)
{
    int success = 0;
    char infoLog[512];

    //we have the vertex and fragment shader objects. now we need to link them to a shader program
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    memset(infoLog, 0, 512);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::LINKED_FAILED\n" << infoLog << std::endl;
        return -1;
    }

    return 0;
}

void Graphics::initializeVertices()
{
  

    //unsigned int indices[] =
    //{
    //    0,1,3,
    //    1,2,3
    //};
    //
    ////creating our Element Buffer Object to store the indices of which to draw the vertices
    //glGenBuffers(1, &EBO);


    /*most of the import initialization*/
    //generate VAO so we do not need to keep specifying the vertex interpretations each render
    glGenVertexArrays(1, &VAO_left);

    //use VAO by binding it, any vertex interpretation will be saved in this VAO
    glBindVertexArray(VAO_left);

    //creating our Vertex Buffer Object to store all the vertices
    glGenBuffers(1, &VBO_left);

    //assigning VBO as a type GL_ARRAY_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, VBO_left);
    //copy vertex information to VBO
    //GL_STATIC_DRAW as the triangle information will not move a lot and will need to be draw each time
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_left), vertices_left, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);   //likewise, 0 is from layout = 0

    glBindVertexArray(0);

    //for the right triangle
    glGenVertexArrays(1, &VAO_right);

    //use VAO by binding it, any vertex interpretation will be saved in this VAO
    glBindVertexArray(VAO_right);

    //creating our Vertex Buffer Object to store all the vertices
    glGenBuffers(1, &VBO_right);

    //assigning VBO as a type GL_ARRAY_BUFFER
    glBindBuffer(GL_ARRAY_BUFFER, VBO_right);
    //copy vertex information to VBO
    //GL_STATIC_DRAW as the triangle information will not move a lot and will need to be draw each time
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_right), vertices_right, GL_STATIC_DRAW);

    //bind the element buffer object to tell OpenGL which vertex indices to draw
    //can now replace GL_DRAW_ARRAY with GL_DRAW_ELEMENTS
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //tell OpenGL how to use the vertex buffer, reads 3 at a time to form a triangle
    //first param is 0 because we specified the layout = 0 in the vertex shader
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);   //likewise, 0 is from layout = 0

    /*end of most of the important initialization*/

}