#!/bin/bash
#g++ -std=c++17 src/login.cpp src/event.cpp -lpthread -o serwer
g++ src/main.cpp src/core_routes.cpp src/registration.cpp src/login.cpp src/dashboard.cpp src/api.cpp src/functions.cpp src/cookies.cpp -o app -lpthread -lboost_system
