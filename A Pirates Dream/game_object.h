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

#define PLAYER 0
#define ENEMY 1
#define COLLECTIBLE 2
#define MISC 3


namespace game {

    /*
        GameObject is responsible for handling the rendering and updating of one object in the game world
        The update and render methods are virtual, so you can inherit them from GameObject and override the update or render functionality (see PlayerGameObject for reference)
    */
    class GameObject {

        public:
            // Constructor
            GameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture, int type = 3);

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
            int GetTimer(void) const;
            inline int GetType(void) const { return type_; }

            // Get bearing direction (direction in which the game object
            // is facing)
            glm::vec3 GetBearing(void) const;

            // Get vector pointing to the right side of the game object
            glm::vec3 GetRight(void) const;

            // Setters
            inline void SetPosition(const glm::vec3& position) { position_ = position; }
            inline void SetScale(float scale) { scale_ = scale; }
            void SetRotation(float angle);
            void SetTimer(float end_time);
            void SetTexture(GLuint texture) { texture_ = texture;}


        protected:
            // Object's Transform Variables
            glm::vec3 position_;
            float scale_;
            float angle_;

            // a total for the amount of time the object has been alive, helps us keep the enemy movement unique for now 
            double time_;

            // The type of game object; 0 is the player, 1 is an enemy, 2 is a collectible, and 3 is misc (just the background and explosions for now)
            int type_;

            // a timer for this objects explosion
            Timer* explosion_timer_;

            // Geometry
            Geometry *geometry_;
 
            // Shader
            Shader *shader_;

            // Object's texture reference
            GLuint texture_;



    }; // class GameObject

} // namespace game

#endif // GAME_OBJECT_H_
