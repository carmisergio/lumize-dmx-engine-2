/*
 * Filename: persistency.cpp
 * Description: implementation of the PersistencyWriter and PersistencyReader class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 20/09/2022
 */

#include "persistency.h" // Include definition of class to be implemented

/*
 ********** PUBLIC FUNCTIONS **********
 */

/*
 * Startup module
 */
bool PersistencyWriter::start()
{
  logger("[PERSISTENCY] Initializing writer with ", LOG_INFO, true);

  // Start the connection manager
  main_loop_thread = std::thread(&PersistencyWriter::main_loop, this);

  return true;
}

/*
 * Stops the main loop
 */
void PersistencyWriter::stop()
{
  // Tell the main loop
  {
    std::lock_guard<std::mutex> lk(main_loop_mutex);
    running = false;
  }
  main_loop_cv.notify_all();

  main_loop_thread.join(); // Wait for the connection manager to exit
}

/*
 * Give PersistencyWriter access to LightStates struct
 * Parameters:
 *  - LightStates &light_states: reference to light states struct
 */
void PersistencyWriter::set_light_states(LightStates &light_states, std::timed_mutex &light_states_lock)
{
  this->light_states = &light_states;
  this->light_states_lock = &light_states_lock;
}

/*
 * Sets up configuration for the PersistencyWriter
 * Parameters:
 *  - int channels: Amount of channels to include in the DMX frame
 */
void PersistencyWriter::configure(std::string file_path, int interval)
{
  this->file_path = file_path;
  this->interval = interval;
}

/*
 ********** PRIVATE FUNCTIONS **********
 */

/*
 * Manages the connection to the FTDI chip with reconnects
 */
void PersistencyWriter::main_loop()
{
  while (running)
  {
    std::unique_lock<std::mutex> lk(main_loop_mutex);

    std::cout << "Checking persistency updates" << std::endl;

    main_loop_cv.wait_for(lk, std::chrono::milliseconds(interval));
  }
}
