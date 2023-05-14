/*
 * Filename: dmxsender.cpp
 * Description: implementation of the DMXSender class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 22/06/2022
 */

#include "dmxsender.h" // Include definition of class to be implemented

/*
 ********** PUBLIC FUNCTIONS **********
 */

/*
 * Startup module
 */
bool DMXSender::start()
{
  logger("[DMX] Initializing with " + std::to_string(channels) + " channels...", LOG_INFO, true);

  // Instantiate FTDI library
  if ((ftdi = ftdi_new()) == 0)
  {
    logger("[DMX] Error instantiating ftdi library!", LOG_ERR, false);
    return false;
  }

  // Start the connection manager
  connection_manager_thread = std::thread(&DMXSender::manage_connection, this);

  return true;
}

/*
 * Sends DMX Frame
 * Parameters:
 *  - unsigned char *dmx_frame: Array of values for each of the 512 channels
 *                        in a DMX universe
 */
void DMXSender::send_frame(unsigned char *dmx_frame)
{
  if (can_send)
  {
    if (
        ftdi_set_line_property2(ftdi, BITS_8, STOP_BIT_2, NONE, BREAK_ON) < 0 ||
        ftdi_set_line_property2(ftdi, BITS_8, STOP_BIT_2, NONE, BREAK_OFF) < 0 ||
        ftdi_write_data(ftdi, &start_code, 1) < 0 ||
        ftdi_write_data(ftdi, dmx_frame, channels) < 0)
    {
      logger("[DMX] Error sending DMX frame! Not ready to send!", LOG_WARN);
      // Tell the connection manager to stop waiting
      {
        std::lock_guard<std::mutex> lk(manager_mutex);
        can_send = false;
      }
      manager_cv.notify_all();
    }
  }
}

/*
 * Tells the connection manager to disconnect safely
 * from the FTDI chip
 */
void DMXSender::stop()
{
  // Tell the connection manager to stop waiting
  {
    std::lock_guard<std::mutex> lk(manager_mutex);
    running = false;
  }
  manager_cv.notify_all();

  connection_manager_thread.join(); // Wait for the connection manager to exit
}

/*
 * Sets up configuration for the DMXSender
 * Parameters:
 *  - int channels: Amount of channels to include in the DMX frame
 */
void DMXSender::configure(int channels)
{
  // Check if number of channels is valid
  if (channels >= 24 || channels <= 512)
    this->channels = channels;

  // If it's not valid use default value
  else
    this->channels = DEFAULT_CHANNELS;
}

/*
 ********** PRIVATE FUNCTIONS **********
 */

/*
 * Opens a USB connection to the FTDI device
 */
bool DMXSender::open_ftdi()
{
  // Open FTDI device
  int return_code;
  return_code = ftdi_usb_open(ftdi, 0x0403, 0x6001);
  if (return_code < 0)
  {
    return false;
  }
  return true;
}

/*
 * Closes a USB connection to the FTDI device
 */
bool DMXSender::close_ftdi()
{
  // Close FTDI device
  if (ftdi_usb_close(ftdi) < 0)
  {
    return false;
  }
  return true;
}

/*
 * Sets up the serial chip for sending DMX data
 */
bool DMXSender::setup_serial_options()
{
  // Set baudrate
  if (ftdi_set_baudrate(ftdi, 250000) < 0)
    return false;

  // Set serial properties to be correct for DMX
  if (ftdi_set_line_property2(ftdi, BITS_8, STOP_BIT_2, NONE, BREAK_ON), 0)
    return false;

  return true;
}

/*
 * Handles enstablishing and veryfiying the connection to an FTDI device
 */
bool DMXSender::reconnect()
{
  close_ftdi(); // Make sure we don't already have something connected

  // Open FTDI device
  if (!open_ftdi())
    return false;

  // Setup serial connection
  if (!setup_serial_options())
    return false;

  return true;
}

/*
 * Checks connection to the FTDI chip
 */
bool DMXSender::check_ftdi_connection()
{
  unsigned int chipid;
  if (ftdi_read_chipid(ftdi, &chipid) < 0)
    return false;
  return true;
}

/*
 * Manages the connection to the FTDI chip with reconnects
 */
void DMXSender::manage_connection()
{
  while (running)
  {
    std::unique_lock<std::mutex> lk(manager_mutex);

    // If we think we are connected, check the connection
    if (can_send)
      if (!check_ftdi_connection())
        can_send = false;

    // If not, try to connect
    if (!can_send)
    {
      if (reconnect())
      {
        can_send = true;
        logger("[DMX] USB connection to FTDI chip enstablished. Ready to send!", LOG_SUCC);
      }
      else
        logger("[DMX] Unable to connect to FTDI device, retrying in a few seconds...", LOG_WARN);
    }

    manager_cv.wait_for(lk, std::chrono::milliseconds(2000));
  }

  // Close connection to FTDI chip
  close_ftdi();

  // Free FTDI context
  ftdi_free(ftdi);
}
