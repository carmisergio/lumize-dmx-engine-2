/*
 * Filename: tcpserver.cpp
 * Description: implementation of the TcpServer class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 30/08/2022
 */

#include "tcpserver.h" // Include definition of class to be implemented

/*
 *********** CONSTRUCTOR **********
 */
TCPServer::TCPServer()
{
   init_client_sockets_array();
}

/*
 ********** PUBLIC FUNCTIONS **********
 */

/*
 * Starts the TCP server
 * Returns: true if succesful
 */
bool TCPServer::start()
{
   int opt = 1;

   logger("[TCP] Starting server...", LOG_INFO, true);

   // Create master socket
   if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      logger("[TCP] Error on socket() system call!", LOG_ERR, false);
      return false;
   }

   // Allow multiple incoming connections
   if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
                  (char *)&opt, sizeof(opt)) < 0)
   {
      logger("[TCP] Error setting socket options!", LOG_ERR, false);
      return false;
   }

   // Type of address to bind to
   address.sin_family = AF_INET;
   address.sin_addr.s_addr = INADDR_ANY;
   address.sin_port = htons(port);

   // Bind master socket to localhost and correct port
   if (bind(master_socket, (struct sockaddr *)&address,
            sizeof(address)) < 0)
   {
      logger("[TCP] Error binding port to socket!", LOG_ERR, false);
      return false;
   }

   // Start listening for connections
   if (listen(master_socket, MAX_CONNECT_QUEUE) < 0)
   {
      logger("[TCP] Error starting listen()!", LOG_ERR, false);
      return false;
   }

   // Get length of host address
   addrlen = sizeof(address);

   // Start handling connections and messages
   tcp_thread = std::thread(&TCPServer::main_loop, this);

   logger("[TCP] Listening on port " + std::to_string(port), LOG_SUCC, false);
   return true;
}

/*
 * Stop execution of TCPServer
 */
void TCPServer::stop()
{
   // Stop listener thread
   running = false;

   // Wait for listener thread to stop
   tcp_thread.join();
}

/*
 * Give TCPServer access to LightStates struct
 * Parameters:
 *  - LightStates &light_states: reference to light states struct
 *  - std::timed_mutex &light_states_lock: reference to light states mutex
 */
void TCPServer::set_light_states(LightStates &light_states, std::timed_mutex &light_states_lock)
{
   this->light_states = &light_states;
   this->light_states_lock = &light_states_lock;
}

/*
 * Give TCPServer all its configuration parameters
 * Parameters:
 *  - int port: TCP port to bind to
 *  - int fps: Rendering FPS to calculate fade deltas
 *  - int default_transition: Default transition value to apply to fades
 *    without transition specified
 */
void TCPServer::configure(int port, int fps, int default_transition)
{
   this->port = port;
   this->fps = fps;
   this->default_transition = default_transition;
}

/*
 * Give TCPServer access to persistency_writer_cv
 * Parameters:
 *  - std::condition_variable &persistency_writer_cv: reference to persistency writer cv
 */
void TCPServer::set_persistency_writer_cv(std::condition_variable &persistency_writer_cv)
{
   this->persistency_writer_cv = &persistency_writer_cv;
}

/*
 ********** PRIVATE FUNCTIONS **********
 */

/*
 * Initializes client sockets araray to 0
 */
void TCPServer::init_client_sockets_array()
{
   for (int i = 0; i < max_clients; i++)
      client_socket[i] = 0;
}

/*
 * Main event handling loop
 */
void TCPServer::main_loop()
{
   while (running)
   {
      // Clear socket set
      FD_ZERO(&readfds);

      // Add master socket to socket set
      FD_SET(master_socket, &readfds);
      max_sd = master_socket;

      // Add client sockets to set
      add_client_sockets_to_set();

      // Wait for activity on any of the sockets
      activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

      // Check for error on select()
      if (activity < 0 && errno != EINTR)
      {
         logger("[TCP] Error on select()", LOG_WARN, true);
         continue;
      }

      // If the master socket has an action
      if (FD_ISSET(master_socket, &readfds))
      {
         accept_connection();
      }

      // Check all client sockets
      for (int i = 0; i < max_clients; i++)
      {
         sd = client_socket[i];

         // If a client socket has an action
         if (FD_ISSET(sd, &readfds))
         {
            handle_action_from_client(sd, i);
         }
      }
   }
}

/*
 * Adds client socket file descriptors to socket set
 */
void TCPServer::add_client_sockets_to_set()
{
   for (int i = 0; i < max_clients; i++)
   {
      // Get socket descriptor
      sd = client_socket[i];

      // If descriptor is valid, add to set
      if (sd > 0)
         FD_SET(sd, &readfds);

      // Keep track of biggest socket descriptor
      if (sd > max_sd)
         max_sd = sd;
   }
}

/*
 * Accepts new connection from client and sends
 * welcome message
 */
void TCPServer::accept_connection()
{
   if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
   {
      logger("[TCP] Error accepting client", LOG_WARN, true);
      return;
   }

   std::string address_string(inet_ntoa(address.sin_addr));
   logger("[TCP] New connection from " + address_string, LOG_INFO, true);

   // Add socket to array of client sockets
   if (!add_client_to_client_sockets(new_socket))
   {
      // If there wasn't space in the clients array
      // close the connection
      close(new_socket);

      logger("[TCP] Client " + address_string + " rejected: too many clients", LOG_WARN, true);

      return;
   }

   logger("[TCP] Client " + address_string + " accepted!", LOG_SUCC, true);

   // Send welcome message
   send_string(new_socket, client_welcome_message);
}

/*
 * Sends a string through a socket
 */
bool TCPServer::send_string(int socketfd, std::string message)
{
   // Convert string to char array
   const char *message_char = message.c_str();

   // Send message
   if (send(socketfd, message_char, strlen(message_char), 0) < 0)
   {
      return false;
   }

   return true;
}

/*
 * Stores new socket in array of managed sockets
 * Parameters:
 *  - int socketfd: socket to add to the array
 * Returns: true if there was an empty spot in the array,
 *    false if there wasn't
 */
bool TCPServer::add_client_to_client_sockets(int socketfd)
{
   bool found = false;
   for (int i = 0; i < max_clients; i++)
   {
      if (client_socket[i] == 0)
      {
         client_socket[i] = socketfd;
         found = true;
         break;
      }
   }

   return found;
}

/*
 * Handles an action coming from a client
 * Parameters:
 * - int socketfd: client socket file descriptor
 * - int i: position of fd in client_socket
 */
void TCPServer::handle_action_from_client(int socketfd, int i)
{
   if ((valread = read(socketfd, buffer, 255)) == 0)
   {
      // Client has disconnected

      // Get info of disconnected client
      getpeername(socketfd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
      std::string address_string(inet_ntoa(address.sin_addr));
      logger("[TCP] Client " + address_string + " disconnected", LOG_INFO, true);

      // Close connection
      close(socketfd);

      // Free up spot in client sockets array
      client_socket[i] = 0;
   }
   else
   {
      // Client has sent some data
      buffer[valread] = '\0'; // Terminate buffer string
      std::string message = buffer;

      // Remove whitespace characters from string
      message.erase(std::remove_if(message.begin(), message.end(), ::isspace), message.end());

      // Parse incoming message
      parse_message(message, socketfd);
   }
}

/*
 * Parses incoming message
 * Parameters:
 *  - std::string message: incoming message
 *  - int requestser_fd: requester socket file description
 */
void TCPServer::parse_message(std::string message, int client_fd)
{
   // Divide message into
   std::vector<std::string> message_split = split_string(message, ',');

   // Check there is a command type provided
   if (message_split.size() < 1)
      return;

   std::string command = message_split[0];

   // Recognize commands
   if (command == "sreq")
      status_request_message(client_fd);
   else if (command == "off")
      turn_off_message(message_split);
   else if (command == "on")
      turn_on_message(message_split);
   else
      logger("[TCP] Received: Unknown message type", LOG_WARN, true);
}

/*
 * Splits a string into a vector of strings based on a separator characters
 * Parameters:
 *  - std::string input: input string
 *  - char separator: separator characters
 * Returns: std:vector<std::string> output vector
 */
std::vector<std::string> TCPServer::split_string(std::string input, char seperator)
{
   std::string section;
   std::stringstream stream(input);
   std::vector<std::string> output;

   // Get separated parts from string stream
   while (std::getline(stream, section, seperator))
      output.push_back(section);

   return output;
}

/*
 * Handles a status request message from the client and sends correct response
 * parameters:
 *  - int client_fd: client socket file descriptor
 */
void TCPServer::status_request_message(int client_fd)
{
   logger("[TCP] Received: Status request message", LOG_INFO, true);
   std::string message;

   // Start with response message type
   message.append("sres");

   // Append all statuses
   for (int i = 0; i < 512; i++)
   {
      message.append(",");
      message.append(std::to_string(light_states->outward_state[i]));
      message.append("-");
      message.append(std::to_string(light_states->outward_brightness[i]));
   }
   message.append("\n");

   send_string(client_fd, message);
}

/*
 * Handles a turn off message from the client
 * parameters:
 *  - std::vector<std::string> split_message: complete message to get parameters
 */
void TCPServer::turn_off_message(std::vector<std::string> split_message)
{
   bool has_transition = false;
   int channel, transition;

   // Check if there are is at least space for the required fields
   if (split_message.size() < 2)
   {
      logger("[TCP] OFF Command, no channel given!", LOG_WARN, true);
      return;
   }

   // Convert channel number string to int
   try
   {
      channel = std::stoi(split_message[1]);

      // Check that channel value is acceptable
      if (channel < 0 || channel > 511)
      {
         logger("[TCP] OFF Command, channel number out of range!", LOG_WARN, true);
         return;
      }
   }
   catch (const std::exception &e)
   {
      logger("[TCP] OFF Command, bad channel!", LOG_WARN, true);
      return;
      return;
   }

   // Parse parameters
   for (long unsigned int i = 2; i < split_message.size(); i++)
   {
      // Check that the parameter isn't empty
      if (split_message[i].length() < 1)
         continue;

      // Parameter is transition
      if (split_message[i].at(0) == 't')
      {
         if (!has_transition)
         {
            // Check and store transition
            try
            {
               transition = std::stoi(split_message[i].substr(1));
               // Check that transition value is acceptable
               if (transition < 0)
               {
                  logger("[TCP] OFF Command, transition value out of range!", LOG_WARN, true);
                  return;
               }
            }
            catch (const std::exception &e)
            {
               logger("[TCP] OFF Command, bad transition!", LOG_WARN, true);
               return;
            }
            has_transition = true;
         }
      }
   }

   // Actually act on the light status arrays
   if (has_transition)
      logger("[TCP] OFF Command, channel: " + std::to_string(channel) + ", transition: " + std::to_string(transition) + "ms", LOG_INFO, true);
   else
      logger("[TCP] OFF Command, channel: " + std::to_string(channel), LOG_INFO, true);

   // Perform turn off fade
   start_off_fade(channel, has_transition, transition);
}

/*
 * Handles a turn on message from the client
 * parameters:
 *  - std::vector<std::string> split_message: complete message to get parameters
 */
void TCPServer::turn_on_message(std::vector<std::string> split_message)
{
   bool has_transition = false, has_brightness = false;
   int channel, transition, brightness;

   // Check if there are is at least space for the required fields
   if (split_message.size() < 2)
   {
      logger("[TCP] ON Command, no channel given!", LOG_WARN, true);
      return;
   }

   // Convert channel number string to int
   try
   {
      channel = std::stoi(split_message[1]);

      // Check that channel value is acceptable
      if (channel < 0 || channel > 511)
      {
         logger("[TCP] ON Command, channel number out of range!", LOG_WARN, true);
         return;
      }
   }
   catch (const std::exception &e)
   {
      logger("[TCP] ON Command, bad channel!", LOG_WARN, true);
      return;
   }

   // Parse parameters
   for (long unsigned int i = 2; i < split_message.size(); i++)
   {
      // Check that the parameter isn't empty
      if (split_message[i].length() < 1)
         continue;

      // Parameter is brightness
      if (split_message[i].at(0) == 'b')
      {
         if (!has_brightness)
         {
            // Check and store brightness
            try
            {
               brightness = std::stoi(split_message[i].substr(1));

               // Check that brightness value is acceptable
               if (brightness < 0 || brightness > 255)
               {
                  logger("[TCP] ON Command, brightness value out of range!", LOG_WARN, true);
                  return;
               }
            }
            catch (const std::exception &e)
            {
               logger("[TCP] ON Command, bad brightness!", LOG_WARN, true);
               return;
            }
            has_brightness = true;
         }
      }

      // Parameter is transition
      else if (split_message[i].at(0) == 't')
      {
         if (!has_transition)
         {
            // Check and store transition
            try
            {
               transition = std::stoi(split_message[i].substr(1));
               // Check that transition value is acceptable
               if (transition < 0)
               {
                  logger("[TCP] ON Command, transition value out of range!", LOG_WARN, true);
                  return;
               }
            }
            catch (const std::exception &e)
            {
               logger("[TCP] ON Command, bad transition!", LOG_WARN, true);
               return;
            }
            has_transition = true;
         }
      }
   }

   if (has_brightness)
   {
      if (has_transition)
         logger("[TCP] ON Command, channel: " + std::to_string(channel) + ", brightness: " + std::to_string(brightness) + ", transition: " + std::to_string(transition) + "ms", LOG_INFO, true);
      else
         logger("[TCP] ON Command, channel: " + std::to_string(channel) + ", brightness: " + std::to_string(brightness), LOG_INFO, true);
   }
   else
   {
      if (has_transition)
         logger("[TCP] ON Command, channel: " + std::to_string(channel) + ", transition: " + std::to_string(transition) + "ms", LOG_INFO, true);
      else
         logger("[TCP] ON Command, channel: " + std::to_string(channel), LOG_INFO, true);
   }

   start_on_fade(channel, has_brightness, has_transition, brightness, transition);
}

void TCPServer::start_on_fade(int channel, bool has_brightness, bool has_transition, int brightness, int transition)
{
   // Acquire lock on light states
   light_states_lock->lock();

   // If brightness was not provided in the message, turn on to previous known brightness
   if (!has_brightness)
      brightness = light_states->outward_brightness[channel];

   // If transition was not provided, use default transition
   if (!has_transition)
      transition = default_transition;

   // Set fade variables
   light_states->fade_progress[channel] = 0;
   light_states->fade_delta[channel] = 1000.0 / (fps * transition);   // 1 / FPS * transition if transition was in seconds
   light_states->fade_start[channel] = light_states->fade_current[0]; // Start where last fade ended
   light_states->fade_end[channel] = brightness;

   // Set outward facing states
   light_states->outward_state[channel] = true;
   light_states->outward_brightness[channel] = brightness;

   // Free lock
   light_states_lock->unlock();

   logger("[LIGHT] Starting fade, channel: " + std::to_string(channel) + ", start: " + std::to_string(light_states->fade_start[channel]) + ", end: " + std::to_string(light_states->fade_end[channel]) + ", delta: " + std::to_string(light_states->fade_delta[channel]), LOG_INFO, true);

   // Notify persistency writer of change
   persistency_writer_cv->notify_all();
}

void TCPServer::start_off_fade(int channel, bool has_transition, int transition)
{
   // Acquire lock on light states
   light_states_lock->lock();

   // If transition was not provided, use default transition
   if (!has_transition)
      transition = default_transition;

   // Set transition variables
   light_states->fade_progress[channel] = 0;
   light_states->fade_delta[channel] = 1000.0 / (fps * transition);   // 1 / FPS * transition if transition was in seconds
   light_states->fade_start[channel] = light_states->fade_current[0]; // Start where last fade ended
   light_states->fade_end[channel] = 0;

   // Set outward facing states
   light_states->outward_state[channel] = false;

   // Free lock
   light_states_lock->unlock();

   logger("[LIGHT] Starting fade, channel: " + std::to_string(channel) + ", start: " + std::to_string(light_states->fade_start[channel]) + ", end: " + std::to_string(light_states->fade_end[channel]) + ", delta: " + std::to_string(light_states->fade_delta[channel]), LOG_INFO, true);

   // Notify persistency writer of change
   persistency_writer_cv->notify_all();
}