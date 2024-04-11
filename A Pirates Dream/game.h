#ifndef GAME_H_
#define GAME_H_

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <time.h>
#include <stdlib.h>

#include "shader.h"
#include "game_object.h"
#include "player_game_object.h"
#include "enemy_game_object.h"
#include "collectible_game_object.h"
#include "projectile_game_object.h"
#include "child_game_object.h"
#include "timer.h"
#include "audio_manager.h"

#define PrintException(exception_object)\
    std::cerr << exception_object.what() << std::endl

namespace game {

    // A class for holding the main game objects
    class Game {

        public:
            // Constructor and destructor
            Game(void);
            ~Game();

            // Call Init() before calling any other method
            // Initialize graphics libraries and main window
            void Init(void); 

            // Set up the game (scene, game objects, etc.)
            void Setup(void);

            // Run the game (keep the game active)
            void MainLoop(void); 

        private:
            // Main window: pointer to the GLFW window structure
            GLFWwindow *window_;

            // Sprite geometry
            Geometry *sprite_;

            // Particle geometry
            Geometry *bullet_particles_;

            Geometry *explosion_particles_;

            // Shader for rendering sprites in the scene
            Shader sprite_shader_;

            // Shader for rendering particles
            Shader particle_shader_;

            // References to textures
            // This needs to be a pointer
            GLuint *tex_;

            // The player object
            PlayerGameObject* player_;

            // blades
            GameObject* blades_;

            // The background
            GameObject* background_tile_;
            GameObject* end_screen_;

            // ui objects
            std::vector<GameObject*> health_objects_;
            std::vector<GameObject*> ui_objects_;
            std::vector<GameObject*> timer_objects_;

            // A vector of enemy entities
            std::vector<EnemyGameObject*> enemy_game_objects_;

            // A vecotr of collectible objects
            std::vector<CollectibleGameObject*> collectible_game_objects_;

            // A vector of basic game objects used for the explosions
            std::vector<GameObject*> explosions_;

            // a collection of bullet objects
            std::vector<ProjectileGameObject*> bullets_;
            std::vector<ProjectileGameObject*> spikes_;

            // particle object to render
            std::vector<GameObject*> particle_game_objects_;

            //
            EnemyGameObject* boss_game_object_;
            std::vector<ChildGameObject*> child_game_objects_;
            
            // Keep track of time
            double current_time_;

            // a tracker to determine the players health
            int player_health_;

            // 
            int score_;

            //
            bool boss_;
            

            // a tracker to determine the number of collectibles the player has gathered
            int buff_count_;

            // the number of enemies on the map
            int num_enemies_;

            // the number of buffs on the map
            int num_buffs_;

            // a timer thatll help with spawning enemies over time
            Timer* enemy_timer_;

            // a timer for spawning buffs over time
            Timer* buff_timer_;

            // a timer to determine if it is appropriate to spawn another bullet
            Timer* bullet_timer_;

            // instance of audio manager that lets us play wav files.
            audio_manager::AudioManager am;

            // refrence index for the explosion sound
            int explosion_index_;

            // refrence index for the looping music
            int background_index_;

            // Callback for when the window is resized
            static void ResizeCallback(GLFWwindow* window, int width, int height);

            // Set a specific texture
            void SetTexture(GLuint w, const char *fname);

            // Load all textures
            void SetAllTextures();

            // Handle user input
            void HandleControls(double delta_time);

            // Update all the game objects
            void Update(double delta_time);
 
            // Render the game world
            void Render(void);

    }; // class Game

} // namespace game

#endif // GAME_H_