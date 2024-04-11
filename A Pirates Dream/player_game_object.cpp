#include "player_game_object.h"

namespace game {

/*
	PlayerGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the player
*/

PlayerGameObject::PlayerGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture)
	: GameObject(position, geom, shader, texture) {}

// Update function for moving the player object around
void PlayerGameObject::Update(double delta_time) {

	// Special player updates go here
	
	position_.x += velocity_.x;
	position_.y += velocity_.y;

	// Call the parent's update method to move the object in standard way, if desired
	GameObject::Update(delta_time);
}

void PlayerGameObject::SetVelocity(glm::vec3 &velocity)
{
	//weird jerky thing when reversing direction

	float x = velocity.x + velocity_.x;
	float y = velocity.y + velocity_.y;

	x /= 0.01f;
	y /= 0.01f;

	float c = sqrt( ( x * x ) + ( y * y ) );

	x = x / c;
	y = y / c;

	x *= 0.01f;
	y *= 0.01f; 

	velocity_.x = x;
	velocity_.y = y;
}

} // namespace game
