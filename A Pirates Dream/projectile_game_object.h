#ifndef PROJECTILE_GAME_OBJECT_H_
#define PROJECTILE_GAME_OBJECT_H_

#include "game_object.h"

namespace game {

    // Inherits from GameObject
    class ProjectileGameObject : public GameObject {

        public:
            ProjectileGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture);

            void SetVelocity(glm::vec3 &velocity) override;

            // Update function for moving the player object around
            void Update(double delta_time) override;

            // getter
            inline glm::vec3 GetStart(void) const { return start_pos_;}

        private:
            glm::vec3 start_pos_;

    }; // class ProjectileGameObject

} // namespace game

#endif // PLAYER_GAME_OBJECT_H_