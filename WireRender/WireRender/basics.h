//
//  basics.h
//  WireRender
//
//  Created by Chuanheng He on 2016-10-29.
//  Copyright © 2016 Chuanheng He. All rights reserved.
//

#ifndef basics_h
#define basics_h

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GLUT/glut.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gli/gli.hpp>
#include <random>

//some basic constants
using namespace std;
#define PI 3.14159265
const float ROTATEDEGREE = 2.0;
const int ROTATENUM = 360/ROTATEDEGREE;
const int VASENUM = 10;
const float diffuseColorEffect = 0.8f;
const float ambientColorEffect = 0.5f;

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct recFace {
    glm::vec3 position1;
    glm::vec3 position2;
    glm::vec3 position3;
    glm::vec3 position4;
    glm::vec3 normal;
};

// this method will take in three points and calculate the normal vector out of it
glm::vec3 getNormal(glm::vec3 point1,glm::vec3 point2,glm::vec3 point3) {
    glm::vec3 normal;
    glm::vec3 edge12 = point2 - point1;
    glm::vec3 edge13 = point3 - point1;
    glm::vec3 cross = glm::cross(edge12,edge13);
    normal = cross/ (glm::length(cross));
//    cout << edge12.x << edge12.y << edge12.z << endl;
    
    return normal;
};

vector<vector<vertex>> readProfiles() {
    // This is to read the initial points from the file
    // and store them into vector of vector
    vector<vector<vertex>> points;
    ifstream file("vase.txt");
    if(file.is_open()) {
        vector<vertex> point;
        string line;
        while (getline(file,line)) {
            istringstream iss(line);
            float xPoint,yPoint,zPoint,wPoint;
            while(iss >> xPoint>>yPoint>>zPoint>>wPoint){
                float xcoordinate = xPoint;
                float ycoordinate = yPoint;
                float zcoordinate = zPoint;
                
                vector<vertex> pointVector;
                glm::vec3 initialPoint = glm::vec3(xcoordinate,ycoordinate,zcoordinate);
                vertex newPoint;
                newPoint.position = initialPoint;
                pointVector.push_back(newPoint);
                points.push_back(pointVector);
            }
            //    read each singe number of the poin
        }
        file.close();
    }
    
    //    auto i = points[0][0];
    //    cout << sin(90.0*(PI/180)) << endl;
    //    cout << i.position.x << " " << i.position.y << " " << i.position.z << endl;
    //
    //  This is the rotation method around z axis, get the points after rotation and store them
    //  into vectors
    for(auto i = points.begin();i!=points.end();i++) {
        auto firstPoint = (*i).front();
        for(int j = 1;j<=ROTATENUM;j++) {
            float rx,ry,rz;
            float degree = j*ROTATEDEGREE;
            rx = firstPoint.position.x*cos(degree*(PI/180)) - firstPoint.position.y*sin(degree*(PI/180));
            ry = firstPoint.position.x*sin(degree*(PI/180)) + firstPoint.position.y*cos(degree*(PI/180));
            rz = firstPoint.position.z;
            glm::vec3 rotatePosition = glm::vec3(rx,ry,rz);
            vertex rotateVertex;
            rotateVertex.position = rotatePosition;
            (*i).push_back(rotateVertex);
        }
    }
    return points;
    
}



#endif /* basics_h */
