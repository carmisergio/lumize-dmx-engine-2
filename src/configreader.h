/*
 * Filename: configreader.h
 * Description: Header for the config reader
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 16/09/2022
 */

#pragma once

#include <algorithm>
#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>

#include "./logger.h"

// Configuration file path
#define CONFIG_FILE_PATH "/etc/lumizedmxengine2.conf"

// Default config values
#define DEFAULT_CONFIG_PORT 8056
#define DEFAULT_CONFIG_CHANNELS 25
#define DEFAULT_CONFIG_FPS 50
#define DEFAULT_CONFIG_DEFAULT_TRANSITION 1000 // ms
#define DEFAULT_CONFIG_ENABLE_PERSISTENCY false
#define DEFAULT_CONFIG_PERSISTENCY_FILE_PATH ""
#define DEFAULT_CONFIG_LOG_DEBUG false

// Configuration keys
#define CONFIG_OPTION_PORT "port"
#define CONFIG_OPTION_CHANNELS "channels"
#define CONFIG_OPTION_FPS "fps"
#define CONFIG_OPTION_DEFAULT_TRANSITION "default_transition"
#define CONFIG_OPTION_ENABLE_PERSISTENCY "enable_persistency"
#define CONFIG_OPTION_PERSISTENCY_PERSISTENCY_FILE_PATH "persistency_file_path"
#define CONFIG_OPTION_LOG_DEBUG "log_debug"

// Struct that will hold the config
struct LumizeConfig
{
   int port = DEFAULT_CONFIG_PORT;
   int channels = DEFAULT_CONFIG_CHANNELS;
   int fps = DEFAULT_CONFIG_FPS;
   int default_transition = DEFAULT_CONFIG_DEFAULT_TRANSITION;
   bool enable_persistency = DEFAULT_CONFIG_ENABLE_PERSISTENCY;
   std::string persistency_file_path = DEFAULT_CONFIG_PERSISTENCY_FILE_PATH;
   bool log_debug = DEFAULT_CONFIG_LOG_DEBUG;
};

bool read_config(LumizeConfig &config);