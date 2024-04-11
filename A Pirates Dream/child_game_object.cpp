#include "child_game_object.h"

namespace game {

ChildGameObject::ChildGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture, GameObject *parent, int mode)
	: GameObject(position, geom, shader, texture) 
    {
        parent_ = parent;
        angle_offset_ = 0;
        mode_ = mode;
    }

void ChildGameObject::SetRotation(float angle)
{
    angle_offset_ += angle;

    if (angle_offset_ > 2 * glm::pi<float>()) angle_offset_ -= 2 * glm::pi<float>();
    if (angle_offset_ < 0) angle_offset_ += (2 * glm::pi<float>());

    //std::cout << angle_offset_ << std::endl;

}

// Update function for moving the player object around
void ChildGameObject::Update(double delta_time) {

	position_.x = parent_->GetPosition().x + 1.0f * scale_;
    position_.y = parent_->GetPosition().y + 1.0f * scale_;

    if (static_cast<int>( time_ * 30 ) < static_cast<int>( (time_ + delta_time) * 30 ) ) 
    {
        angle_ = (static_cast<float>( static_cast<int>( (time_ + delta_time) * 30 ) % 360 )  * glm::pi<float>() / 180.0f) + angle_offset_;
        //std::cout << angle_ << std::endl;
    }

	// Call the parent's update method to move the object in standard way, if desired
	GameObject::Update(delta_time);
}

} // namespace game
