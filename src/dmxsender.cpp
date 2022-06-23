/*
 * Filename: dmxsender.cpp
 * Description: implementation of the DMXSender class 
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 22/06/2022
 */

#include "dmxsender.h" // Include definition of class to be implemented

/*
 *********** CONSTRUCTOR **********
 */
DMXSender::DMXSender(int channels) {

  // Check if number of channels is valid
  if(channels >= 24 || channels <= 512)
    this->channels = channels;

  // If it's not valid use default value
  else this->channels = DEFAULT_CHANNELS;


  // Start the connection manager
  connection_manager_thread = std::thread(&DMXSender::manage_connection, this);
}


/*
 ********** PUBLIC FUNCTIONS **********
 */

/*
 * Sends DMX Frame
 * Parameters:
 *  - char dmx_frame[512]: Array of values for each of the 512 channels 
 *                        in a DMX universe
 */
void DMXSender::send_frame(char dmx_frame[512]) {
  if(can_send)
    std::cout << "Sending DMX Frame..." << std::endl;
  else
    std::cout << "Send_frame called but not connected to FTDI chip!" << std::endl;
}

/*
 * Tells the connection manager to disconnect safely
 * from the FTDI chip
 */
void DMXSender::stop() {
  std::cout << "Stop called!" << std::endl;
  
  // Tell the connection manager to stop
  {
    std::lock_guard<std::mutex> lk(manager_mutex);
    running = false;
  }
  manager_cv.notify_all();

  connection_manager_thread.join(); // Wait for the connection manager to exit
}

/*
 ********** PRIVATE FUNCTIONS **********
 */

/*
 * Manages the connection to the FTDI chip with reconnects
 */
void DMXSender::manage_connection() {
  while(running) {
    std::unique_lock<std::mutex> lk(manager_mutex);
    std::cout << "Managing connection..." << std::endl;
    // std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    manager_cv.wait_for(lk, std::chrono::milliseconds(2000));
  }
  std::cout << "Closing connection" << std::endl;
}

