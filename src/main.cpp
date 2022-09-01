/*
 * Filename: main.cpp
 * Description main program
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 22/06/2022
 */

#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <mutex>

#include "tcpserver.h" // TCPServer class
#include "lightrenderer.h"
#include "dmxsender.h"
#include "lightstates.h" // Light statses struct
#include "logger.h"      // Logger class

/*
 * Sets up the light states struct
 * Parameters:
 *  - LightStates &light_states: light states struct
 */
void setup_light_states(LightStates &light_states)
{
  for (int i = 0; i < 512; i++)
  {
    light_states.outward_state[i] = false;
    light_states.outward_brightness[i] = 255;
    light_states.fade_delta[i] = 0;
    light_states.fade_progress[i] = 0;
    light_states.fade_start[i] = 0;
    light_states.fade_end[i] = 0;
  }
}

int main()
{
  TCPServer tcp_server(8000);
  LightRenderer light_renderer;

  // Setup light states structs
  LightStates light_states;
  setup_light_states(light_states);
  std::timed_mutex light_states_lock;

  // Give TCPServer and LightRenderer access to light states struct
  tcp_server.set_light_states(light_states, light_states_lock);
  light_renderer.set_light_states(light_states, light_states_lock);

  // Start LightRenderer
  if (!light_renderer.start())
  {
    return 1;
  }
  // Start the TCPServer
  if (!tcp_server.start())
  {
    light_renderer.stop();
    return 2;
  }

  while (true)
  {
    continue;
  }

  // Stop TCPServer
  tcp_server.stop();

  // Stop DMXSender
  light_renderer.stop();

  return 0;
}
