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

/*
 * Splits a string into a vector of strings based on a separator characters
 * Parameters:
 *  - std::string input: input string
 *  - char separator: separator characters
 * Returns: std:vector<std::string> output vector
 */
std::vector<std::string> persistency_split_string(std::string input, char seperator)
{
  std::string section;
  std::stringstream stream(input);
  std::vector<std::string> output;

  // Get separated parts from string stream
  while (std::getline(stream, section, seperator))
    output.push_back(section);

  return output;
}

/*
 * Actually parse states string and save it into the tmp_light_states
 * Parameters:
 *  - std::string file_path: persistency file path
 *  - LightStates &tmp_light_states: reference to temporary light states struct
 * Returns: true if succesful
 */
bool parse_states_string(std::string states_string, LightStates &light_states, std::timed_mutex &light_states_lock)
{
  // Split states string by ','
  std::vector<std::string> string_split = persistency_split_string(states_string, ',');
  std::vector<std::string> command_split;
  int tmp_brightness;

  // Check that persistency file version string matches
  if (!(string_split[0] == PERSISTENCY_FILE_VERSION_STRING))
  {
    logger("[PERSISTENCY] Persistency file was written by different version of Lumize DMX Engine! Skipping...", LOG_WARN, false);
    return false;
  }

  light_states_lock.lock();
  for (int i = 0; i < 512; i++)
  {
    // i + 1 because the persistency file version string has to be ignored
    command_split = persistency_split_string(string_split[i + 1], '-');

    // Parse states
    if (command_split[0] == "1")
      light_states.outward_state[i] = true;
    else if (command_split[0] == "0")
      light_states.outward_state[i] = false;
    else
    {
      // If value is corrupted, skip
      logger("[PERSISTENCY] Bad state value in persistency file at channel " + std::to_string(i), LOG_WARN, true);
    }

    // Parse brightness
    try
    {
      tmp_brightness = std::stoi(command_split[1]);

      // Check that brightness value is acceptable
      if (tmp_brightness < 0 || tmp_brightness > 511)
      {
        logger("[PERSISTENCY] Brightness value out of range in perstency file at channel " + std::to_string(i), LOG_WARN, true);
        continue;
      }

      // Copy value to temporary light states
      light_states.outward_brightness[i] = tmp_brightness;
    }
    catch (const std::exception &e)
    {
      logger("[PERSISTENCY] Bad brightness value in persistency file at channel " + std::to_string(i), LOG_WARN, true);
    }

    // Calculate current brightness
    light_states.fade_current[i] = light_states.outward_state[i] ? light_states.outward_brightness[i] : 0;
  }
  light_states_lock.unlock();

  return true;
}

/*
 * Read data from persistency file and store it in light states struct
 * Parameters:
 *  - std::string file_path: persistency file path
 *  - LightStates &light_states: reference to light states struct
 *  - std::timed_mutex &light_states_lock reference to light states struct mutex
 * Returns: true if succesful
 */
bool read_persistency_file(std::string file_path, LightStates &light_states, std::timed_mutex &light_states_lock)
{
  logger("[PERSISTENCY] Reading persistency file: " + file_path + "...", LOG_INFO, true);

  std::string states_string;

  // Open file as read only
  std::ifstream read_file(file_path);

  // Read first line from file (it's all we need)
  if (!getline(read_file, states_string))
  {
    logger("[PERSISTENCY] Unable to read persistency file");
    read_file.close();
    return false;
  }

  // Close the file
  read_file.close();

  // Parse states string
  if (!parse_states_string(states_string, light_states, light_states_lock))
    return false;

  logger("[PERSISTENCY] Succesfully read persistency file!", LOG_SUCC, true);
  return true;
}
