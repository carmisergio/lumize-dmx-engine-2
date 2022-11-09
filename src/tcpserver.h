/*
 * Filename: tcpserver.h
 * Description: interface for the TcpServer class
 * Author: Sergio Carmine <me@sergiocarmi.net>
 * Date of Creation: 30/08/2022
 */

#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <thread>
#include <chrono>
#include <ctype.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <mutex>
#include <condition_variable>

// Network libraries
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lightstates.h"
#include "logger.h"

#define DEFAULT_PORT 3141
#define MAX_CLIENTS 5
#define MAX_CONNECT_QUEUE 5
#define CLIENT_WELCOME_MESSAGE "Lumize DMX Engine v2.0\n"

/*
 * Definition of the TcpServer class
 */
class TCPServer
{
public:
   // Constructor
   TCPServer();

   // Methods
   bool start();
   void stop();
   void set_light_states(LightStates &light_states, std::timed_mutex &light_states_lock);
   void send_state_update();
   void configure(int port, int fps, int default_transition, int direction_reset_delay);
   void set_persistency_writer_cv(std::condition_variable &persistency_writer_cv);

private:
   int master_socket,
       addrlen,
       new_socket,
       activity,
       i,
       valread,
       sd;
   int client_socket[MAX_CLIENTS];
   int max_sd, max_clients = MAX_CLIENTS;
   struct sockaddr_in address;
   std::string client_welcome_message = CLIENT_WELCOME_MESSAGE;
   bool running = true;
   char buffer[255];

   // Socket description set
   fd_set readfds;

   std::thread tcp_thread;

   LightStates *light_states;
   std::timed_mutex *light_states_lock;

   // PersistencyWriter condition variable
   std::condition_variable *persistency_writer_cv;

   // Config
   int port, fps, default_transition, direction_reset_delay;

   // Internal functions
   void init_client_sockets_array();
   void main_loop();
   void add_client_sockets_to_set();
   void accept_connection();
   bool send_string(int socketfd, std::string message);
   bool add_client_to_client_sockets(int socketfd);
   void handle_action_from_client(int socketfd, int i);
   void parse_message(std::string message, int client_fd);
   std::vector<std::string> split_string(std::string input, char seperator);
   void status_request_message(std::vector<std::string> split_message, int client_fd);
   void turn_off_message(std::vector<std::string> split_message, int client_fd);
   void turn_on_message(std::vector<std::string> split_message, int client_fd);
   void pushbutton_fade_end_message(std::vector<std::string> split_message, int client_fd);
   void pushbutton_fade_start_message(std::vector<std::string> split_message, int client_fd);
   void start_on_fade(int channel, bool has_brightness, bool has_transition, int brightness, int transition);
   void start_off_fade(int channel, bool has_transition, int transition);
   void start_pushbutton_fade(int channel, bool has_direction, bool is_direction_up);
   void end_pushbutton_fade(int channel);
   bool get_pushbutton_fade_direction(int channel, bool has_direction, bool is_direction_up);
};
