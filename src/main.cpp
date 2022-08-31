/*
 * Filename: main.cpp
 * Description main program
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 22/06/2022
 */

#include <iostream>
#include <unistd.h>
#include <thread>
#include <chrono>

#include "dmxsender.h"   // DMXSender class
#include "tcpserver.h"   // TCPServer class
#include "lightstates.h" // Light statses struct

/*
 * Sets up the light states struct
 * Parameters:
 *  - LightStates &light_states: light states struct
 */
void setup_light_states(LightStates &light_states)
{
  for (int i = 0; i < 512; i++)
  {
    light_states.outward_state[i] = false;
    light_states.outward_brightness[i] = 255;
  }
}

/*
 * Test fades
 */
void dmx_fades_test(DMXSender &dmxsender)
{
  unsigned char dmx_buffer[512];
  float i;
  while (true)
  {
    for (i = 0; i <= 255; i += 2.5)
    {
      dmx_buffer[0] = i;
      dmxsender.send_frame(dmx_buffer);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // sleep(1);
    for (i = 255; i >= 0; i -= 2.5)
    {
      dmx_buffer[0] = i;
      dmxsender.send_frame(dmx_buffer);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    // sleep(1);
  }
}

int main()
{
  // Setup light states structs
  LightStates light_states;
  setup_light_states(light_states);

  for (int i = 0; i < 512; i++)
    light_states.test[i] = 0;

  // Initialize DMXSender
  DMXSender dmxsender(50);

  // Initialize TCPServer
  TCPServer tcpserver(8000);
  // Give TCPServer access to light states struct
  tcpserver.set_light_states(light_states);
  // Start the TCPServer
  tcpserver.start();

  unsigned char dmx_buffer[512];
  while (true)
  {
    dmx_buffer[0] = light_states.test[0];
    dmxsender.send_frame(dmx_buffer);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Stop DMXSender
  dmxsender.stop();

  return 0;
}
