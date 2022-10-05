/*
 * Filename: configreader.cpp
 * Description: implementation of config reader
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 16/09/2022
 */

#include "configreader.h" // Include definition of function to be implemented

/*
 * Removes whitespace from a string
 * Parameters:
 *  - std::string &string: reference to input string
 */
void remove_whitespace(std::string &string)
{
  string.erase(std::remove(string.begin(), string.end(), ' '), string.end());
}

/*
 * Splits a string into a vector of strings based on a separator characters
 * Parameters:
 *  - std::string input: input string
 *  - char separator: separator characters
 * Returns: std:vector<std::string> output vector
 */
std::vector<std::string> configreader_split_string(std::string input, char seperator)
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
 * Transforms bools to human readable strings
 * Parameters:
 *  - bool input: input bool
 * Returns: "true" if true and "false" if false
 */
std::string humanize_bool(bool input)
{
  if (input)
    return "true";
  else
    return "false";
}

/*
 * Prints to console a recap of the config
 * Parameters:
 *  - LumizeConfig &config: reference to the config to recap
 */
void recap_config(LumizeConfig &config)
{
  logger("[CONFIG] Config file read successfully!", LOG_SUCC, false);
  logger("         Port: " + std::to_string(config.port), LOG_INFO, false);
  logger("         Channels: " + std::to_string(config.channels), LOG_INFO, false);
  logger("         FPS: " + std::to_string(config.fps), LOG_INFO, false);
  logger("         Default transition: " + std::to_string(config.default_transition) + "ms", LOG_INFO, false);
  logger("         Enable persistency: " + humanize_bool(config.enable_persistency), LOG_INFO, false);

  if (config.enable_persistency)
  {
    logger("         Persistency file path: " + config.persistency_file_path, LOG_INFO, false);
    logger("         Persistency write interval: " + std::to_string(config.persistency_write_interval), LOG_INFO, false);
  }

  logger("         Debug logging: " + humanize_bool(config.log_debug), LOG_INFO, false);
}

/*
 * Checks if a string is a number
 * Parameters:
 *  - std::string &string: input string
 * Returns: true if string contains only numbers
 */
bool isNumber(std::string &string)
{
  for (char const &character : string)
    if (std::isdigit(character) == 0)
      return false;
  return true;
}

/*
 * Parse "port" config parameter
 * Parameters:
 *  - LumizeConfig &config: config struct to update
 *  - std::string &value_string: reference to input string
 * Returns: true if parameter was correct
 */
bool parse_port_value(LumizeConfig &config, std::string &value_string)
{
  int tmp_port;
  // Check that string contains a number
  if (!isNumber(value_string))
  {
    logger("[CONFIG] Error parsing parameter \"port\": value is not a number!", LOG_ERR, false);
    return false;
  }

  // Convert from string to int
  tmp_port = std::stoi(value_string);

  if (tmp_port < 1000)
  {
    logger("[CONFIG] Error parsing parameter \"port\": Value must be greater than 1000!", LOG_ERR, false);
    return false;
  }

  // Set config parameter
  config.port = tmp_port;

  return true;
}

/*
 * Parse "channels" config parameter
 * Parameters:
 *  - LumizeConfig &config: config struct to update
 *  - std::string &value_string: reference to input string
 * Returns: true if parameter was correct
 */
bool parse_channels_value(LumizeConfig &config, std::string &value_string)
{
  int tmp_channels;

  // Check that string contains a number
  if (!isNumber(value_string))
  {
    logger("[CONFIG] Error parsing parameter \"channels\": value is not a number!", LOG_ERR, false);
    return false;
  }

  // Convert from string to int
  tmp_channels = std::stoi(value_string);

  if (tmp_channels < 1 || tmp_channels > 512)
  {
    logger("[CONFIG] Error parsing parameter \"channels\": Value must be between 1 and 512!", LOG_ERR, false);
    return false;
  }

  // Set config parameter
  config.channels = tmp_channels;

  return true;
}

/*
 * Parse "fps" config parameter
 * Parameters:
 *  - LumizeConfig &config: config struct to update
 *  - std::string &value_string: reference to input string
 * Returns: true if parameter was correct
 */
bool parse_fps_value(LumizeConfig &config, std::string &value_string)
{
  int tmp_fps;

  // Check that string contains a number
  if (!isNumber(value_string))
  {
    logger("[CONFIG] Error parsing parameter \"fps\": value is not a number!", LOG_ERR, false);
    return false;
  }

  // Convert from string to int
  tmp_fps = std::stoi(value_string);

  if (tmp_fps < 10 || tmp_fps > 200)
  {
    logger("[CONFIG] Error parsing parameter \"fps\": Value must be between 10 and 200!", LOG_ERR, false);
    return false;
  }

  // Set config parameter
  config.fps = tmp_fps;

  return true;
}

/*
 * Parse "default_transition" config parameter
 * Parameters:
 *  - LumizeConfig &config: config struct to update
 *  - std::string &value_string: reference to input string
 * Returns: true if parameter was correct
 */
bool parse_default_transition_value(LumizeConfig &config, std::string &value_string)
{
  int tmp_default_transition;

  // Check that string contains a number
  if (!isNumber(value_string))
  {
    logger("[CONFIG] Error parsing parameter \"default_transition\": value is not a number!", LOG_ERR, false);
    return false;
  }

  // Convert from string to int
  tmp_default_transition = std::stoi(value_string);

  if (tmp_default_transition < 0)
  {
    logger("[CONFIG] Error parsing parameter \"fps\": Value must be between greater than or equals to 0!", LOG_ERR, false);
    return false;
  }

  // Set config parameter
  config.default_transition = tmp_default_transition;

  return true;
}

/*
 * Parse "enable_persistency" config parameter
 * Parameters:
 *  - LumizeConfig &config: config struct to update
 *  - std::string &value_string: reference to input string
 * Returns: true if parameter was correct
 */
bool parse_enable_persistency_value(LumizeConfig &config, std::string &value_string)
{
  bool tmp_enable_persistency;

  // Differentiate between values
  if (value_string == "true" || value_string == "yes" || value_string == "on" || value_string == "1")
  {
    tmp_enable_persistency = true;
  }
  else if (value_string == "false" || value_string == "no" || value_string == "off" || value_string == "0")
  {
    tmp_enable_persistency = false;
  }
  else
  {
    // Value was not valid
    logger("[CONFIG] Error parsing parameter \"enable_persistency\": value is not a valid boolean!", LOG_ERR, false);
    return false;
  }

  // Set config parameter
  config.enable_persistency = tmp_enable_persistency;

  return true;
}

/*
 * Parse "persistency_file_path" config parameter
 * Parameters:
 *  - LumizeConfig &config: config struct to update
 *  - std::string &value_string: reference to input string
 * Returns: true if parameter was correct
 */
bool parse_persistency_file_path_value(LumizeConfig &config, std::string &value_string)
{
  if (value_string == "")
  {
    logger("[CONFIG] Error parsing parameter \"persistency_file_path\": value cannot be empty!", LOG_ERR, false);
    return false;
  }

  // Set config parameter
  config.persistency_file_path = value_string;

  return true;
}

/*
 * Parse "persistency_write_interval" config parameter
 * Parameters:
 *  - LumizeConfig &config: config struct to update
 *  - std::string &value_string: reference to input string
 * Returns: true if parameter was correct
 */
bool parse_persistency_write_interval_value(LumizeConfig &config, std::string &value_string)
{
  int tmp_persistency_write_interval;

  // Check that string contains a number
  if (!isNumber(value_string))
  {
    logger("[CONFIG] Error parsing parameter \"persistency_write_interval\": value is not a number!", LOG_ERR, false);
    return false;
  }

  // Convert from string to int
  tmp_persistency_write_interval = std::stoi(value_string);

  if (tmp_persistency_write_interval <= 0)
  {
    logger("[CONFIG] Error parsing parameter \"fps\": Value must be greater than 0!", LOG_ERR, false);
    return false;
  }

  // Set config parameter
  config.persistency_write_interval = tmp_persistency_write_interval;

  return true;
}

/*
 * Parse "log_debug" config parameter
 * Parameters:
 *  - LumizeConfig &config: config struct to update
 *  - std::string &value_string: reference to input string
 * Returns: true if parameter was correct
 */
bool parse_log_debug_value(LumizeConfig &config, std::string &value_string)
{
  bool tmp_log_debug;

  // Differentiate between values
  if (value_string == "true" || value_string == "yes" || value_string == "on" || value_string == "1")
  {
    tmp_log_debug = true;
  }
  else if (value_string == "false" || value_string == "no" || value_string == "off" || value_string == "0")
  {
    tmp_log_debug = false;
  }
  else
  {
    // Value was not valid
    logger("[CONFIG] Error parsing parameter \"log_deubg\": value is not a valid boolean!", LOG_ERR, false);
    return false;
  }

  // Set config parameter
  config.log_debug = tmp_log_debug;

  return true;
}

/*
 * Reads from configuration file and saves the parameters in LumizeConfig struct
 * Parameters:
 *  - LumizeConfig &config: reference to the LumizeConfig struct to save into
 * Returns: true if succesful
 */
bool read_config(LumizeConfig &config)
{
  std::string line;
  std::vector<std::string> string_split;
  std::ifstream file(CONFIG_FILE_PATH);

  // Check that file opened succesfully
  if (file.is_open())
  {
    while (getline(file, line))
    {
      remove_whitespace(line);

      // Check if line is comment
      if (line[0] == '#')
        continue;

      // Split string by = character
      string_split = configreader_split_string(line, '=');

      // Check that the message is of right format
      if (string_split.size() > 1)
        // Iterate over messages
        for (size_t i = 0; i < string_split.size(); i++)
        {
          // Read messages
          if (string_split[0] == CONFIG_OPTION_PORT)
          {
            if (!parse_port_value(config, string_split[1]))
              return false;
          }
          else if (string_split[0] == CONFIG_OPTION_CHANNELS)
          {
            if (!parse_channels_value(config, string_split[1]))
              return false;
          }
          else if (string_split[0] == CONFIG_OPTION_FPS)
          {
            if (!parse_fps_value(config, string_split[1]))
              return false;
          }
          else if (string_split[0] == CONFIG_OPTION_DEFAULT_TRANSITION)
          {
            if (!parse_default_transition_value(config, string_split[1]))
              return false;
          }
          else if (string_split[0] == CONFIG_OPTION_ENABLE_PERSISTENCY)
          {
            if (!parse_enable_persistency_value(config, string_split[1]))
              return false;
          }
          else if (string_split[0] == CONFIG_OPTION_PERSISTENCY_FILE_PATH)
          {
            if (!parse_persistency_file_path_value(config, string_split[1]))
              return false;
          }
          else if (string_split[0] == CONFIG_OPTION_PERSISTENCY_WRITE_INTERVAL)
          {
            if (!parse_persistency_write_interval_value(config, string_split[1]))
              return false;
          }
          else if (string_split[0] == CONFIG_OPTION_LOG_DEBUG)
          {
            if (!parse_log_debug_value(config, string_split[1]))
              return false;
          }
        }
    }

    // Close file
    file.close();
  }
  else
  {
    logger("[CONFIG] Error opening config file: " + std::string(CONFIG_FILE_PATH), LOG_ERR, false);
    return false;
  }

  // The logger isn't yet configured properly at this point, so we
  // only call it when debug mode is enabled
  if (config.log_debug)
    recap_config(config);

  return true;
}