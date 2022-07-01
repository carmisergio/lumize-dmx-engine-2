/*
 * Filename: logger.h
 * Description: simple logging helper
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 23/06/2022
 */

#pragma once

#include <iostream>
#include <string>

// Types of log levels
#define LOG_INFO 0
#define LOG_SUCC 1
#define LOG_WARN 2
#define LOG_ERR 3

/*
 * Logs some information
 * Parameters:
 * - std::string message: message to be logged
 * - int log_color: color of the message by type of log
 * - bool debug_only: this message should only be logged if debug log is enabled
 */
void logger(std::string message, int log_level = LOG_INFO, bool debug_only = false);
