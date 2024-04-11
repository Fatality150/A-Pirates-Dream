#ifndef ENEMY_GAME_OBJECT_H_
#define ENEMY_GAME_OBJECT_H_

#include "game_object.h"

#define PATROLLING 0
#define INTERCEPTING 1

namespace game {

    // Inherits from GameObject
    class EnemyGameObject : public GameObject {

        public:
            EnemyGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture, int health = 1, int state = 0);
            ~EnemyGameObject();

            // Update function for moving the Enemy object around
            void Update(double delta_time) override;

            inline int GetState(void) const { return state_; }
            inline int GetHealth(void) const { return health_; }
            inline int GetHitTimer(void) const { return hit_timer_->Finished(); }

            inline void SetIntercepting(void) { state_ = INTERCEPTING; }
            void SetTarget(glm::vec3 &position);
            inline void Hit(void) { health_-= 1;}
            inline void SetHitTimer(float t = 3.0f) { hit_timer_->Start(t); }


        private:

            // how much health the enemy has
            int health_;

            // the enemies state, 0 is patrolling, 1 is  intercepting
            int state_;

            Timer* hit_timer_;

            // the point around which were gonna rotate the enemy
            glm::vec3 centre_point_;

            // the point from which the enemy started moving from
            glm::vec3 start_;

            // the time at which the enemy started moving towards the target
            double start_time_;

            // the target where which we wanna move
            glm::vec3 target_;



    }; // class EnemyGameObject

} // namespace game

#endif // Enemy_GAME_OBJECT_H_
