//
//  Shader.hpp
//  Lab3
//
//  Created by CGIS on 05/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//
#ifndef Shader_hpp
#define Shader_hpp

#include <stdio.h>
#include <iostream>
#include "GLEW/glew.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <detail/type_vec3.hpp>
#include <mat3x2.hpp>

namespace gps {

class Shader
{
public:
    GLuint shaderProgram;
    void loadShader(std::string vertexShaderFileName, std::string fragmentShaderFileName);
    void useShaderProgram();

	//utility uniform functions
	void setBool(const std::string &name, bool value);
	void setInt(const std::string &name, int value);
	void setFloat(const std::string &name, float value);
	void setVec3(const std::string &name, glm::vec3 value);
	void setMat3(const std::string &name, glm::mat3 value);
	void setMat4(const std::string &name, glm::mat4 value);

private:
    std::string readShaderFile(std::string fileName);
    void shaderCompileLog(GLuint shaderId);
    void shaderLinkLog(GLuint shaderProgramId);
};

}

#endif /* Shader_hpp */
