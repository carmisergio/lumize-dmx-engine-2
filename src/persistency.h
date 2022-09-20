/*
 * Filename: persistency.h
 * Description: interface for the PersistencyWriter and PersistencyReader class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 20/09/2022
 */

#pragma once

#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

// logger helper
#include "logger.h"

#include "lightstates.h"

/*
 * Definition of the PersistencyWriter class
 */
class PersistencyWriter
{
public:
  // Methods
  bool start();
  void stop();
  void configure(std::string file_path, int interval);
  void set_light_states(LightStates &light_states, std::timed_mutex &light_states_lock);

private:
  std::string file_path;
  int interval;
  bool running = true;          // Used to disconnect gracefully
  std::thread main_loop_thread; // Reference to the connection manager thread
  std::mutex main_loop_mutex;
  std::condition_variable main_loop_cv;

  LightStates *light_states;
  std::timed_mutex *light_states_lock;

  // Internal functions
  void main_loop();
};
