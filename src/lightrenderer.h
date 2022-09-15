
/*
 * Filename: lightrenderer.h
 * Description: interface for the LightRenderer class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 01/09/2022
 */

#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <mutex>

// DMX output
#include "dmxsender.h"

#include "lightstates.h"

// logger helper
#include "logger.h"

#define DEFAULT_CHANNELS 24

/*
 * Definition of the LightRenderer class
 */
class LightRenderer
{
public:
   // Methods
   bool start();
   void stop();
   void set_light_states(LightStates &light_states, std::timed_mutex &light_states_lock);
   void configure(int fps, int channels);

private:
   DMXSender dmx_sender;
   LightStates *light_states;
   std::timed_mutex *light_states_lock;
   unsigned char dmx_frame[512]; // DMX frame to be sent
   bool running = true;
   std::thread rendering_thread;
   int total_wait;
   std::chrono::steady_clock::time_point render_begin_time, render_end_time;
   int wait_time;

   // Config
   int fps, channels;

   // Internal functions
   void main_loop();

   // Easing functions
   double ease_in_out_sine(double t);
};
