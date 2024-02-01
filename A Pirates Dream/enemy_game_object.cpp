#include "enemy_game_object.h"

namespace game {

/*
	EnemyGameObject inherits from GameObject
	It overrides GameObject's update method, so that you can check for input to change the velocity of the Enemy

	copied mostly from player game object file
*/

EnemyGameObject::EnemyGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture)
	: GameObject(position, geom, shader, texture, 1) 
	{
		// base state should always be patrolling
		state_ = PATROLLING;
		
		//centre_point_ = glm::vec3(position_.x, 0.0f, 0.0f);

		// were gonna set the center point towards the center from wherever the enemy spawns

		if (position_.x > 0 && position_.y > 0)
		{
			centre_point_ = glm::vec3(position_.x - .5f, position_.y - .5f, 0.0f);
		}
		else if (position_.x <= 0 && position_.y > 0)
		{
			centre_point_ = glm::vec3(position_.x + .5f, position_.y - .5f, 0.0f);
		}
		else if (position_.x > 0 && position_.y <= 0)
		{
			centre_point_ = glm::vec3(position_.x - .5f, position_.y + .5f, 0.0f);
		}
		else
		{
			centre_point_ = glm::vec3(position_.x + .5f, position_.y + .5f, 0.0f);
		}
		
	}

// Update function for moving the Enemy object around
void EnemyGameObject::Update(double delta_time) {

	// first well decide what we wanna do by checking the state

	if (state_ == PATROLLING)
	{
		/*
		problems:
			dist not working properly
		*/

		// every approx 2 miliseconds were gonna update
		if (static_cast<int>( time_ * 30 ) < static_cast<int>( (time_ + delta_time) * 30 ) ) 
		{
			//std::cout << "time = " << static_cast<int>(time_ + delta_time) << std::endl;

			// were gonna get the radians as a partial roation over a certain amount of time
			float radians = static_cast<float>( static_cast<int>( (time_ + delta_time) * 30 ) % 360 ) * glm::pi<float>() / 180.0f;
			
			//float dist = sqrt( pow( position_.x + centre_point_.x, 2 ) + pow( position_.y + centre_point_.y, 2 ) );

			// were gonna use those radians to get the new x and y values
			float new_x = ( 1.0f * static_cast<float> ( cos( radians ) ) ) + centre_point_.x;
			float new_y = ( 1.0f * static_cast<float> ( sin( radians ) ) ) + centre_point_.y;
			
			// were gonna make the angle the direction were moving
			angle_ = static_cast<float>(atan2(new_y - position_.y, new_x - position_.x));
			
			// finally were gonna set the positions to the entity
			position_.x = new_x;
			position_.y = new_y;
		}
	}
	else if (state_ == INTERCEPTING)
	{
		// how do i reliably get the target position here? can i make a set target function only for the child function and then call it with the game objects list in the game.cpp file?
		
	}

	

	// Call the parent's update method to move the object in standard way, if desired
	GameObject::Update(delta_time);
}

} // namespace game
