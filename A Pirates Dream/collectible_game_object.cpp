#include "collectible_game_object.h"

namespace game {

/*
	CollectibleGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the Collectible

	copied mostly from player game object file
*/

CollectibleGameObject::CollectibleGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture)
	: GameObject(position, geom, shader, texture, 2) {}

// Update function for moving the Collectible object around
void CollectibleGameObject::Update(double delta_time) {

	// Special Collectible updates go here

	// Call the parent's update method to move the object in standard way, if desired
	GameObject::Update(delta_time);
}

} // namespace game
