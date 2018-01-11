//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"
#include <iostream>

namespace gps {
    
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
    {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        this->cameraUpDirection = glm::vec3(0.0f, 1.0f, 0.0f);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, this->cameraUpDirection));
        this->worldUpDirection = this->cameraUpDirection;
    }

    glm::mat4 Camera::getViewMatrix()
    {
//        return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , glm::vec3(0.0f, 1.0f, 0.0f));
		return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , cameraUpDirection);
    }
    
    void Camera::move(MOVE_DIRECTION direction, float speed)
    {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += cameraDirection * speed;
                break;
                
            case MOVE_BACKWARD:
                cameraPosition -= cameraDirection * speed;
                break;
                
            case MOVE_RIGHT:
                cameraPosition += cameraRightDirection * speed;
                break;
                
            case MOVE_LEFT:
                cameraPosition -= cameraRightDirection * speed;
                break;
        }
		//std::cout << "Camera at: x: " << cameraPosition.x <<  ", y: " << cameraPosition.y << ", z:" << cameraPosition.z << std::endl;
		//cameraPosition.y = 0.0f;
    }
    
    void Camera::rotate(float pitch, float yaw)
    {
        glm::vec3 front;
        front.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
        front.y = glm::sin(glm::radians(pitch));
        front.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));

        cameraDirection = glm::normalize(front);
        cameraRightDirection = glm::normalize(glm::cross(cameraDirection, worldUpDirection));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraDirection));
    }

    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }

}
