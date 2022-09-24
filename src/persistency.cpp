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
  logger("[PERSISTENCY] Starting writer on file " + file_path, LOG_INFO, true);

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
 * Give access to internal condition variable to be able to notify PersistencyWiter
 * of change of light states
 * Returns: condition variable
 */
std::condition_variable &PersistencyWriter::get_cv()
{
  return main_loop_cv;
}

/*
 ********** PRIVATE FUNCTIONS **********
 */

/*
 * Generates the states string used for persistency
 * Returns: states string
 */
std::string PersistencyWriter::generate_states_string()
{
  std::string string;

  // Start with response message type
  string.append("2.0");

  // Append all statuses
  for (int i = 0; i < 512; i++)
  {
    string.append(",");
    string.append(std::to_string(light_states->outward_state[i]));
    string.append("-");
    string.append(std::to_string(light_states->outward_brightness[i]));
  }

  return string;
}

/*
 * Writes light states to persistency file
 */
void PersistencyWriter::write_persistency_file()
{
  // Generate persistency states string
  std::string states_string = generate_states_string();

  // Create and open a text file
  std::ofstream PersistencyFile(file_path);

  // Write to the file
  if (!(PersistencyFile << states_string))
    logger("[PERSISTENCY] Error writing to persistency file!", LOG_WARN, false);

  // Close the file
  PersistencyFile.close();
}

/*
 * Manages the connection to the FTDI chip with reconnects
 */
void PersistencyWriter::main_loop()
{
  while (running)
  {
    std::unique_lock<std::mutex> lk(main_loop_mutex);

    logger("[PERSISTENCY] Writing persistency file...", LOG_INFO, true);

    // Write persistency file
    write_persistency_file();

    // Wait for interval or next update
    main_loop_cv.wait_for(lk, std::chrono::seconds(interval));
  }
}
