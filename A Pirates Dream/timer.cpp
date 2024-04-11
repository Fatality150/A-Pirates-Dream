#include <GLFW/glfw3.h>
#include <iostream>

#include "timer.h"

namespace game {

Timer::Timer(void)
{
    // by default a timer is not active
    is_active_ = false;
}


Timer::~Timer(void)
{
}


void Timer::Start(float end_time)
{
    // set the start time to now
    start_time_ = glfwGetTime();

    // mark how long we want the timer to run   
    end_time_ = end_time;

    // and set the timer to be active
    is_active_ = true;
}



int Timer::Finished(int i)
{
    // if the timer hasnt been activated than we can have possibly reached the end
    if (!is_active_) return 2;

    // if the current time - when we started the timer is greater than the number of seconds we want to run the timer than the timer has finished
    if (glfwGetTime() - start_time_ > end_time_) 
    {
        // lets set it back to inactive so we can tell if we can use it again
        if (i == 1) is_active_ = false;
        return 1;
    }
    
    // otherwise return false
    return 0;
}

} // namespace game
