#ifndef CHILD_GAME_OBJECT_H_
#define CHILD_GAME_OBJECT_H_

#include "game_object.h"

namespace game {

    // Inherits from GameObject
    class ChildGameObject : public GameObject {

        public:
            ChildGameObject(const glm::vec3 &position, Geometry *geom, Shader *shader, GLuint texture, GameObject *parent, int mode = 0);

            void SetRotation(float angle);

            // Update function for moving the blades object around
            void Update(double delta_time) override;

        private:
            GameObject * parent_;
            float angle_offset_;
            int mode_;

    }; // class ChildGameObject

} // namespace game

#endif // CHILD_GAME_OBJECT_H_