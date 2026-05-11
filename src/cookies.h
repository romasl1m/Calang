#pragma once
#include <string>
#include <unordered_map>
using namespace std;
extern unordered_map<string, string> active_sessions;
string generate_session_id();
string get_logged_in_user(const string& cookie_header);
