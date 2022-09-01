
/*
 * Filename: logger.cpp
 * Description: implementation of the logger function
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 28/06/2022
 */

#include "logger.h" // Include definition of function to be implemented

void logger(std::string message, int log_level, bool debug_only)
{
  // For each log level use a differnt color
  switch (log_level)
  {
  case 1:
    std::cout << "\u001b[32m" << message << "\u001b[0m" << std::endl;
    break;
  case 2:
    std::cout << "\u001b[33m" << message << "\u001b[0m" << std::endl;
    break;
  case 3:
    std::cout << "\u001b[31m" << message << "\u001b[0m" << std::endl;
    break;
  case 4:
    std::cout << "\u001b[34m" << message << "\u001b[0m" << std::endl;
    break;
  default:
    std::cout << message << std::endl;
    break;
  }
}
