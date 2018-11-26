#include "Cloth.h"

namespace CGCommon
{
	void ClothParticle::addForce(glm::vec3 f)
	{
		acceleration += f / mass;
	}
	
	/* This is one of the important methods, where the time is progressed a single step size (TIME_STEPSIZE)
	The method is called by Cloth.time_step()
	Given the equation "force = mass * acceleration" the next position is found through verlet integration*/
	void ClothParticle::timeStep(float deltaTime)
	{
		if (this->movable)
		{
			glm::vec3 temp = this->pos;
			this->pos = this->pos + (this->pos - this->old_pos)*(1.0F - dumping) + this->acceleration * deltaTime;
			this->old_pos = temp;
			this->acceleration = glm::vec3(0, 0, 0); // acceleration is reset since it HAS been translated into a change in position (and implicitely into velocity)	
		}
	}

	glm::vec3& ClothParticle::getPos() { return pos; };

	void ClothParticle::resetAcceleration() { acceleration = glm::vec3(0, 0, 0); }

	void ClothParticle::offsetPos(const glm::vec3 v) { if (movable) pos += v; }

	void ClothParticle::makeUnmovable() { movable = false; }

	void ClothParticle::addToNormal(glm::vec3 normal)
	{
		accumulated_normal += glm::normalize(normal);
	}

	glm::vec3& ClothParticle::getNormal() { return accumulated_normal; } 

	void ClothParticle::resetNormal() { accumulated_normal = glm::vec3(0, 0, 0); }

	Constraint::Constraint(ClothParticle *p1, ClothParticle *p2) 
	{
		this->p1 = p1;
		this->p2 = p2;
		glm::vec3 vec = p1->getPos() - p2->getPos();
		rest_distance = sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
	}
	
	/* This is one of the important methods, where a single constraint between two particles p1 and p2 is solved
	the method is called by Cloth.time_step() many times per frame*/
	void Constraint::satisfyConstraint()
	{
		glm::vec3 p1_to_p2 = p2->getPos() - p1->getPos(); // vector from p1 to p2
		float current_distance = glm::distance(p1->getPos(), p2->getPos());//   p1_to_p2.length(); // current distance between p1 and p2
		glm::vec3 correctionVector = p1_to_p2 * (1 - rest_distance / current_distance); // The offset vector that could moves p1 into a distance of rest_distance to p2
		glm::vec3 correctionVectorHalf = glm::vec3(correctionVector.x * 0.5, correctionVector.y * 0.5, correctionVector.z * 0.5); // half-length
		p1->offsetPos(correctionVectorHalf); // correctionVectorHalf is pointing from p1 to p2, so the length should move p1 half the length needed to satisfy the constraint.
		p2->offsetPos(-correctionVectorHalf); // we must move p2 the negative direction of correctionVectorHalf since it points from p2 to p1, and not p1 to p2.	
	}

	Cloth::Cloth(float width, float height, int num_particles_width, int num_particles_height)
	{
		this->num_particles_width = num_particles_width;
		this->num_particles_height = num_particles_height;

		particles.resize(num_particles_width * num_particles_height); 

		float step_x = width / (float)num_particles_width;
		float step_y = height / (float)num_particles_height;
																			
		for (int x = 0; x < num_particles_width; x++)   // creating particles in a grid of particles from (0,0,0) to (width,-height,0)
		{
			for (int y = 0; y < num_particles_height; y++)
			{
				glm::vec3 pos = glm::vec3(step_x * x, -step_y * y, 0);
				particles[y*num_particles_width + x] = ClothParticle(pos); // insert particle in column x at y'th row  - because using just one array
			}
		}

		// Connecting immediate neighbor particles with constraints (distance 1 and sqrt(2) in the grid)
		for (int x = 0; x < num_particles_width; x++)
		{
			for (int y = 0; y < num_particles_height; y++)
			{
				if (x < num_particles_width - 1) makeConstraint(GetParticle(x, y), GetParticle(x + 1, y));
				if (y < num_particles_height - 1) makeConstraint(GetParticle(x, y), GetParticle(x, y + 1));
				if (x < num_particles_width - 1 && y < num_particles_height - 1) makeConstraint(GetParticle(x, y), GetParticle(x + 1, y + 1));
				if (x < num_particles_width - 1 && y < num_particles_height - 1) makeConstraint(GetParticle(x + 1, y), GetParticle(x, y + 1));
			}
		}


		// Connecting secondary neighbors with constraints (distance 2 and sqrt(4) in the grid)
		for (int x = 0; x < num_particles_width; x++)
		{
			for (int y = 0; y < num_particles_height; y++)
			{
				if (x < num_particles_width - 2) makeConstraint(GetParticle(x, y), GetParticle(x + 2, y));
				if (y < num_particles_height - 2) makeConstraint(GetParticle(x, y), GetParticle(x, y + 2));
				if (x < num_particles_width - 2 && y < num_particles_height - 2) makeConstraint(GetParticle(x, y), GetParticle(x + 2, y + 2));
				if (x < num_particles_width - 2 && y < num_particles_height - 2) makeConstraint(GetParticle(x + 2, y), GetParticle(x, y + 2));
			}
		}

		// making the left size unmovable
		for (int i = 0; i < num_particles_height; i= i+3)
		{	
			GetParticle(0, i)->makeUnmovable();
		}
	}
	
	Cloth::~Cloth()
	{
	}

	ClothParticle* Cloth::GetParticle(int x, int y) 
	{ 
		return &particles[y*num_particles_width + x]; 
	}
	
	void Cloth::makeConstraint(ClothParticle *p1, ClothParticle *p2) 
	{ 
		constraints.push_back(Constraint(p1, p2)); 
	}

	void Cloth::Draw()
	{
		for (int x = 0; x < num_particles_width - 1; x++)
		{
			for (int y = 0; y < num_particles_height - 1; y++)
			{
				// Write particles into buffer
				//particles.assign
			}
		}
	}

	int  Cloth::NumParticlesWidth() { return this->num_particles_width; }
	int  Cloth::NumParticlesHeight() { return this->num_particles_height; }

	/* used to add gravity (or any other arbitrary vector) to all particles*/
	void Cloth::addForce(const glm::vec3 direction)
	{
		std::vector<ClothParticle>::iterator particle;
		for (particle = particles.begin(); particle != particles.end(); particle++)
		{
			(*particle).addForce(direction); // add the forces to each particle
		}

	}

	glm::vec3 Cloth::calcTriangleNormal(ClothParticle *p1, ClothParticle *p2, ClothParticle *p3)
	{
		glm::vec3 pos1 = p1->getPos();
		glm::vec3 pos2 = p2->getPos();
		glm::vec3 pos3 = p3->getPos();

		glm::vec3 v1 = pos2 - pos1;
		glm::vec3 v2 = pos3 - pos1;

		return glm::cross( v1, v2);
	}

	/* A private method used by windForce() to calcualte the wind force for a single triangle
	defined by p1,p2,p3*/
	void Cloth::addWindForcesForTriangle(ClothParticle *p1, ClothParticle *p2, ClothParticle *p3, const glm::vec3 direction)
	{
		glm::vec3 normal = calcTriangleNormal(p1, p2, p3);
		glm::vec3 d = glm::normalize(normal);
		glm::vec3 force = normal*(glm::dot(d, direction)); 
		p1->addForce(force);
		p2->addForce(force);
		p3->addForce(force);
	}

	/* used to add wind forces to all particles, is added for each triangle since the final force is proportional to the triangle area as seen from the wind direction*/
	void Cloth::windForce(const glm::vec3 direction)
	{
		for (int x = 0; x<num_particles_width - 1; x++)
		{
			for (int y = 0; y<num_particles_height - 1; y++)
			{
				addWindForcesForTriangle(GetParticle(x + 1, y), GetParticle(x, y), GetParticle(x, y + 1), direction);
				addWindForcesForTriangle(GetParticle(x + 1, y + 1), GetParticle(x + 1, y), GetParticle(x, y + 1), direction);
			}
		}
	}

	void Cloth::timeStep(float deltaTime)
	{
		std::vector<Constraint>::iterator constraint;
		for (int i = 0; i<15; i++) // iterate over all constraints several times
		{
			for (constraint = constraints.begin(); constraint != constraints.end(); constraint++)
			{
				(*constraint).satisfyConstraint(); // satisfy constraint.
			}
		}

		std::vector<ClothParticle>::iterator particle;
		for (particle = particles.begin(); particle != particles.end(); particle++)
		{
			(*particle).timeStep(deltaTime); // calculate the position of each particle at the next time step.
		}
	}
}