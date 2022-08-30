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
TCPServer::TCPServer(int port)
{
   // Set port
   this->port = port;

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

   // Create master socket
   if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
      std::cout << "Error on socket()" << std::endl;
      return false;
   }

   // Allow multiple incoming connections
   if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
                  (char *)&opt, sizeof(opt)) < 0)
   {
      std::cout << "Error on setsockopt()" << std::endl;
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
      std::cout << "Error on bind()" << std::endl;
      return false;
   }

   // Start listening for connections
   if (listen(master_socket, MAX_CONNECT_QUEUE) < 0)
   {
      std::cout << "Error on listen()" << std::endl;
      return false;
   }

   // Get length of host address
   addrlen = sizeof(address);

   // Start handling connections and messages
   tcp_thread = std::thread(&TCPServer::main_loop, this);

   return true;
}

void TCPServer::set_light_states(LightStates &light_states)
{
   this->light_states = &light_states;
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
   std::cout << "In the main loop!" << std::endl;

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
         std::cout << "Error on select()" << std::endl;
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
      std::cout << "Failure accepting client" << std::endl;
      return;
   }

   std::cout << "New connection from " << inet_ntoa(address.sin_addr) << std::endl;

   // Send welcome message
   send_string(new_socket, client_welcome_message);

   // Add socket to array of client sockets
   if (!add_client_to_client_sockets(new_socket))
   {
      // If there wasn't space in the clients array
      // close the connection
      close(new_socket);

      std::cout << "Client rejected: too many clients" << std::endl;
   }
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
   if ((valread = read(sd, buffer, 255)) == 0)
   {
      // Client has disconnected

      std::cout << "Client disconnected" << std::endl;

      // Close connection
      close(sd);

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
      parse_message(message);
   }
}

void TCPServer::parse_message(std::string message)
{
   // Check that the message received is of a valid length
   if (message.length() < 2)
      return;

   std::string message_type = message.substr(0, 2);

   std::cout << message_type << std::endl;
}