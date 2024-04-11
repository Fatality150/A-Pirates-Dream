#include "projectile_game_object.h"

namespace game {

/*
	ProjectileGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the player
*/

ProjectileGameObject::ProjectileGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture)
	: GameObject(position, geom, shader, texture) 
	{ 
		start_pos_ = position;
	}

// Update function for moving the player object around
void ProjectileGameObject::Update(double delta_time) {

	// Special player updates go here
	
	//std::cout << time_ << std::endl;

	position_.x = start_pos_.x + (velocity_.x * 100 * time_);
	position_.y = start_pos_.y + (velocity_.y * 100 * time_);
	

	

	// Call the parent's update method to move the object in standard way, if desired
	GameObject::Update(delta_time);
}

void ProjectileGameObject::SetVelocity(glm::vec3 &velocity)
{
	velocity_ = velocity;
}

}