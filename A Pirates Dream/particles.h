#ifndef PARTICLES_H_
#define PARTICLES_H_

#include "geometry.h"

#define NUM_PARTICLES 4000

namespace game {

    // A set of particles that can be rendered
    class Particles : public Geometry {

        public:
            Particles(const glm::vec3 &color_value = glm::vec3(0.8f, 0.4f, 0.01f), float spread = 0.13f, float length = 0.8f, float t = 1.0f );

            // Create the geometry (called once)
            void CreateGeometry(void);

            // Use the geometry
            void SetGeometry(GLuint shader_program);

        private:

            glm::vec3 color_value_;
            float spread_;
            float length_;
            float t_;

    }; // class Particles
} // namespace game

#endif // PARTICLES_H_
