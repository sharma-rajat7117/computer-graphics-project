//#pragma once
//
//#include <glm/glm.hpp>
//#include "glm/gtc/matrix_transform.hpp"
//
//#define DAMPING 0.01 // how much to damp the cloth simulation each frame
//#define TIME_STEPSIZE2 0.5*0.5 // how large time step each particle takes each frame
//#define CONSTRAINT_ITERATIONS 15 // how many iterations of constraint satisfaction each frame (more is rigid, less is soft)
//
///* The particle class represents a particle of mass that can move around in 3D space*/
//class Particle
//{
//private:
//	bool movable; // can the particle move or not ? used to pin parts of the cloth
//
//	float mass; // the mass of the particle (is always 1 in this example)
//	glm::vec3 pos; // the current position of the particle in 3D space
//	glm::vec3 old_pos; // the position of the particle in the previous time step, used as part of the verlet numerical integration scheme
//	glm::vec3 acceleration; // a vector representing the current acceleration of the particle
//	glm::vec3 accumulated_normal; // an accumulated normal (i.e. non normalized), used for OpenGL soft shading
//
//public:
//	Particle(glm::vec3 pos) : pos(pos), old_pos(pos), acceleration(glm::vec3(0, 0, 0)), mass(1), movable(true), accumulated_normal(glm::vec3(0, 0, 0)) {}
//	Particle() {}
//
//	void addForce(glm::vec3 f)
//	{
//		acceleration += f / mass;
//	}
//
//	/* This is one of the important methods, where the time is progressed a single step size (TIME_STEPSIZE)
//	The method is called by Cloth.time_step()
//	Given the equation "force = mass * acceleration" the next position is found through verlet integration*/
//	void timeStep()
//	{
//		if (movable)
//		{
//			glm::vec3 temp = pos;
//			pos = pos + (pos - old_pos)*(1.0 - DAMPING) + acceleration * TIME_STEPSIZE2;
//			old_pos = temp;
//			acceleration = glm::vec3(0, 0, 0); // acceleration is reset since it HAS been translated into a change in position (and implicitely into velocity)	
//		}
//	}
//
//	glm::vec3& getPos() { return pos; }
//
//	void resetAcceleration() { acceleration = glm::vec3(0, 0, 0); }
//
//	void offsetPos(const glm::vec3 v) { if (movable) pos += v; }
//
//	void makeUnmovable() { movable = false; }
//
//	void addToNormal(glm::vec3 normal)
//	{
//		accumulated_normal += normal.normalized();
//	}
//
//	glm::vec3& getNormal() { return accumulated_normal; } // notice, the normal is not unit length
//
//	void resetNormal() { accumulated_normal = glm::vec3(0, 0, 0); }
//
//};
//
//class Cloth
//{
//public:
//	Cloth();
//	virtual ~Cloth();
//};
//
//class Constraint
//{
//private:
//	float rest_distance; // the length between particle p1 and p2 in rest configuration
//
//public:
//	Particle * p1, *p2; // the two particles that are connected through this constraint
//
//	Constraint(Particle *p1, Particle *p2) : p1(p1), p2(p2)
//	{
//		glm::vec3 vec = p1->getPos() - p2->getPos();
//		rest_distance = vec.length();
//	}
//
//	/* This is one of the important methods, where a single constraint between two particles p1 and p2 is solved
//	the method is called by Cloth.time_step() many times per frame*/
//	void satisfyConstraint()
//	{
//		glm::vec3 p1_to_p2 = p2->getPos() - p1->getPos(); // vector from p1 to p2
//		float current_distance = p1_to_p2.length(); // current distance between p1 and p2
//		glm::vec3 correctionVector = p1_to_p2 * (1 - rest_distance / current_distance); // The offset vector that could moves p1 into a distance of rest_distance to p2
//		glm::vec3 correctionVectorHalf = correctionVector * 0.5; // Lets make it half that length, so that we can move BOTH p1 and p2.
//		p1->offsetPos(correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
//		p2->offsetPos(-correctionVectorHalf); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
//	}
//
//};
