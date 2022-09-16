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
std::vector<std::string> split_string(std::string input, char seperator)
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

bool parse_port_value(LumizeConfig &config, std::string &value_string)
{
  // Check that string contains a number
  if (!isNumber(value_string))
  {
    logger("[CONFIG] Error parsing parameter \"port\": value is not a number!", LOG_ERR, false);
    return false;
  }
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
      string_split = split_string(line, '=');

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
            config.channels = std::stoi(string_split[1]);
          else if (string_split[0] == CONFIG_OPTION_FPS)
            config.fps = std::stoi(string_split[1]);
          else if (string_split[0] == CONFIG_OPTION_DEFAULT_TRANSITION)
            config.default_transition = std::stoi(string_split[1]);
          else if (string_split[0] == CONFIG_OPTION_LOG_DEBUG)
            config.log_debug = string_split[1] == "true" || string_split[1] == "1" || string_split[1] == "yes";
        }
    }

    // Close file
    file.close();
  }
  else
  {
    logger("[CONFIG] Error opening config file!", LOG_ERR, false);
    return false;
  }

  // The logger isn't yet configured properly at this point, so we
  // only call it when debug mode is enabled
  if (config.log_debug)
    recap_config(config);

  return true;
}