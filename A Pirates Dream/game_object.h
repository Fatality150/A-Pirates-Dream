#ifndef GAME_OBJECT_H_
#define GAME_OBJECT_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#define GLEW_STATIC
#include <GL/glew.h>
#include <cmath>
#include <iostream>


#include "shader.h"
#include "geometry.h"
#include "timer.h"

namespace game {

    /*
        GameObject is responsible for handling the rendering and updating of one object in the game world
        The update and render methods are virtual, so you can inherit them from GameObject and override the update or render functionality (see PlayerGameObject for reference)
    */
    class GameObject {

        public:
            // Constructor
            GameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture);

            // Destructor
            ~GameObject();

            // Update the GameObject's state. Can be overriden in children
            virtual void Update(double delta_time);

            // Renders the GameObject 
            virtual void Render(glm::mat4 view_matrix, double current_time);

            // Getters
            inline glm::vec3 GetPosition(void) const { return position_; }
            inline float GetScale(void) const { return scale_; }
            inline float GetRotation(void) const { return angle_; }
            int GetTimer(int i = 1) const { return timer_->Finished(i); }
            inline double GetTimerTime(void) const { return timer_->GetTime(); }
            glm::vec3 GetVelocity(void) const { return velocity_; }
            inline double GetTime(void) const { return time_; }
            //inline double 
            virtual inline glm::vec3 GetStart(void) const { return glm::vec3(0.0f,0.0f,0.0f); }
            // 
            virtual inline void GetParent(GameObject **parent) {}

            // Get bearing direction (direction in which the game object
            // is facing)
            glm::vec3 GetBearing(void) const;

            // Get vector pointing to the right side of the game object
            glm::vec3 GetRight(void) const;

            // Setters
            inline void SetPosition(const glm::vec3& position) { position_ = position; }
            inline void SetScale(float scale) { scale_ = scale; }
            virtual void SetRotation(float angle);
            void SetTimer(float end_time);
            void SetTexture(GLuint texture) { texture_ = texture;}
            virtual void SetVelocity(glm::vec3 &velocity);


        protected:
            // Object's Transform Variables
            glm::vec3 position_;
            glm::vec3 velocity_;
            float scale_;
            float angle_;

            // a total for the amount of time the object has been alive, helps us keep the enemy movement unique for now 
            double time_;

            // a timer for this objects explosion
            Timer* timer_;

            // Geometry
            Geometry *geometry_;
 
            // Shader
            Shader *shader_;

            // Object's texture reference
            GLuint texture_;

    }; // class GameObject

} // namespace game

#endif // GAME_OBJECT_H_
