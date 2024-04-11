#include "collectible_game_object.h"

namespace game {

/*
	CollectibleGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the Collectible

	copied mostly from player game object file
*/

CollectibleGameObject::CollectibleGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture, int type)
	: GameObject(position, geom, shader, texture) 
	{
		type_ = type;
		start_pos_ = position_;
	}

// Update function for moving the Collectible object around
void CollectibleGameObject::Update(double delta_time) {

	// Special Collectible updates go here

	position_.y = start_pos_.y + 0.1 * sin(3  * time_);

	// Call the parent's update method to move the object in standard way, if desired
	GameObject::Update(delta_time);
}

} // namespace game
