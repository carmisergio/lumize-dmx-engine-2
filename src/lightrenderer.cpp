/*
 * Filename: lightrenderer.cpp
 * Description: implementation of the LighrRenderer class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 01/09/2022
 */

#include "lightrenderer.h" // Include definition of class to be implemented

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

   logger("[LIGHT] Light output started at " + std::to_string(fps) + " FPS!", LOG_SUCC, false);
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
 * Configure the light renderer
 * Parameters:
 *  - int fps: FPS to render at
 *  - int channels: Amount of channels to output
 */
void LightRenderer::configure(int fps, int channels, int pushbutton_fade_delta)
{
   this->fps = fps;
   this->channels = channels;
   this->pushbutton_fade_delta_divided = (double)pushbutton_fade_delta / fps;

   // Configure DMXSender
   dmx_sender.configure(channels);
}

/*
 ********** PRIVATE FUNCTIONS **********
 */

void LightRenderer::main_loop()
{
   // Calculate total wait time between each frame
   total_wait = 1000 / fps;

   while (running)
   {
      // Get render start time
      render_begin_time = std::chrono::steady_clock::now();

      // Acquire lock on light states
      if (light_states_lock->try_lock_for(std::chrono::milliseconds(5)))
      {
         // If we were able to acquire the lock, compute new frame
         for (int i = 0; i < 512; i++)
         {
            // No fade active
            if (light_states->fade_delta[i] == 0)
               dmx_frame[i] = light_states->fade_current[i];
            else
            {
               // Increment fade progress
               light_states->fade_progress[i] += light_states->fade_delta[i];

               // If fade is finished
               if (light_states->fade_progress[i] > 1)
               {
                  light_states->fade_delta[i] = 0;
                  light_states->fade_progress[i] = 0;
                  light_states->fade_current[i] = light_states->fade_end[i];
                  logger("[LIGHT] Fade finished, channel: " + std::to_string(i), LOG_INFO, true);
               }
               else
               {
                  // Compute light value
                  light_states->fade_current[i] = (double)(light_states->fade_end[i] - light_states->fade_start[i]) * ease_in_out_sine(light_states->fade_progress[i]) + light_states->fade_start[i];
                  dmx_frame[i] = light_states->fade_current[i];
               }
            }

            // There is a pushbutton fade active
            if (light_states->pushbutton_fade[i])
            {

               // Calculate new value
               if (light_states->pushbutton_fade_up[i])
                  light_states->pushbutton_fade_current[i] += pushbutton_fade_delta_divided;
               else
                  light_states->pushbutton_fade_current[i] -= pushbutton_fade_delta_divided;

               // Check limits
               if (light_states->pushbutton_fade_current[i] >= 255)
               {
                  light_states->pushbutton_fade_current[i] = 255;
                  // Invert direction
                  light_states->pushbutton_fade_up[i] = false;
               }
               else if (light_states->pushbutton_fade_current[i] <= 0)
               {
                  light_states->pushbutton_fade_up[i] = true;
                  // Invert direction
                  light_states->pushbutton_fade_current[i] = 0;
               }

               // Save new value into DMX frame
               dmx_frame[i] = light_states->pushbutton_fade_current[i];
            }
         }

         // std::cout << (int)dmx_frame[0] << std::endl;
      };

      // Free lock
      light_states_lock->unlock();

      // Send dmx frame
      dmx_sender.send_frame(dmx_frame);

      // Get render end time
      render_end_time = std::chrono::steady_clock::now();
      wait_time = total_wait - std::chrono::duration_cast<std::chrono::milliseconds>(render_end_time - render_begin_time).count();

      // Wait correct amount of time
      std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
   }
}

double LightRenderer::ease_in_out_sine(double t)
{
   return 0.5 * (1 + sin(3.1415926 * (t - 0.5)));
}