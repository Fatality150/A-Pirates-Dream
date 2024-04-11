#include <stdexcept>
#include <string>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp> 
#include <SOIL/SOIL.h>
#include <iostream>

#include <path_config.h>

#include "sprite.h"
#include "shader.h"
#include "player_game_object.h"
#include "collectible_game_object.h"
#include "enemy_game_object.h"
#include "child_game_object.h"
#include "game.h"
#include "timer.h"
#include "particles.h"
#include "particle_system.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Globals that define the OpenGL window and viewport
const char *window_title_g = "Assignment 4";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const glm::vec3 viewport_background_color_g(0.0, 0.0, 1.0);

// Directory with game resources such as textures
const std::string resources_directory_g = RESOURCES_DIRECTORY;


Game::Game(void)
{
    // Don't do work in the constructor, leave it for the Init() function
}


void Game::Init(void)
{

    // Initialize the window management library (GLFW)
    if (!glfwInit()) {
        throw(std::runtime_error(std::string("Could not initialize the GLFW library")));
    }

    // Set whether window can be resized
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); 

    // Create a window and its OpenGL context
    window_ = glfwCreateWindow(window_width_g, window_height_g, window_title_g, NULL, NULL);
    if (!window_) {
        glfwTerminate();
        throw(std::runtime_error(std::string("Could not create window")));
    }

    // Make the window's OpenGL context the current one
    glfwMakeContextCurrent(window_);

    // Initialize the GLEW library to access OpenGL extensions
    // Need to do it after initializing an OpenGL context
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        throw(std::runtime_error(std::string("Could not initialize the GLEW library: ") + std::string((const char *)glewGetErrorString(err))));
    }

    // Set event callbacks
    glfwSetFramebufferSizeCallback(window_, ResizeCallback);

    // Initialize sprite geometry
    sprite_ = new Sprite();
    sprite_->CreateGeometry();

    // Initialize particle geometry
    bullet_particles_ = new Particles(glm::vec3(0.8f, 0.4f, 0.01f), .05f, 1.0f);
    bullet_particles_->CreateGeometry();

    explosion_particles_ = new Particles(glm::vec3(0.8f, 0.4f, 0.01f), 3.14f, 0.4f, 15.0f);
    explosion_particles_->CreateGeometry();

    // Initialize particle shader
    particle_shader_.Init((resources_directory_g+std::string("/particle_vertex_shader.glsl")).c_str(), (resources_directory_g+std::string("/particle_fragment_shader.glsl")).c_str());

    // Initialize sprite shader
    sprite_shader_.Init((resources_directory_g+std::string("/sprite_vertex_shader.glsl")).c_str(), (resources_directory_g+std::string("/sprite_fragment_shader.glsl")).c_str());

    // Initialize time
    current_time_ = 0.0;

    // Initialize player health
    player_health_ = 3;

    //
    score_ = 0;

    //
    boss_ = false;

    // Initialize buff count
    buff_count_ = 0;

    try
    {
        // Initialize audio manager
        am.Init(NULL);

        // Set position of listener
        am.SetListenerPosition(0.0, 0.0, 0.0);

        // Load first sound to be played
        std::string filename = std::string(RESOURCES_DIRECTORY).append(std::string("/audio/").append(std::string("frog.wav")));
        explosion_index_ = am.AddSound(filename.c_str());
        // Set sound properties
        am.SetSoundPosition(explosion_index_, 0.0, 0.0, 0.0);

        // Load second sound to be played
        filename = std::string(RESOURCES_DIRECTORY).append(std::string("/audio/").append(std::string("background.wav")));
        background_index_ = am.AddSound(filename.c_str());
        // Set sound properties
        am.SetSoundPosition(background_index_, -10.0, 0.0, 0.0);
        // Set the background music to loop
        am.SetLoop(background_index_, true);
        // Play the sound
        am.PlaySound(background_index_);
    }
    catch (std::exception &e)
    {
        PrintException(e);
    }
}


Game::~Game()
{
    // Free memory for all objects
    // Only need to delete objects that are not automatically freed
    delete sprite_;

    delete bullet_particles_;

    delete player_;
    
    delete background_tile_;

    for (int i = 0; i < enemy_game_objects_.size(); i++)
    {
        delete enemy_game_objects_[i];
    }

    for (int i = 0; i < collectible_game_objects_.size(); i++)
    {
        delete collectible_game_objects_[i];
    }

    for (int i = 0; i < particle_game_objects_.size(); i++)
    {
        delete particle_game_objects_[i];
    }

    delete enemy_timer_;
    delete buff_timer_;

    // Close window
    glfwDestroyWindow(window_);
    glfwTerminate();
    
    
}


void Game::Setup(void)
{

    // Setup the game world

    // Load textures
    SetAllTextures();

    // seed the random 
    srand (time(NULL));

    // Setup the player object (position, texture, vertex count)
    // Note that, in this specific implementation, the player object should always be the first object in the game object vector 
    player_ = new PlayerGameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[0]);
    float pi_over_two = glm::pi<float>() / 2.0f;
    player_->SetRotation(pi_over_two);
   

    for (int i = 0; i < player_health_; i++)
    {
        health_objects_.push_back( new GameObject(glm::vec3(0.0f,0.0f,0.0f), sprite_, &sprite_shader_, tex_[7]) );
        health_objects_.back()->SetScale(0.5f);
    }


    num_enemies_ = 0;


    // spawn 5 enemies to start at random positions
    while(num_enemies_ < 5)
    {
        // get a random x and y for the new enemy
        float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/8.0f)) - 4.0f;
        float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/8.0f)) - 4.0f;

        // make sure the new frog isnt too close to the player, and if it isnt add it to the list
        if (! ( player_->GetPosition().x + 1.4f > x && player_->GetPosition().x - 1.4f < x ) && ! ( player_->GetPosition().y + 1.4f > y && player_->GetPosition().y - 1.4f < y ) )
        {
            enemy_game_objects_.push_back(new EnemyGameObject(glm::vec3(x, y, 0.0f), sprite_, &sprite_shader_, tex_[1]));
            num_enemies_ ++;
        }
    }
    
    //set the base buffs to 0 since were not spawning any here
    num_buffs_ = 0;


    // Setup background
    background_tile_ = new GameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[3]);
    background_tile_->SetScale(100.0);

    for (int i = 0; i < 3; i++)
    {
        ui_objects_.push_back( new GameObject( glm::vec3(0.0f,0.0f,0.0f), sprite_, &sprite_shader_, tex_[9]) );
        ui_objects_.back()->SetScale(0.5);
    }

    timer_objects_.push_back( new GameObject (glm::vec3(0.0f), sprite_, &sprite_shader_, tex_[9]) );
    timer_objects_.back()->SetScale(0.5);
    timer_objects_.push_back( new GameObject (glm::vec3(0.0f), sprite_, &sprite_shader_, tex_[10]) );
    timer_objects_.back()->SetScale(0.5);

    // initialize the timers for spawning
    enemy_timer_ = new Timer();
    buff_timer_ = new Timer();
    bullet_timer_ = new Timer();
}


void Game::ResizeCallback(GLFWwindow* window, int width, int height)
{

    // Set OpenGL viewport based on framebuffer width and height
    glViewport(0, 0, width, height);
}


void Game::SetTexture(GLuint w, const char *fname)
{
    // Bind texture buffer
    glBindTexture(GL_TEXTURE_2D, w);

    // Load texture from a file to the buffer
    int width, height;
    unsigned char* image = SOIL_load_image(fname, &width, &height, 0, SOIL_LOAD_RGBA);
    if (!image){
        std::cout << "Cannot load texture " << fname << std::endl;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);

    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Texture Wrapping
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Texture Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    
}


void Game::SetAllTextures(void)
{
    // Load all textures that we will need
    // Declare all the textures here
    const char *texture[] = {"/textures/PirateShip.png", "/textures/NavyShip.png", "/textures/Apple.png", "/textures/Ocean.png", "/textures/boom.png", "/textures/SeaMonster.png", "/textures/Cannon Ball.png", "/textures/Health.png", "/textures/Barrel.png", "/textures/DamageBoost.png", "/textures/0.png", "/textures/1.png", "/textures/2.png", "/textures/3.png", "/textures/4.png", "/textures/5.png", "/textures/6.png", "/textures/7.png", "/textures/8.png", "/textures/9.png", "/textures/Spike.png", "/textures/Gold.png", "/textures/KrakenHead.png", "/textures/KrakenArm.png", "/textures/KrakenTentacle.png",  "/textures/Clear.png"};
    // Get number of declared textures
    int num_textures = sizeof(texture) / sizeof(char *);
    // Allocate a buffer for all texture references
    tex_ = new GLuint[num_textures];
    glGenTextures(num_textures, tex_);
    // Load each texture
    for (int i = 0; i < num_textures; i++){
        SetTexture(tex_[i], (resources_directory_g+std::string(texture[i])).c_str());
    }
    // Set first texture in the array as default
    glBindTexture(GL_TEXTURE_2D, tex_[0]);
}


void Game::MainLoop(void)
{
    // Loop while the user did not close the window
    double last_time = glfwGetTime();
    while (!glfwWindowShouldClose(window_)){

        // Calculate delta time
        double current_time = glfwGetTime();
        double delta_time = current_time - last_time;
        last_time = current_time;

        // Update window events like input handling
        glfwPollEvents();

        // Handle user input
        HandleControls(delta_time);

        // Update all the game objects
        Update(delta_time);

        // Render all the game objects
        Render();

        // Push buffer drawn in the background onto the display
        glfwSwapBuffers(window_);
    }
}


void Game::HandleControls(double delta_time)
{

    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, true);
    }

    if (boss_ && enemy_game_objects_.size() == 0) 
    {
        return;
    }

    if (player_health_ == 0) return;

    // Get current position and angle
    glm::vec3 curpos = player_->GetPosition();
    float angle = player_->GetRotation();
    // Compute current bearing direction
    glm::vec3 dir = player_->GetBearing();
    // Adjust motion increment and angle increment 
    // if translation or rotation is too slow
    float speed = delta_time*1000.0;
    float motion_increment = 0.001*speed;
    float angle_increment = (glm::pi<float>() / 1800.0f)*speed;

    // Check for player input and make changes accordingly

    // add to a velocity based on the keys being pressed

    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        //curpos += ;
        player_->SetVelocity((motion_increment/5)*dir);
    }
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
        //curpos -= motion_increment*dir;
        player_->SetVelocity(-(motion_increment/5)*dir);
    }
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
        angle -= angle_increment;
    }
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
        angle += angle_increment;
    }
    if (glfwGetKey(window_, GLFW_KEY_Q) == GLFW_PRESS) {
        //curpos += motion_increment*;
        player_->SetVelocity(-(motion_increment/5)*player_->GetRight());
    }
    if (glfwGetKey(window_, GLFW_KEY_E) == GLFW_PRESS) {
        //curpos -= motion_increment*player_->GetRight();
        player_->SetVelocity((motion_increment/5)*player_->GetRight());
    }
    if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        if (bullet_timer_->Finished() != 0)
        {
            bullets_.push_back(new ProjectileGameObject(glm::vec3(player_->GetPosition().x, player_->GetPosition().y, 0.0f), sprite_, &sprite_shader_, tex_[6]));
            bullets_.back()->SetScale(.25);
            bullets_.back()->SetVelocity(0.03f * player_->GetBearing());
            bullets_.back()->SetRotation( player_->GetRotation() - (glm::pi<float>() / 2.0f) );
            bullets_.back()->SetTimer(2);
            bullet_timer_->Start(1);

            //std::cout << atan2( bullets_.back()->GetVelocity().y, bullets_.back()->GetVelocity().x ) << std::endl;
            //bullets_.back()->GetPosition()
            GameObject *particles = new ParticleSystem(glm::vec3(0,0,0), bullet_particles_, &particle_shader_, tex_[6], bullets_.back());
            particles->SetScale(0.2);
            particle_game_objects_.push_back(particles); 
        }
    }
    if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        if (bullet_timer_->Finished() != 0)
        {
            spikes_.push_back(new ProjectileGameObject(glm::vec3(player_->GetPosition().x, player_->GetPosition().y, 0.0f), sprite_, &sprite_shader_, tex_[20]));
            spikes_.back()->SetScale(.5);
            spikes_.back()->SetVelocity(-0.001f * player_->GetBearing());
            //spikes_.back()->SetRotation( player_->GetRotation() - (glm::pi<float>() / 2.0f) );
            spikes_.back()->SetTimer(2);
            bullet_timer_->Start(3);
        }
    }

    
    player_->SetRotation(angle);
}


void Game::Update(double delta_time)
{

    // Update time
    current_time_ += delta_time;

    // Update all other game objects (for now just explosions)
    for (int i = 0; i < explosions_.size(); i++) {
        // Get the current game object
        GameObject* current_game_object = explosions_[i];

        // Update the current game object
        //std::cout << i << std::endl;
        current_game_object->Update(delta_time);

        //std::cout << i << " is inactive" << std::endl;
        //if the explosion is active and the timer is finished then we can proceed in removing the object, otherwise we continue on as normal.
        if (current_game_object->GetTimer() == 1)
        {
            // we added a temporary game object as the parent for the explosions so were gonna get rid of the memory we used before deleting the expolsion
            GameObject* temp;
            explosions_[i]->GetParent(&temp);
            delete temp;

            //std::cout << "another explosion fades away..." << std::endl;
            // free the space from the object list and remove it
            delete explosions_[i];
            explosions_.erase(explosions_.begin()+i);

            num_enemies_ --;
        }
           
    }

    if (boss_ && enemy_game_objects_.size() == 0) return;

    // if the player is dead then we want to start moving towards the shut down state
    if (player_health_ == 0)
    {
        // if the explosion vector is empty it means that they have all resolved and we can shut the game down now
        if (explosions_.size() == 0)
        {
            // Free memory for all objects
            // Only need to delete objects that are not automatically freed
            std::cout << "Game Over!" << std::endl;
            
            delete sprite_;
            
            delete background_tile_;
            

            for (int i = 0; i < enemy_game_objects_.size(); i++)
            {
                delete enemy_game_objects_[i];
            }

            for (int i = 0; i < collectible_game_objects_.size(); i++)
            {
                delete collectible_game_objects_[i];
            }

            for (int i = 0; i < particle_game_objects_.size(); i++)
            {
                delete particle_game_objects_[i];
            }

            delete enemy_timer_;
            delete buff_timer_;

            // Close window
            glfwDestroyWindow(window_);
            glfwTerminate();            
        }/**/ 
        else
        {
            player_->SetTexture(tex_[0]);
        }   
    }
    
    if (score_ >= 25 && !boss_)
    {
        enemy_game_objects_.push_back(new EnemyGameObject( player_->GetPosition() + glm::vec3(6.0f,0.0f,0.0f), sprite_, &sprite_shader_, tex_[22], 15, 1));
        
        /*
        child_game_objects_.push_back(new ChildGameObject (enemy_game_objects_.back()->GetPosition(), sprite_, &sprite_shader_, tex_[23], enemy_game_objects_.back()) );
        child_game_objects_.back()->SetRotation((glm::pi<float>() / 2.0f) );
        child_game_objects_.back()->SetScale(0.5);
        child_game_objects_.push_back(new ChildGameObject (child_game_objects_.back()->GetPosition(), sprite_, &sprite_shader_, tex_[23], child_game_objects_.back()) );
        child_game_objects_.back()->SetRotation((glm::pi<float>() / 1.0f) );
        child_game_objects_.back()->SetScale(0.5);
        child_game_objects_.push_back(new ChildGameObject (child_game_objects_.back()->GetPosition(), sprite_, &sprite_shader_, tex_[23], child_game_objects_.back()) );
        child_game_objects_.back()->SetRotation((glm::pi<float>() / 3.0f) );
        child_game_objects_.back()->SetScale(0.5);*/

        boss_ = true;
    }

    // handling enemy spawning (same as the buff spawner below)
    if (num_enemies_ < 5 && player_health_ > 0 && score_ < 25)
    {
        if (enemy_timer_->Finished() == 1 && score_ + enemy_game_objects_.size() < 25)
        {
            while(true)
            {
                float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10.0f)) - 5.0f;
                float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/10.0f)) - 5.0f;

                if (! ( player_->GetPosition().x + 2.0f > x && player_->GetPosition().x - 2.0f < x ) && ! ( player_->GetPosition().y + 2.0f > y && player_->GetPosition().y - 2.0f < y ) )
                {
                    
                    if (score_ > 10)
                    {
                        if ( rand() / (RAND_MAX / 5) < 3 )
                        {
                            enemy_game_objects_.push_back( new EnemyGameObject(glm::vec3(x, y, 0.0f), sprite_, &sprite_shader_, tex_[5], 3, 1) );
                            num_enemies_ ++;
                        }
                        else
                        {
                            enemy_game_objects_.push_back( new EnemyGameObject(glm::vec3(x, y, 0.0f), sprite_, &sprite_shader_, tex_[1] ) );
                            num_enemies_ ++;
                        }
                    }
                    else
                    {
                        enemy_game_objects_.push_back( new EnemyGameObject(glm::vec3(x, y, 0.0f), sprite_, &sprite_shader_, tex_[1] ) );
                        num_enemies_ ++;
                    }
                    
                    break;
                }
            }
        }
        else if (enemy_timer_->Finished() == 2)
        {
            enemy_timer_->Start(5);
        }
    }

    //handling buff spawning, for now well make sure that we hover around 3 buffs at once
    if (num_buffs_ < 5 && player_health_ > 0)
    {
        // if the timers done then we can continue
        if (buff_timer_->Finished() == 1)
        {
            // were gonna loop around until we get a value that satifies our conditions
            while(true)
            {
                // randomly generate an x and y value for the entity
                float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/8.0f)) - 4.0f;
                float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/8.0f)) - 4.0f;

                // if its not too close to the player we can accept the spawn ( this is pretty inneficien however its not very likely this will cause any large scale lag on this scale)
                if (! ( player_->GetPosition().x + 1.0f > x && player_->GetPosition().x - 1.0f < x ) && ! ( player_->GetPosition().y + 1.0f > y && player_->GetPosition().y - 1.0f < y ) )
                {
                    // add a new entity to the list and increment the counter
                    collectible_game_objects_.push_back( new CollectibleGameObject(glm::vec3(x, y, 0.0f), sprite_, &sprite_shader_, tex_[8]));
                    collectible_game_objects_.back()->SetScale(0.5);
                    //collectible_game_objects_.back()->SetRotation(glm::pi<float>() / 2.0f);
                    num_buffs_ ++;
                    break;
                }
            }
        }
        else if (buff_timer_->Finished() == 2)
        {
            buff_timer_->Start(5);
        }
    }
    
    // update the player since we not check for player player collision
    if (player_health_ > 0) 
    {
        player_->Update(delta_time);
    }

    //
    for (int i = 0; i < particle_game_objects_.size(); i++)
    {
        particle_game_objects_[i]->Update(delta_time);
    }

    // update the player since we not check for player player collision
    background_tile_->Update(delta_time);
    
    for (int i = 0; i < child_game_objects_.size(); i++)
    {
        child_game_objects_[i]->Update(delta_time);
    }

    // update all enemy game objects
    for (int i = 0; i < enemy_game_objects_.size(); i++) 
    {
        // Get the current game object
        EnemyGameObject* current_game_object = enemy_game_objects_[i];

        // Update the current game object
        //std::cout << i << std::endl;
        current_game_object->Update(delta_time);
        

        float distance = glm::length(current_game_object->GetPosition() - player_->GetPosition());

        // check if we get close to an enemy, if so we wanna set it to patrolling, give it its first tagert
        if (distance < 1.8f && player_health_ > 0)
        {
            // if we get close enough we wanna set it to patrolling and if the object is patrolling we wanna
            if (enemy_game_objects_[i]->GetState() == 0)
            {
                enemy_game_objects_[i]->SetTarget(player_->GetPosition());
            }
        }/**/

        // if the entity is intercepting we wanna update the target if its timer is done
        if ( enemy_game_objects_[i]->GetState() == 1 && enemy_game_objects_[i]->GetTimer() == 1)
        {
            enemy_game_objects_[i]->SetTarget(player_->GetPosition());
        }

        // If distance is below a threshold, we have a collision
        if (distance < 0.8f && player_health_ > 0 && enemy_game_objects_[i]->GetHitTimer() != 0)
        {
            
            //std::cout << "Contact!" << std::endl;
            
            //here were just getting the position of the object we wanna blow up
            glm::vec3 pos = current_game_object->GetPosition();

            if (enemy_game_objects_[i]->GetHealth() == 1)
            {
                int r = rand() / (RAND_MAX / 5);
                if ( r == 2 )
                {
                    collectible_game_objects_.push_back(new CollectibleGameObject(pos, sprite_, &sprite_shader_, tex_[2], 1) );
                }
                else if (r == 1)
                {
                    collectible_game_objects_.push_back(new CollectibleGameObject(pos, sprite_, &sprite_shader_, tex_[21], 2));
                }
                

                // we them blow it up metaphorically by deleting it (dont wanna waste space)
                delete current_game_object;
                enemy_game_objects_.erase(enemy_game_objects_.begin() + i);

                // we then replace the object with an explosion, set the explosion to false so that we dont accidentally blow up the explosion (that would be weird), and set a timer for how long itll stay on screen
                //pos
                GameObject *particles = new ParticleSystem(glm::vec3(0,0,0), explosion_particles_, &particle_shader_, tex_[4], new GameObject(pos, sprite_, &sprite_shader_, tex_[4]));
                particles->SetScale(0.2);
                particles->SetTimer(1.0f);
                explosions_.push_back(particles); 

                // and next were gonna play a nom sound cause he ate that thang
                if (! am.SoundIsPlaying(explosion_index_) ) am.PlaySound(explosion_index_);

                score_++;
            }
            else
            {
                enemy_game_objects_[i]->Hit();
                if (player_->GetTimer() == 0) enemy_game_objects_[i]->Hit();
                enemy_game_objects_[i]->SetHitTimer();
            }

            // player hit another object so were gonna take 1 health away
            player_->SetTexture(tex_[0]);
            player_health_ -= 1;
            
            // same as above but for the player if we hit 3 enemies
            if (player_health_ == 0)
            {
                glm::vec3 pos = player_->GetPosition();

                delete player_;

                //pos
                GameObject *particles = new ParticleSystem(glm::vec3(0,0,0), explosion_particles_, &particle_shader_, tex_[4], new GameObject(pos, sprite_, &sprite_shader_, tex_[4]));
                particles->SetScale(0.2);
                particles->SetTimer(1.0f);
                explosions_.push_back(particles); 
            }
            
            // restart from the beginning since we shrunk the enemy vector by 1 after the collision
            i--;

            if (i < 0) goto endloop;

            continue;
        }
    }

    // update all collectible game objects
    for (int i = 0; i < collectible_game_objects_.size(); i++) 
    {
        // Get the current game object
        CollectibleGameObject* current_game_object = collectible_game_objects_[i];

        // Update the current game object
        //std::cout << i << std::endl;
        current_game_object->Update(delta_time);


        float distance = glm::length(current_game_object->GetPosition() - player_->GetPosition());
        // check if we contacted a collectible
        if (distance < 0.6f && player_health_ > 0)
        {
            // were gonna get rid of the object first since we dont need it anymore

            if (collectible_game_objects_[i]->GetType() == 0)
            {
                // were gonna change the values for 
                num_buffs_ --;
                buff_count_++;

                // if the number of buffs weve collected is greater than or equal to 5 were gonna go into gold mode
                if (buff_count_ >= 5)
                {
                    // set the timer on the power up
                    player_->SetTimer(10.0f);
                    // reset the buff count so we dont chain power ups
                    buff_count_ = 0;
                }
            }
            else if (collectible_game_objects_[i]->GetType() == 1 && player_health_ < 3)
            {
                player_health_++;
            }
            else if (collectible_game_objects_[i]->GetType() == 2)
            {
                score_++;
            }

            delete current_game_object;
            collectible_game_objects_.erase(collectible_game_objects_.begin()+i);
            // were gonna move back to the same i since everything after the object we just deleted shifted down one (i+1 is now just i) and we wouldnt wanna miss any collision
            i--;
        }
    }

    for (int i = 0; i < bullets_.size(); i++)
    {
        bullets_[i]->Update(delta_time);

        for (int j = 0; j < enemy_game_objects_.size(); j++)
        {
            glm::vec3 d = bullets_[i]->GetVelocity();

            // vector for the line between the start of the bullets path and the centre of the circle
            glm::vec3 sc = bullets_[i]->GetPosition() - enemy_game_objects_[j]->GetPosition();

            double time = bullets_[i]->GetTime();

            //std::cout << j << ": sc = ()" << sc.x << ", " << sc.y << "), d = (" << d.x * time << ", " << d.y * time << ")" << std::endl;

            double a = glm::dot(d, d);
            double b = 2 * glm::dot(d, sc);
            double c = glm::dot(sc, sc) - 0.1f;

            float disc = pow(b, 2) - 4 * a * c;

            if (disc >= 0)
            {
                disc = sqrt(disc);

                float t1 = ((-b) - disc) / (2 * a);
                float t2 = ((-b) + disc) / (2 * a);

                //std::cout << j << ": t1 = " << t1 << ", t2 =  " << t2 << std::endl;

                if(t1 <= 0 && t2 >= 1)
                {
                    if (enemy_game_objects_[j]->GetHealth() <= 1)
                    {
                        int r = rand() / (RAND_MAX / 5);
                        if ( r == 2 )
                        {
                            collectible_game_objects_.push_back(new CollectibleGameObject(enemy_game_objects_[j]->GetPosition(), sprite_, &sprite_shader_, tex_[2], 1));
                        }
                        else if (r == 1)
                        {
                            collectible_game_objects_.push_back(new CollectibleGameObject(enemy_game_objects_[j]->GetPosition(), sprite_, &sprite_shader_, tex_[21], 2));
                        }

                        //enemy_game_objects_[j]->GetPosition()
                        GameObject *particles = new ParticleSystem(glm::vec3(0,0,0), explosion_particles_, &particle_shader_, tex_[4], new GameObject(enemy_game_objects_[j]->GetPosition(), sprite_, &sprite_shader_, tex_[4]));
                        particles->SetScale(0.2);
                        particles->SetTimer(1.0f);
                        explosions_.push_back(particles); 

                        delete enemy_game_objects_[j];
                        enemy_game_objects_.erase(enemy_game_objects_.begin()+j);

                        score_++;
                    }
                    else
                    {
                        enemy_game_objects_[j]->Hit();
                        if (player_->GetTimer() == 0) enemy_game_objects_[j]->Hit();
                    }
                    

                    delete bullets_[i];
                    bullets_.erase(bullets_.begin()+i);

                    delete particle_game_objects_[0];
                    particle_game_objects_.erase(particle_game_objects_.begin());

                    if (! am.SoundIsPlaying(explosion_index_) ) am.PlaySound(explosion_index_);

                    i--;

                    if (i < 0) goto endloop;
                    break;
                }
            }
        }

        if ( bullets_[i]->GetTimer() == 2 )
        {
            delete bullets_[i];
            bullets_.erase(bullets_.begin()+i);

            delete particle_game_objects_[0];
            particle_game_objects_.erase(particle_game_objects_.begin());

            i--;
            if (i < 0) goto endloop;
            break;
        }
    }

    for (int i = 0; i < spikes_.size(); i++)
    {
        spikes_[i]->Update(delta_time);

        /**/

        for (int j = 0; j < enemy_game_objects_.size(); j++)
        {
            float distance = glm::length(spikes_[i]->GetPosition() - enemy_game_objects_[j]->GetPosition());
            // If distance is below a threshold, we have a collision
            if (distance < 0.8f)
            {
                if (enemy_game_objects_[j]->GetHealth() <= 1)
                {
                    int r = rand() / (RAND_MAX / 5);
                    if ( r == 2 )
                    {
                        collectible_game_objects_.push_back( new CollectibleGameObject(enemy_game_objects_[j]->GetPosition(), sprite_, &sprite_shader_, tex_[2], 1) );
                    }
                    else if (r == 1)
                    {
                        collectible_game_objects_.push_back(new CollectibleGameObject(enemy_game_objects_[j]->GetPosition(), sprite_, &sprite_shader_, tex_[21], 2));
                    }

                    //enemy_game_objects_[j]->GetPosition()
                    GameObject *particles = new ParticleSystem(glm::vec3(0,0,0), explosion_particles_, &particle_shader_, tex_[4], new GameObject(enemy_game_objects_[j]->GetPosition(), sprite_, &sprite_shader_, tex_[4]));
                    particles->SetScale(0.2);
                    particles->SetTimer(1.0f);
                    explosions_.push_back(particles); 

                    delete enemy_game_objects_[j];
                    enemy_game_objects_.erase(enemy_game_objects_.begin()+j);

                    score_++;
                }
                else
                {
                    enemy_game_objects_[j]->Hit();
                    if (player_->GetTimer() == 0) enemy_game_objects_[j]->Hit();
                }
                
                delete spikes_[i];
                spikes_.erase(spikes_.begin()+i);

                if (! am.SoundIsPlaying(explosion_index_) ) am.PlaySound(explosion_index_);

                i--;

                if (i < 0) goto endloop;
                break;
            }
            
        }

        if ( spikes_[i]->GetTimer() == 2 )
        {
            delete spikes_[i];
            spikes_.erase(spikes_.begin()+i);

            i--;
        }
    }

    glm::vec3 pos = player_->GetPosition();
    for (int i = 0; i < health_objects_.size(); i++)
    {
        health_objects_[i]->SetPosition( glm::vec3(pos.x - 5.0f + (0.5f * i), pos.y + 3.5f, 0.0f ) );
    }

    for (int i = ui_objects_.size()-1; i >= 0; i--)
    {
        ui_objects_[i]->SetPosition( glm::vec3(pos.x + 0.5f - (0.5f * i), pos.y + 3.5f, 0.0f ) );
        ui_objects_[i]->SetTexture(tex_[(score_ / static_cast<int> ( pow(10, i) ) ) % 10 + 10]);
    }

    if (player_->GetTimer(0) == 0)
    {
        timer_objects_[0]->SetPosition( glm::vec3(pos.x + 4.5f, pos.y + 3.5f, 0.0f ) );
        timer_objects_[1]->SetPosition( glm::vec3(pos.x + 5.0f, pos.y + 3.5f, 0.0f ) );
        timer_objects_[1]->SetTexture(tex_[static_cast<int> ( player_->GetTimerTime() )  % 10 + 10]);
    }

    

    endloop:
    {
        if (boss_ && enemy_game_objects_.size() == 0) 
        {
            end_screen_ = new GameObject(player_->GetPosition(), sprite_, &sprite_shader_, tex_[25]);
            end_screen_->SetScale(10);
            player_->SetVelocity(glm::vec3(0,0,0));
        }
        return;
    }
        
}


void Game::Render(void){

    // Clear background
    glClearColor(viewport_background_color_g.r,
                 viewport_background_color_g.g,
                 viewport_background_color_g.b, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use aspect ratio to properly scale the window
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    glm::mat4 window_scale_matrix;
    if (width > height){
        float aspect_ratio = ((float) width)/((float) height);
        window_scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f/aspect_ratio, 1.0f, 1.0f));
    } else {
        float aspect_ratio = ((float) height)/((float) width);
        window_scale_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f/aspect_ratio, 1.0f));
    }

    // Set view to zoom out, centered by default at 0,0
    float camera_zoom = 0.25f;
    glm::mat4 camera_zoom_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(camera_zoom, camera_zoom, camera_zoom));
    glm::mat4 view_matrix = window_scale_matrix * camera_zoom_matrix;
    

    // getting the inverse of our position so we can translate the camera in the same direction as the player
    glm::vec3 vector_translation;
    if (player_health_ > 0) vector_translation = glm::vec3(-1.0f * player_->GetPosition().x, -1.0f * player_->GetPosition().y, 0.0f);
    else vector_translation = glm::vec3(-1.0f * explosions_.back()->GetPosition().x, -1.0f * explosions_.back()->GetPosition().y, 0.0f);

    // updating the matrix to include the translation
    view_matrix = glm::translate(view_matrix, vector_translation);

    if (boss_ && enemy_game_objects_.size() == 0) 
    {
        end_screen_->Render(view_matrix, current_time_);
    }
    

    // Render all game objects
    if (player_health_ > 0) 
    {
        for (int i = 0; i < player_health_; i++)
        {
            health_objects_[i]->Render(view_matrix, current_time_);
        }

        for (int i = 0; i < ui_objects_.size(); i++)
        {
            ui_objects_[i]->Render(view_matrix, current_time_);
        }

        if (player_->GetTimer(0) == 0)
        {
            for (int i = 0; i < timer_objects_.size(); i++)
            {
                timer_objects_[i]->Render(view_matrix, current_time_);
            }
        }
        

        player_->Render(view_matrix, current_time_);
    }

    for (int i = 0; i < enemy_game_objects_.size(); i++)
    {
        enemy_game_objects_[i]->Render(view_matrix, current_time_);
    }

    for (int i = 0; i < child_game_objects_.size(); i++)
    {
        child_game_objects_[i]->Render(view_matrix, current_time_);
    }

    for (int i = 0; i < collectible_game_objects_.size(); i++)
    {
        collectible_game_objects_[i]->Render(view_matrix, current_time_);
    }

    for ( int i = 0; i < bullets_.size(); i++)
    {
        bullets_[i]->Render(view_matrix, current_time_);
    }

    for ( int i = 0; i < spikes_.size(); i++)
    {
        spikes_[i]->Render(view_matrix, current_time_);
    }

    sprite_->SetScale(10.0f);

    background_tile_->Render(view_matrix, current_time_);

    sprite_->SetScale(1.0f);

    for (int i = 0; i < explosions_.size(); i++)
    {
        explosions_[i]->Render(view_matrix, current_time_);
    }

    for (int i = 0; i < particle_game_objects_.size(); i++)
    {
        particle_game_objects_[i]->Render(view_matrix, current_time_);
    }
}
      
} // namespace game
