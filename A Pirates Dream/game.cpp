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
#include "game.h"
#include "timer.h"

namespace game {

// Some configuration constants
// They are written here as global variables, but ideally they should be loaded from a configuration file

// Globals that define the OpenGL window and viewport
const char *window_title_g = "Game Demo";
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

    // Initialize sprite shader
    sprite_shader_.Init((resources_directory_g+std::string("/sprite_vertex_shader.glsl")).c_str(), (resources_directory_g+std::string("/sprite_fragment_shader.glsl")).c_str());

    // Initialize time
    current_time_ = 0.0;

    // Initialize player health
    player_health_ = 3;

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
    for (int i = 0; i < game_objects_.size(); i++){
        delete game_objects_[i];
    }

    // free the timers
    delete enemy_timer_;
    delete buff_timer_;
    
    //shut down the music player
    am.ShutDown();
    
    // Close window
    glfwDestroyWindow(window_);
    glfwTerminate();
    
    
}


void Game::Setup(void)
{

    // Setup the game world

    // Load textures
    SetAllTextures();

    // Setup the player object (position, texture, vertex count)
    // Note that, in this specific implementation, the player object should always be the first object in the game object vector 
    game_objects_.push_back(new PlayerGameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[0]));
    float pi_over_two = glm::pi<float>() / 2.0f;
    game_objects_[0]->SetRotation(pi_over_two);

    // set the base enemies to 0 since were not spawning any here
    num_enemies_ = 0;
    
    //set the base buffs to 0 since were not spawning any here
    num_buffs_ = 0;


    // Setup background
    // In this specific implementation, the background is always the
    // last object
    GameObject *background = new GameObject(glm::vec3(0.0f, 0.0f, 0.0f), sprite_, &sprite_shader_, tex_[3]);
    background->SetScale(20.0);
    game_objects_.push_back(background);

    // seed the random 
    srand (time(NULL));

    // initialize the timers for spawning
    enemy_timer_ = new Timer();
    buff_timer_ = new Timer();
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

    // Texture Wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Texture Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


void Game::SetAllTextures(void)
{
    // Load all textures that we will need
    // Declare all the textures here
    const char *texture[] = {"/textures/ship.png", "/textures/stars.png", "/textures/boom.png"};
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

    if (player_health_ == 0) return;

    // Get player game object
    GameObject *player = game_objects_[0];
    // Get current position and angle
    glm::vec3 curpos = player->GetPosition();
    float angle = player->GetRotation();
    // Compute current bearing direction
    glm::vec3 dir = player->GetBearing();
    // Adjust motion increment and angle increment 
    // if translation or rotation is too slow
    float speed = delta_time*1000.0;
    float motion_increment = 0.001*speed;
    float angle_increment = (glm::pi<float>() / 1800.0f)*speed;

    // Check for player input and make changes accordingly

    // changed the input system to allow for better movement; player now can press both buttons at once 
    //and will not move, instead of moving in a preset direction based on the order of the if statements.
    
    // takes the net changes from each button press before making changes to the player position and angle.

    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        curpos += motion_increment*dir;
        //player->SetPosition(curpos + motion_increment*dir);
    }
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
        curpos -= motion_increment*dir;
        //player->SetPosition(curpos - motion_increment*dir);
    }
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
        angle -= angle_increment;
        //player->SetRotation(angle - angle_increment);
    }
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
        angle += angle_increment;
        //player->SetRotation(angle + angle_increment);
    }
    if (glfwGetKey(window_, GLFW_KEY_Z) == GLFW_PRESS) {
        curpos += motion_increment*player->GetRight();
        //player->SetPosition(curpos - motion_increment*player->GetRight());
    }
    if (glfwGetKey(window_, GLFW_KEY_C) == GLFW_PRESS) {
        curpos -= motion_increment*player->GetRight();
        //player->SetPosition(curpos + motion_increment*player->GetRight());
    }
    
    player->SetPosition(curpos);
    player->SetRotation(angle);
}


void Game::Update(double delta_time)
{

    // Update time
    current_time_ += delta_time;
    
    GameObject *player = game_objects_[0];
    // if the player is dead and the explosion timers are empty, just copy the destructor and shut down the window
    if (player->GetTimer() == 1)
    {
       if (player_health_ == 0)
        {
            // Free memory for all objects
            // Only need to delete objects that are not automatically freed
            delete sprite_;
            for (int i = 0; i < game_objects_.size(); i++){
                delete game_objects_[i];
            }

            // Close window
            glfwDestroyWindow(window_);
            glfwTerminate();

            std::cout << "Game Over!" << std::endl;
        }
        else
        {
            player->SetTexture(tex_[0]);
        }
    }
    
    // update the player since we not check for player player collision
    game_objects_[0]->Update(delta_time);


    // handling enemy spawning (same as the buff spawner below)
    if (num_enemies_ < 5)
    {
        if (enemy_timer_->Finished() == 1)
        {
            while(true)
            {
                float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/8.0f)) - 4.0f;
                float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX/8.0f)) - 4.0f;

                if (! ( game_objects_[0]->GetPosition().x + 1.0f > x && game_objects_[0]->GetPosition().x - 1.0f < x ) && ! ( game_objects_[0]->GetPosition().y + 1.0f > y && game_objects_[0]->GetPosition().y - 1.0f < y ) )
                {
                    game_objects_.insert(game_objects_.end()-1, new EnemyGameObject(glm::vec3(x, y, 0.0f), sprite_, &sprite_shader_, tex_[1]));
                    num_enemies_ ++;
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
    if (num_buffs_ < 3)
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
                if (! ( game_objects_[0]->GetPosition().x + 1.0f > x && game_objects_[0]->GetPosition().x - 1.0f < x ) && ! ( game_objects_[0]->GetPosition().y + 1.0f > y && game_objects_[0]->GetPosition().y - 1.0f < y ) )
                {
                    // add a new entity to the list and increment the counter
                    game_objects_.insert(game_objects_.end()-1, new CollectibleGameObject(glm::vec3(x, y, 0.0f), sprite_, &sprite_shader_, tex_[2]));
                    game_objects_[game_objects_.size()-2]->SetScale(0.5);
                    game_objects_[game_objects_.size()-2]->SetRotation(glm::pi<float>() / 2.0f);
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


    // Update all game objects
    for (int i = 1; i < game_objects_.size(); i++) {
        // Get the current game object
        GameObject* current_game_object = game_objects_[i];

        // Update the current game object
        //std::cout << i << std::endl;
        current_game_object->Update(delta_time);

        // if the object is inactive (background or an explosion)
        if (current_game_object->GetType() == 3)
        {
            //std::cout << i << " is inactive" << std::endl;
            //if the explosion is active and the timer is finished then we can proceed in removing the object, otherwise we continue on as normal.
            if (current_game_object->GetTimer() == 1)
            {
                //std::cout << "another explosion fades away..." << std::endl;
                // free the space from the object list and remove it
                delete game_objects_[i];
                game_objects_.erase(game_objects_.begin()+i);

                num_enemies_ --;

                i--;
                continue;
            }
        }

        
        float distance = glm::length(current_game_object->GetPosition() - game_objects_[0]->GetPosition());
        // If distance is below a threshold, we have a collision
        if (distance < 0.8f && current_game_object->GetType() == 1) 
        {
            
            //std::cout << "Contact!" << std::endl;
            
            //here were just getting the position of the object we wanna blow up
            glm::vec3 pos = current_game_object->GetPosition();

            // we them blow it up metaphorically by deleting it (dont wanna waste space)
            delete current_game_object;

            // we then replace the object with an explosion, set the explosion to false so that we dont accidentally blow up the explosion (that would be weird), and set a timer for how long itll stay on screen
            game_objects_[i] = new GameObject(pos, sprite_, &sprite_shader_, tex_[4]);
            game_objects_[i]->SetTimer(5.0f);

            if (player->GetTimer() == 1 || player->GetTimer() == 2)
            {
                // player hit another object so were gonna take 1 health away
                player_health_ -= 1;
            }
            

            // and next were gonna play a nom sound cause he ate that thang
            am.PlaySound(explosion_index_);

            // same as above but for the player if we hit 3 enemies
            if (player_health_ == 0)
            {
                glm::vec3 pos = game_objects_[0]->GetPosition();

                delete game_objects_[0];

                game_objects_[0] = new GameObject(pos, sprite_, &sprite_shader_, tex_[4]);
                game_objects_[0]->SetTimer(5.0f);
            }
        }

        // check if we get close to an enemy
        if (distance < 1.4f && current_game_object->GetType() == 1)
        {

        }

        // check if we contacted a collectible
        if (distance < 0.6f && current_game_object->GetType() == 2) 
        {
            // were gonna get rid of the object first since we dont need it anymore
            delete current_game_object;
            game_objects_.erase(game_objects_.begin()+i);

            // were gonna change the values for 
            num_buffs_ --;
            buff_count_++;

            // if the number of buffs weve collected is greater than or equal to 5 were gonna go into gold mode
            if (buff_count_ >= 5)
            {
                // change the texture of the player
                player->SetTexture(tex_[5]);
                // set the timer on the power up
                player->SetTimer(10.0f);
                // reset the buff count so we dont chain power ups
                buff_count_ = 0;
            }

            // were gonna move back to the same i since everything after the object we just deleted shifted down one (i+1 is now just i) and we wouldnt wanna miss any collision
            i--;
        }
        
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

    // Render all game objects
    for (int i = 0; i < game_objects_.size(); i++) {
        game_objects_[i]->Render(view_matrix, current_time_);
    }
}
      
} // namespace game
