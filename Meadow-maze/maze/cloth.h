#ifndef CLOTH_H
#define CLOTH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <cmath>

#include "shader.h"
#include "model.h"

enum TIP { LOWER_LEFT, LOWER_RIGHT, UPPER_LEFT, UPPER_RIGHT };

class Cloth
{
	class Ball {
	  public:
		glm::vec3 position;
		glm::vec3 velocity;
		float mass;
		//Is this ball held in position?
		bool fixed;
		//Vertex normal for this ball
		glm::vec3 normal;
	};
	class Spring {
	  public:
		//Indices of the balls at either end of the spring
		int ball1;
		int ball2;
		//Tension in the spring
		float tension;
		float springConstant;
		float naturalLength;
		Spring() : ball1(-1), ball2(-1) {}
		~Spring() {}
	};
  private:
	// Grid complexity. This is the number of balls across and down in the model
	const int gridSize;
	// Values given to each spring
	float springConstant;
	float naturalLength;
	// Values given to each ball
	float mass;
	// Damping factor. Velocities are multiplied by this
	float dampFactor;

	// 2 arrays of balls
	int numBalls;
	Ball* balls1;
	Ball* balls2;
	// Pointers to the arrays. One holds the balls for the current frame, and one holds those
	// for the next frame
	Ball* currentBalls;
	Ball* nextBalls;
	// Array of springs
	int numSprings;
	Spring* springs;

 	// Force
	glm::vec3 FORCE = glm::vec3(0.0f, -0.98f, 0.0f);

    // cloth VAO
    unsigned int clothVAO;
    unsigned int clothVBO;
    float* clothVertices;

  public:
	Cloth(const int gridSize = 13, float springConstant = 15.0f, float naturalLength = 1.0f, float mass = 0.01f, float dampFactor = 0.9f);
	void reset();
	void update(float deltaTime);
	void draw();
	void unfix(TIP tip);
	~Cloth();
};


Cloth::Cloth(const int gridSize, float springConstant, float naturalLength, float mass, float dampFactor)
			: gridSize(gridSize), springConstant(springConstant), naturalLength(naturalLength), mass(mass), dampFactor(dampFactor) {
	// Calculate number of balls
	numBalls = gridSize * gridSize;
	// Calculate number of springs
	// There is a spring pointing right for each ball which is not on the right edge, 
	// and one pointing down for each ball not on the bottom edge
	numSprings = (gridSize-1) * gridSize * 2;
	// There is a spring pointing down & right for each ball not on bottom or right,
	// and one pointing down & left for each ball not on bottom or left
	numSprings += (gridSize-1) * (gridSize-1) * 2;
	// There is a spring pointing right (to the next but one ball)
	// for each ball which is not on or next to the right edge, 
	// and one pointing down for each ball not on or next to the bottom edge
	numSprings += (gridSize-2) * gridSize * 2;
	// Create space for balls & springs
	balls1 = new Ball[numBalls];
	balls2 = new Ball[numBalls];
	springs = new Spring[numSprings];
	if (!balls1 || !balls2 || !springs) {
		std::cerr << "Unable to allocate space for balls & springs" << std::endl;
		exit(1); 
	}
	// Reset cloth
	clothVertices = new float[36*(gridSize-1)*(gridSize-1)];
	reset();
}

Cloth::~Cloth() {
	if (balls1) {
		delete[] balls1;
		balls1 = NULL;
	}
	if (balls2) {
		delete[] balls2;
		balls2 = NULL;
	}
	if (springs) {
		delete[] springs;
		springs = NULL;
	}
	if (clothVertices) {
		delete[] clothVertices;
		clothVertices = NULL;
	}
    glDeleteVertexArrays(1, &clothVAO);
    glDeleteBuffers(1, &clothVBO);
}

void Cloth::reset() {
	// Initialise the balls in an evenly spaced grid in the x-z plane
	for (int i = 0; i < gridSize; ++i) {
		for (int j = 0; j < gridSize; ++j) {
			balls1[i*gridSize+j].position = glm::vec3(float(j)-float(gridSize-1)/2, float(i)-float(gridSize-1)/2, 0.0f);
			balls1[i*gridSize+j].velocity = glm::vec3(0.0f);
			balls1[i*gridSize+j].mass = mass;
			balls1[i*gridSize+j].fixed = false;
		}
		balls1[i*gridSize].fixed = (i%2==0);
	}

	// // Fix the top left & top right balls in place
	// balls1[0].fixed = true;
	// balls1[gridSize-1].fixed = true;
	// // Fix the bottom left & bottom right balls
	// balls1[gridSize*(gridSize-1)].fixed = true;
	// balls1[gridSize*gridSize-1].fixed = true;

	// Copy the balls into the other array
	for (int i = 0; i < numBalls; ++i)
		balls2[i] = balls1[i];
	// Set the currentBalls and nextBalls pointers
	currentBalls = balls1;
	nextBalls = balls2;
	// Initialise the springs
	Spring* currentSpring = &springs[0];
	// The first (gridSize-1)*gridSize springs go from one ball to the next,
	// excluding those on the right hand edge
	for (int i = 0; i < gridSize; ++i) {
		for (int j = 0; j < gridSize-1; ++j) {
			currentSpring->ball1 = i*gridSize + j;
			currentSpring->ball2 = i*gridSize + j + 1;

			currentSpring->springConstant = springConstant;
			currentSpring->naturalLength = naturalLength;
			
			++currentSpring;
		}
	}
	// The next (gridSize-1)*gridSize springs go from one ball to the one below,
	// excluding those on the bottom edge
	for (int i = 0; i < gridSize-1; ++i) {
		for (int j = 0; j < gridSize; ++j) {
			currentSpring->ball1 = i*gridSize + j;
			currentSpring->ball2 = (i+1)*gridSize + j;

			currentSpring->springConstant = springConstant;
			currentSpring->naturalLength = naturalLength;
			
			++currentSpring;
		}
	}
	// The next (gridSize-1)*(gridSize-1) go from a ball to the one below and right
	// excluding those on the bottom or right
	for (int i = 0; i < gridSize-1; ++i) {
		for (int j = 0; j < gridSize-1; ++j) {
			currentSpring->ball1 = i*gridSize + j;
			currentSpring->ball2 = (i+1)*gridSize + j + 1;

			currentSpring->springConstant = springConstant;
			currentSpring->naturalLength = naturalLength * sqrt(2.0f);
			
			++currentSpring;
		}
	}
	// The next (gridSize-1)*(gridSize-1) go from a ball to the one below and left
	// excluding those on the bottom or right
	for (int i = 0; i < gridSize-1; ++i) {
		for (int j = 1; j < gridSize; ++j) {
			currentSpring->ball1 = i*gridSize + j;
			currentSpring->ball2 = (i+1)*gridSize + j - 1;

			currentSpring->springConstant = springConstant;
			currentSpring->naturalLength = naturalLength * sqrt(2.0f);
			
			++currentSpring;
		}
	}
	// The first (gridSize-2)*gridSize springs go from one ball to the next but one,
	// excluding those on or next to the right hand edge
	for (int i = 0; i < gridSize; ++i) {
		for (int j = 0; j < gridSize-2; ++j) {
			currentSpring->ball1 = i*gridSize + j;
			currentSpring->ball2 = i*gridSize + j + 2;

			currentSpring->springConstant = springConstant;
			currentSpring->naturalLength = naturalLength * 2;
			
			++currentSpring;
		}
	}
	// The next (gridSize-2)*gridSize springs go from one ball to the next but one below,
	// excluding those on or next to the bottom edge
	for (int i=0; i < gridSize-2; ++i) {
		for (int j=0; j < gridSize; ++j) {
			currentSpring->ball1 = i*gridSize + j;
			currentSpring->ball2 = (i+2)*gridSize + j;

			currentSpring->springConstant = springConstant;
			currentSpring->naturalLength = naturalLength * 2;
			
			++currentSpring;
		}
	}
}

void Cloth::update(float deltaTime) {
	// Update the physics in intervals of 10ms to prevent problems
	// with different frame rates causing different damping
	static double timeSinceLastUpdate = 0.0f;
	static double timeSinceLastChangeOfForce = 0.0f;
	timeSinceLastUpdate += deltaTime;
	timeSinceLastChangeOfForce += deltaTime;

	bool updateMade = false;	// did we update the positions etc this time?

	if (timeSinceLastChangeOfForce > 0.25f) {
		timeSinceLastChangeOfForce -= 0.25f;
		FORCE.x += 0.55f * ((float)rand()/(RAND_MAX+1.0f)*2 - 1);
		FORCE.z += 0.55f * ((float)rand()/(RAND_MAX+1.0f)*2 - 1);
	}
	FORCE.x = glm::clamp(FORCE.x, -0.98f, 0.98f);
	FORCE.z = glm::clamp(FORCE.z, -0.98f, 0.98f);
	
	while (timeSinceLastUpdate > 0.01f) {
		timeSinceLastUpdate -= 0.01f;
		float timePassedInSeconds = 0.01f;
		updateMade = true;

		// Calculate the tensions in the springs
		for (int i = 0; i < numSprings; ++i) {
			float springLength = glm::length(currentBalls[springs[i].ball1].position-currentBalls[springs[i].ball2].position);

			float extension = springLength - springs[i].naturalLength;
	
			springs[i].tension = springs[i].springConstant * extension / springs[i].naturalLength;
		}

		// Calculate the nextBalls from the currentBalls
		for (int i = 0; i < numBalls; ++i) {
			// Transfer properties which do not change
			nextBalls[i].fixed = currentBalls[i].fixed;
			nextBalls[i].mass = currentBalls[i].mass;

			// If the ball is fixed, transfer the position and zero the velocity, otherwise calculate
			// the new values
			if(currentBalls[i].fixed) {
				nextBalls[i].position = currentBalls[i].position;
				nextBalls[i].velocity = glm::vec3(0.0f);
			} else {
				// Calculate the force on this ball
				glm::vec3 force = FORCE;

				// Loop through springs
				for (int j = 0; j < numSprings; ++j) {
					// If this ball is "ball1" for this spring, add the tension to the force
					if (springs[j].ball1 == i) {
						glm::vec3 tensionDirection = currentBalls[springs[j].ball2].position - currentBalls[i].position;
						tensionDirection = glm::normalize(tensionDirection);
	
						force += springs[j].tension * tensionDirection;
					}
	
					// Similarly if the ball is "ball2"
					if (springs[j].ball2 ==i) {
						glm::vec3 tensionDirection = currentBalls[springs[j].ball1].position - currentBalls[i].position;
						tensionDirection = glm::normalize(tensionDirection);
	
						force += springs[j].tension * tensionDirection;
					}
				}

				// Calculate the acceleration
				glm::vec3 acceleration = force / currentBalls[i].mass;

				float ratio = (float)gridSize / (abs(gridSize-2*(i%gridSize)+1)+abs(gridSize-2*(i/gridSize)+1)+1) / 1.8f;
				acceleration.z *= ratio;

				// Update velocity
				nextBalls[i].velocity = currentBalls[i].velocity + acceleration * timePassedInSeconds;

				// Damp the velocity
				nextBalls[i].velocity *= dampFactor;
			
				// Calculate new position
				nextBalls[i].position = currentBalls[i].position + (nextBalls[i].velocity+currentBalls[i].velocity)*(timePassedInSeconds/2.0f);

				// Check against floor
				if (nextBalls[i].position.y < -21.25f)
					nextBalls[i].position.y = -21.25f;

				float v = abs(nextBalls[i].velocity.z);
				nextBalls[i].velocity.x = (glm::clamp(nextBalls[i].velocity.x, 1.225f, 4.9f)+v) / 2.1f;
			}
		}

		// Swap the currentBalls and newBalls pointers
		Ball* temp = currentBalls;
		currentBalls = nextBalls;
		nextBalls = currentBalls;
	}

	// Calculate the normals if we have updated the positions
	if (updateMade) {
		//Zero the normals on each ball
		for (int i = 0; i < numBalls; ++i)
			currentBalls[i].normal = glm::vec3(0.0f);

		// Calculate the normals on the current balls
		for (int i = 0; i < gridSize-1; ++i) {
			for(int j = 0; j < gridSize-1; ++j) {
				glm::vec3& p0 = currentBalls[i*gridSize+j].position;
				glm::vec3& p1 = currentBalls[i*gridSize+j+1].position;
				glm::vec3& p2 = currentBalls[(i+1)*gridSize+j].position;
				glm::vec3& p3 = currentBalls[(i+1)*gridSize+j+1].position;

				glm::vec3& n0 = currentBalls[i*gridSize+j].normal;
				glm::vec3& n1 = currentBalls[i*gridSize+j+1].normal;
				glm::vec3& n2 = currentBalls[(i+1)*gridSize+j].normal;
				glm::vec3& n3 = currentBalls[(i+1)*gridSize+j+1].normal;

				// Calculate the normals for the 2 triangles and add on
				glm::vec3 normal = glm::cross(p1-p0, p2-p0);
	
				n0 += normal;
				n1 += normal;
				n2 += normal;

				normal = glm::cross(p1-p2, p3-p2);
	
				n1 += normal;
				n2 += normal;
				n3 += normal;
			}
		}

		// Normalize normals
		for(int i = 0; i < numBalls; ++i)
			currentBalls[i].normal = glm::normalize(currentBalls[i].normal);
	}
}

void Cloth::draw() {
	// Initialize
	if (clothVAO == 0) {
	    glGenVertexArrays(1, &clothVAO);
	    glGenBuffers(1, &clothVBO);
	}
	//Draw cloth as triangles
	int index = 0;
	for (int i = 0; i < gridSize-1; ++i) {
		for (int j = 0; j < gridSize-1; ++j) {
			clothVertices[index++] = currentBalls[i*gridSize+j].position.x;
			clothVertices[index++] = currentBalls[i*gridSize+j].position.y;
			clothVertices[index++] = currentBalls[i*gridSize+j].position.z;
			clothVertices[index++] = currentBalls[i*gridSize+j].normal.x;
			clothVertices[index++] = currentBalls[i*gridSize+j].normal.y;
			clothVertices[index++] = currentBalls[i*gridSize+j].normal.z;

			clothVertices[index++] = currentBalls[i*gridSize+j+1].position.x;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].position.y;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].position.z;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].normal.x;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].normal.y;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].normal.z;

			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].position.x;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].position.y;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].position.z;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].normal.x;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].normal.y;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].normal.z;

			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].position.x;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].position.y;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].position.z;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].normal.x;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].normal.y;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j].normal.z;

			clothVertices[index++] = currentBalls[i*gridSize+j+1].position.x;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].position.y;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].position.z;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].normal.x;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].normal.y;
			clothVertices[index++] = currentBalls[i*gridSize+j+1].normal.z;

			clothVertices[index++] = currentBalls[(i+1)*gridSize+j+1].position.x;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j+1].position.y;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j+1].position.z;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j+1].normal.x;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j+1].normal.y;
			clothVertices[index++] = currentBalls[(i+1)*gridSize+j+1].normal.z;
		}
	}
	glBindVertexArray(clothVAO);
	glBindBuffer(GL_ARRAY_BUFFER, clothVBO);
	glBufferData(GL_ARRAY_BUFFER, 36*(gridSize-1)*(gridSize-1)*sizeof(float), clothVertices, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glDrawArrays(GL_TRIANGLES, 0, 6*(gridSize-1)*(gridSize-1));
    glBindVertexArray(0);
}

void Cloth::unfix(TIP tip) {
	switch (tip) {
		case 0: {
			currentBalls[0].fixed = false;
			break;
		}
		case 1: {
			currentBalls[gridSize-1].fixed = false;
			break;
		}
		case 2: {
			currentBalls[gridSize*(gridSize-1)].fixed = false;
			break;
		}
		case 3: {
			currentBalls[gridSize*gridSize-1].fixed = false;
			break;			
		}
		default:
			break;
	}
}

#endif