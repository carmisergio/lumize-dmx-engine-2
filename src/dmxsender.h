/*
 * Filename: dmxsender.h
 * Description: interface for the DMXSender class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 22/06/2022
 */

#pragma once

#include <thread>
#include <chrono>
#include <condition_variable>

// libFTDi
#include <libftdi1/ftdi.h>

// logger helper
#include "logger.h"

#define DEFAULT_CHANNELS 24

/*
 * Definition of the DMXSender class
 */
class DMXSender
{
public:
  // Methods
  bool start();
  void send_frame(unsigned char *dmx_frame);
  void stop();
  void configure(int channels = DEFAULT_CHANNELS);

private:
  int channels;                          // Number of channels to output
  struct ftdi_context *ftdi;             // libFTDI FTDI context
  bool running = true;                   // Used to disconnect gracefully
  bool can_send = false;                 // Bool representing current connection
                                         // status to the FTDI chip
  std::thread connection_manager_thread; // Reference to the connection manager thread
  std::mutex manager_mutex;              // Mutex to be used with the condition variable
  std::condition_variable manager_cv;    // Condition variable to stop
                                         // the connection manager from waiting
  const unsigned char start_code = 0;
  // Internal functions
  bool open_ftdi();
  bool close_ftdi();
  bool setup_serial_options();
  bool reconnect();
  bool check_ftdi_connection();
  void manage_connection();
};
