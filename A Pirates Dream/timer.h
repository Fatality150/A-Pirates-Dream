#ifndef TIMER_H_
#define TIMER_H_

#include <GLFW/glfw3.h>

namespace game {

    // A class implementing a simple timer
    class Timer {

        public:
            // Constructor and destructor
            Timer(void);
            ~Timer();

            // Start the timer now: end time given in seconds
            void Start(float end_time); 

            inline double GetTime(void) { return end_time_ - (glfwGetTime() - start_time_);   }

            // Check if timer has finished
            int Finished(int i = 1);

        private:
            // the time we started the timer
            float start_time_;

            // the number of seconds we want to run the timer for
            float end_time_;

            // a checker to see if the timer has been started
            bool is_active_;

    }; // class Timer

} // namespace game

#endif // TIMER_H_
