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
  LightStates light_states;

  for (int i = 0; i < 512; i++)
    light_states.test[i] = 0;

  DMXSender dmxsender(50); // Initialize DMX Sender with 50 channels

  // dmx_fades_test(dmxsender);

  TCPServer tcpserver(8000);

  tcpserver.set_light_states(light_states);

  tcpserver.start();

  unsigned char dmx_buffer[512];

  while (true)
  {
    dmx_buffer[0] = light_states.test[0];
    dmxsender.send_frame(dmx_buffer);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  dmxsender.stop();

  return 0;
}
