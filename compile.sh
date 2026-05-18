#!/bin/bash
#g++ -std=c++17 src/login.cpp src/event.cpp -lpthread -o serwer
# g++ -O3 src/main.cpp src/core_routes.cpp src/registration.cpp src/login.cpp src/dashboard.cpp src/api.cpp src/functions.cpp src/cookies.cpp -o app -I/.include -lpthread -lboost_system
# g++ -O3 src/main.cpp src/api.cpp src/functions.cpp src/cookies.cpp -o calang_app -I./include -lpthread
g++ -O3 src/main.cpp src/core_routes.cpp src/registration.cpp src/login.cpp src/dashboard.cpp src/api.cpp src/functions.cpp src/cookies.cpp src/terminal.cpp -o app -I./include -lpthread -lboost_system #18.05.26
