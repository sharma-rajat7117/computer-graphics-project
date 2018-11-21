#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "glm/gtc/matrix_transform.hpp"


const float dumping = 0.01f;
const float time_stepsize2 = 0.5f*0.5f;
const int constraint_iterations = 15;

namespace CGCommon
{

	/* The particle class represents a particle of mass that can move around in 3D space*/
	class ClothParticle
	{
	private:
		bool movable; // can the particle move or not ? used to pin parts of the cloth

		float mass; // the mass of the particle (is always 1 in this example)
		glm::vec3 pos; // the current position of the particle in 3D space
		glm::vec3 old_pos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
		glm::vec3 acceleration; // a vector representing the current acceleration of the particle
		glm::vec3 accumulated_normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading

	public:
		ClothParticle(glm::vec3 pos) : pos(pos), old_pos(pos), acceleration(glm::vec3(0, 0, 0)), mass(1), movable(true), accumulated_normal(glm::vec3(0, 0, 0)) {}
		ClothParticle() {}

		void addForce(glm::vec3 f);

		void timeStep();
		
		glm::vec3& getPos();

		void resetAcceleration();

		void offsetPos(const glm::vec3 v);

		void makeUnmovable();

		void addToNormal(glm::vec3 normal);

		glm::vec3& getNormal();

		void resetNormal();

	};

	class Constraint
	{
	private:
		float rest_distance; // the length between particle p1 and p2 in rest configuration

	public:
		ClothParticle *p1, *p2; // the two particles that are connected through this constraint

		Constraint(ClothParticle *p1, ClothParticle *p2);
		
		/* This is one of the important methods, where a single constraint between two particles p1 and p2 is solved
		the method is called by Cloth.time_step() many times per frame*/
		void satisfyConstraint();
	};

	class Cloth
	{
	private:

		glm::vec3 initialPosition = glm::vec3(0, 0, 0);
		int num_particles_width; // number of particles in "width" direction
		int num_particles_height; // number of particles in "height" direction
		std::vector<ClothParticle> particles; // all particles that are part of this cloth
		std::vector<Constraint> constraints; // alle constraints between particles as part of this cloth
		
		ClothParticle* getParticle(int x, int y);
		void makeConstraint(ClothParticle *p1, ClothParticle *p2);

	public:
		Cloth(float width, float height, int num_particles_width, int num_particles_height);
		virtual ~Cloth();
	};

}
