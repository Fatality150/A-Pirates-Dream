#ifndef ENEMY_GAME_OBJECT_H_
#define ENEMY_GAME_OBJECT_H_

#include "game_object.h"

#define PATROLLING 0
#define INTERCEPTING 1

namespace game {

    // Inherits from GameObject
    class EnemyGameObject : public GameObject {

        public:
            EnemyGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture);

            // Update function for moving the Enemy object around
            void Update(double delta_time) override;

        private:

            // the enemies state, 0 is patrolling, 1 is  intercepting
            int state_;

            // the point around which were gonna rotate the enemy
            glm::vec3 centre_point_;

            // the target where which we wanna move
            glm::vec3 target_;



    }; // class EnemyGameObject

} // namespace game

#endif // Enemy_GAME_OBJECT_H_
