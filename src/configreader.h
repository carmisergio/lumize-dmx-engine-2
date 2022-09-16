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

#define CONFIG_FILE_PATH "./lumizedmxengine2.conf"

#define CONFIG_OPTION_PORT "port"
#define CONFIG_OPTION_CHANNELS "channels"
#define CONFIG_OPTION_FPS "fps"
#define CONFIG_OPTION_DEFAULT_TRANSITION "default_transition"
#define CONFIG_OPTION_LOG_DEBUG "log_debug"

// Struct that will hold the config
struct LumizeConfig
{
   int port = 8056;
   int fps = 100;
   int default_transition = 750;
   int channels = 50;
   int log_debug = false;
};

bool read_config(LumizeConfig &config);