/*
 * Filename: main.cpp
 * Description main program
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 22/06/2022
 */

#include <iostream>

#include "dmxsender.h" // DMXSender class

int main() {
  DMXSender dmxsender(50); // Initialize DMX Sender with 50 channels

  for(int i = 0; i < 20; i++) {
    std::cout << "Doing lighting things..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  dmxsender.stop();

  return 0;
}
