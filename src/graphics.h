#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <string>
#include <vector>

class GLFWwindow;


class Graphics
{
private:
    enum SHADER_TYPE 
    {
        VERTEX_SHADER = 0,
        FRAGMENT_SHADER
    };

    enum SHADER_PURPOSE
    {
        COLOR_SHADER = 0,
        COLOR_SHADER2,
        SIZE
    };
    struct Shader
    {
        Shader(std::string const&, std::string const&, SHADER_PURPOSE const);

        std::string vString;
        std::string fString;
        SHADER_PURPOSE shaderPurpose;
        unsigned int vertexShader;
        unsigned int fragmentShader;
        unsigned int shaderProgram;
    };

public:
    Graphics(int const, int const);
    ~Graphics();

    int initialize();
    void update();
    void release();

private:
    void processInput(GLFWwindow*);
    int compileShader(Shader&);
    int compileIndividualShader(Shader&, SHADER_TYPE const);
    int linkShaderProgram(Shader&);
    int createShader(unsigned int&, unsigned int&, unsigned int&);
    int initializeShaders();
    void initializeVertices();

    int width;
    int height;
    GLFWwindow* window;
    unsigned int VAO_left;
    unsigned int VAO_right;
    unsigned int VBO_left;
    unsigned int VBO_right;
    unsigned int EBO;

    std::vector<Shader> shaders;
    
    static const int TRIANGLE_LIMIT = 9;
    static const int TRIANGLE_INDICES = 3;
    float vertices_left[TRIANGLE_LIMIT];
    float vertices_right[TRIANGLE_LIMIT];
    float indices[TRIANGLE_INDICES];
};

#endif
