/*
 * Filename: dmxsender.h
 * Description: interface for the DMXSender class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 22/06/2022
 */

#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <condition_variable>

// libFTDi
#include <libftdi1/ftdi.h>

#define DEFAULT_CHANNELS 24

/*
 * Definition of the DMXSender class
 */
class DMXSender {
  public:
    // Constructor
    DMXSender(int channels = DEFAULT_CHANNELS);

    // Methods
    void send_frame(char dmx_frame[512]);
    void stop();

  private:
    int channels; // Number of channels to output
    struct ftdi_context ftdi; // libFTDI FTDI context
    bool running = true; // Used to disconnect gracefully
    bool can_send = false; // Bool representing current connection
                           // status to the FTDI chip
    std::thread connection_manager_thread; // Reference to the connection manager thread
    std::mutex manager_mutex; // Mutex to be used with the condition variable
    std::condition_variable manager_cv; // Condition variable to stop
                                        // the connection manager from waiting
    // Internal functions
    void manage_connection();
};

