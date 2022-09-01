/*
 * Filename: lightrenderer.cpp
 * Description: implementation of the LighrRenderer class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 01/09/2022
 */

#include "lightrenderer.h" // Include definition of class to be implemented

#define DEFAULT_TRANSITION 1000
#define FPS 100

/*
 *********** CONSTRUCTOR **********
 */
LightRenderer::LightRenderer() : dmx_sender(50)
{
   total_wait = 1000 / FPS;
}

/*
 ********** PUBLIC FUNCTIONS **********
 */

/*
 * Startup module
 */
bool LightRenderer::start()
{
   logger("[LIGHT] Starting output to lights...", LOG_INFO, false);

   if (!dmx_sender.start())
   {
      return false;
   }

   rendering_thread = std::thread(&LightRenderer::main_loop, this);

   logger("[LIGHT] Light output started!", LOG_SUCC, false);
   return true;
}

/*
 * Stops the rendering loop and the DMX Sender
 */
void LightRenderer::stop()
{
   // Stop rendering thread
   running = false;
   rendering_thread.join();

   // Stop DMX Sender
   dmx_sender.stop();
}

/*
 * Give LightRenderer access to LightStates struct
 * Parameters:
 *  - LightStates &light_states: reference to light states struct
 */
void LightRenderer::set_light_states(LightStates &light_states, std::timed_mutex &light_states_lock)
{
   this->light_states = &light_states;
   this->light_states_lock = &light_states_lock;
}

/*
 ********** PRIVATE FUNCTIONS **********
 */

void LightRenderer::main_loop()
{
   while (running)
   {
      // Get render start time
      render_begin_time = std::chrono::steady_clock::now();

      // std::cout << light_states->fade_progress[0] << std::endl;

      // Acquire lock on light states
      if (light_states_lock->try_lock_for(std::chrono::milliseconds(5)))
      {
         // If we were able to acquire the lock, compute new frame
         for (int i = 0; i < 512; i++)
         {
            if (light_states->fade_delta[i] == 0)
               dmx_frame[i] = light_states->fade_end[i];
            else
            {
               dmx_frame[i] = (double)(light_states->fade_end[i] - light_states->fade_start[i]) * ease_in_out_sine(light_states->fade_progress[i]) + light_states->fade_start[i];
               light_states->fade_progress[i] += light_states->fade_delta[i];

               // If fade is finished
               if (light_states->fade_progress[i] > 1)
               {
                  light_states->fade_delta[i] = 0;
                  light_states->fade_progress[i] = 0;
               }
            }
         }
      };

      // Free lock
      light_states_lock->unlock();

      std::cout << (int)dmx_frame[0] << std::endl;

      // Send dmx frame
      dmx_sender.send_frame(dmx_frame);

      // Get render end time
      render_end_time = std::chrono::steady_clock::now();
      wait_time = total_wait - std::chrono::duration_cast<std::chrono::milliseconds>(render_end_time - render_begin_time).count();
      // std::cout << wait_time << std::endl;
      // Wait correct amount of time
      std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
   }
}

double LightRenderer::ease_in_out_sine(double t)
{
   return 0.5 * (1 + sin(3.1415926 * (t - 0.5)));
}