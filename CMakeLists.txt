cmake_minimum_required(VERSION 3.16)
project(zad2)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-std=gnu++20 -Wall -Wextra -Wconversion -Werror -O2 -pthread")

add_executable(robots-client
	client/client.cpp
	client/utils.h
	client/utils.cpp
	client/server_message.h
	client/server_message.cpp
	client/input_parser.cpp
	client/input_parser.h
	client/map_objects.cpp
	client/map_objects.h
	client/player.cpp
	client/player.h
	client/server.cpp
	client/server.h
	client/gui.cpp
	client/gui.h
	client/event.cpp
	client/event.h
	client/move_message.cpp
	client/move_message.h
	client/game_state.cpp
	client/game_state.h
	client/buffer.h)

add_executable(robots-server
	server/server.cpp
	server/utils.h
	server/map_objects.cpp
	server/map_objects.h
	server/player.cpp
	server/player.h
	server/client_connection.cpp
	server/client_connection.h
	server/event.cpp
	server/event.h
	server/move.cpp
	server/move.h
	server/game_state.cpp
	server/game_state.h
	server/buffer.h
	server/input_parser.cpp
	server/input_parser.h
	server/random_generator.cpp
	server/random_generator.h)

target_link_libraries(robots-client -lpthread -lboost_program_options)
target_link_libraries(robots-server -lpthread -lboost_program_options)
