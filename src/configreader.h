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
#define DEFUALT_CONFIG_LOG_DEBUG false

// Configuration keys
#define CONFIG_OPTION_PORT "port"
#define CONFIG_OPTION_CHANNELS "channels"
#define CONFIG_OPTION_FPS "fps"
#define CONFIG_OPTION_DEFAULT_TRANSITION "default_transition"
#define CONFIG_OPTION_LOG_DEBUG "log_debug"

// Struct that will hold the config
struct LumizeConfig
{
   int port = DEFAULT_CONFIG_PORT;
   int channels = DEFAULT_CONFIG_CHANNELS;
   int fps = DEFAULT_CONFIG_FPS;
   int default_transition = DEFAULT_CONFIG_DEFAULT_TRANSITION;
   int log_debug = DEFUALT_CONFIG_LOG_DEBUG;
};

bool read_config(LumizeConfig &config);