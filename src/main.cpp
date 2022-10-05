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
#include <condition_variable>

#include "tcpserver.h" // TCPServer class
#include "lightrenderer.h"
#include "dmxsender.h"
#include "lightstates.h"  // Light statses struct
#include "logger.h"       // Logger class
#include "configreader.h" // Config reader
#include "persistency.h"  // Persistency writer and reader

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
    light_states.fade_current[i] = 0;
  }
}

int main()
{
  std::cout << "##### Lumize DMX Engine 2 #####" << std::endl;
  std::cout << "Starting..." << std::endl;

  TCPServer tcp_server;
  LightRenderer light_renderer;
  LumizeConfig config;
  PersistencyWriter persistency_writer;

  // Setup light states structs
  LightStates light_states;
  setup_light_states(light_states);
  std::timed_mutex light_states_lock;

  // Read config
  if (!read_config(config))
  {
    return 1;
  }

  // Initialize condition variable for PersistencyWriter
  std::condition_variable &persistency_writer_cv = persistency_writer.get_cv();

  // Give TCPServer and LightRenderer access to light states struct
  tcp_server.set_light_states(light_states, light_states_lock);
  light_renderer.set_light_states(light_states, light_states_lock);
  persistency_writer.set_light_states(light_states, light_states_lock);

  // Configure TCPServer and LightRenderer
  tcp_server.configure(config.port, config.fps, config.default_transition);
  light_renderer.configure(config.fps, config.channels);
  persistency_writer.configure(config.persistency_file_path, config.persistency_write_interval);
  set_enable_debug(config.log_debug);

  // Give access to persistency_writer_cv
  tcp_server.set_persistency_writer_cv(persistency_writer_cv);

  // Read persistency file
  if (config.enable_persistency)
    read_persistency_file(config.persistency_file_path, light_states, light_states_lock);

  // Start LightRenderer
  if (!light_renderer.start())
  {
    return 2;
  }

  // Start the TCPServer
  if (!tcp_server.start())
  {
    light_renderer.stop();
    return 3;
  }

  // Start the PersistencyWriter if it's enabled
  if (config.enable_persistency)
  {

    if (!persistency_writer.start())
    {
      light_renderer.stop();
      tcp_server.stop();
      return 4;
    }
  }

  // Keep program running
  while (true)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // Stop TCPServer
  tcp_server.stop();

  // Stop DMXSender
  light_renderer.stop();

  return 0;
}
