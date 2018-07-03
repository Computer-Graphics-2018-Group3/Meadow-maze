#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H
#include <vector>

#include "sprite_renderer.h"


// Represents a single particle and its state
struct Particle {
	glm::vec3 Position, Velocity;
	glm::vec4 Color;
	GLfloat Life;

	Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};


// ParticleGenerator acts as a container for rendering a large number of 
// particles by repeatedly spawning and updating particles and killing 
// them after a given amount of time.
class ParticleGenerator
{
public:
	// Constructor
	ParticleGenerator(Shader shader, unsigned int texture, GLuint amount);
	// Update all particles
	//void Update(GLfloat dt, GameObject &object, GLuint newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
	void Update(Shader shader, GLfloat dt, glm::vec3 position, GLuint newParticles, glm::vec3 velocity, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f));
	// Render all particles
	void Draw();
private:
	// State
	std::vector<Particle> particles;
	GLuint amount;
	// Render state
	Shader shader;
	unsigned int texture;
	GLuint VAO;
	// Initializes buffer and vertex attributes
	void init();
	// Returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
	GLuint firstUnusedParticle();
	// Respawns particle
	//void respawnParticle(Particle &particle, GameObject &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
	void respawnParticle(Particle &particle, glm::vec3 position, glm::vec3 velocity, glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f));
};

ParticleGenerator::ParticleGenerator(Shader shader, unsigned int texture, GLuint amount)
	: shader(shader), texture(texture), amount(amount)
{
	this->init();
}

void ParticleGenerator::Update(Shader shader, GLfloat dt, glm::vec3 position, GLuint newParticles, glm::vec3 velocity, glm::vec3 offset)
{
	this->shader = shader;
	// Add new particles 
	for (GLuint i = 0; i < newParticles; ++i)
	{
		int unusedParticle = this->firstUnusedParticle();
		this->respawnParticle(this->particles[unusedParticle], position, velocity, offset);
	}
	// Update all particles
	for (GLuint i = 0; i < this->amount; ++i)
	{
		Particle &p = this->particles[i];
		p.Life -= dt; // reduce life
		if (p.Life > 0.0f)
		{	// particle is alive, thus update
			p.Position -= p.Velocity * dt;
			//p.Color.a -= dt * 2.5;
		}
	}
}

// Render all particles
void ParticleGenerator::Draw()
{
	// Use additive blending to give it a 'glow' effect
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	this->shader.use();
	for (Particle particle : this->particles)
	{
		if (particle.Life > 0.0f)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			this->shader.setVec3("offset", particle.Position);
			this->shader.setVec4("color", particle.Color);
			glBindVertexArray(this->VAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
		}
	}
	// Don't forget to reset to default blending mode
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init()
{
	// Set up mesh and attribute properties
	GLuint VBO;
	GLfloat particle_quad[] = {
		// Pos            // Tex
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.5f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f, 0.0f,

		0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.5f, 1.0f, 0.0f,
	};
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(this->VAO);
	// Fill mesh buffer
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
	// Set mesh attributes
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(float)));
	glBindVertexArray(0);

	// Create this->amount default particle instances
	for (GLuint i = 0; i < this->amount; ++i)
		this->particles.push_back(Particle());
}

// Stores the index of the last particle used (for quick access to next dead particle)
GLuint lastUsedParticle = 0;
GLuint ParticleGenerator::firstUnusedParticle()
{
	// First search from last used particle, this will usually return almost instantly
	for (GLuint i = lastUsedParticle; i < this->amount; ++i) {
		if (this->particles[i].Life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// Otherwise, do a linear search
	for (GLuint i = 0; i < lastUsedParticle; ++i) {
		if (this->particles[i].Life <= 0.0f) {
			lastUsedParticle = i;
			return i;
		}
	}
	// All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
	lastUsedParticle = 0;
	return 0;
}
/*
void ParticleGenerator::respawnParticle(Particle &particle, GameObject &object, glm::vec2 offset)
{
GLfloat random = ((rand() % 100) - 50) / 10.0f;
GLfloat rColor = 0.5 + ((rand() % 100) / 100.0f);
particle.Position = object.Position + random + offset;
particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
particle.Life = 1.0f;
particle.Velocity = object.Velocity * 0.1f;
}
*/
void ParticleGenerator::respawnParticle(Particle &particle, glm::vec3 position, glm::vec3 velocity, glm::vec3 offset)
{
	glm::vec3 random(((rand() % 350)-150) / 10.0f, ((rand() % 100) - 50) / 10.0f,((rand() % 300) - 220) / 10.0f);
	//GLfloat rColor = 0.5 + ((rand() % 100) / 100.0f);
	particle.Position = position + random + offset;
	//particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
	particle.Life = 10.0f;
	particle.Velocity = velocity * 0.05f;
}

#endif