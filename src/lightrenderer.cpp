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
 *  - BrightnessLimits brightness_limits: brightness limits of all lights
 *  - int channels: Amount of channels to output
 */
void LightRenderer::configure(int fps, int channels, std::array<BrightnessLimits, 512> *brightness_limits, int pushbutton_fade_delta, int pushbutton_fade_pause)
{
   this->fps = fps;
   this->channels = channels;
   this->pushbutton_fade_delta_divided = (double)pushbutton_fade_delta / fps;
   this->pushbutton_fade_pause_frames = pushbutton_fade_pause * fps / 1000;
   this->brightness_limits = brightness_limits;

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
            double computation_value;

            // No fade active
            if (light_states->fade_delta[i] == 0)
               computation_value = light_states->fade_current[i];
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
                  computation_value = light_states->fade_current[i];
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
                  // Pause stuff
                  if (light_states->pushbutton_fade_pause_counter[i] < pushbutton_fade_pause_frames)
                     light_states->pushbutton_fade_pause_counter[i]++;
                  else
                  {
                     // Invert direction
                     light_states->pushbutton_fade_up[i] = false;

                     // Reset counter
                     light_states->pushbutton_fade_pause_counter[i] = 0;
                  }

                  // Clean up value
                  light_states->pushbutton_fade_current[i] = 255;
               }
               else if (light_states->pushbutton_fade_current[i] <= 0)
               {
                  light_states->pushbutton_fade_up[i] = true;
                  // Invert direction
                  light_states->pushbutton_fade_current[i] = 0;
               }

               // Save new value into DMX frame
               computation_value = light_states->pushbutton_fade_current[i];
            }

            // Save computed value into dmx frame
            dmx_frame[i] = map_brightness_limits(computation_value, brightness_limits->at(i));
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

/*
 * Interpolates between with sine exponentiation
 * Parameters:
 *  - double t: input
 * Returns: sine interpolation
 */
double LightRenderer::ease_in_out_sine(double t)
{
   return 0.5 * (1 + sin(3.1415926 * (t - 0.5)));
}

/*
 * Maps brightness between 0 and 255 to a brightness within brightness limits
 * Parameters:
 *  - int value: value to map
 *  - BrightnessLimits brightness_limits: brightness limits to use for the map
 * Returns: mapped value
 */
unsigned char LightRenderer::map_brightness_limits(double value, BrightnessLimits brightness_limits)
{
   if (value < 1)
      return 0;

   if (value > 254)
      return 255;

   // Compute mapped value
   return value * (brightness_limits.max - brightness_limits.min) / 255 + brightness_limits.min;
}