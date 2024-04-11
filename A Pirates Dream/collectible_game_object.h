#ifndef COLLECTIBLE_GAME_OBJECT_H_
#define COLLECTIBLE_GAME_OBJECT_H_

#include "game_object.h"

namespace game {

    // Inherits from GameObject
    class CollectibleGameObject : public GameObject {

        public:
            CollectibleGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture, int type = 0 );

            // Update function for moving the Collectible object around
            void Update(double delta_time) override;

            inline int GetType(void) const { return type_; }

        private:
            int type_;
            glm::vec3 start_pos_;

    }; // class CollectibleGameObject

} // namespace game

#endif // Collectible_GAME_OBJECT_H_
