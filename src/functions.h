#pragma once
#include <string>
#include <vector>
#include "event.cpp"
using namespace std;

bool user_exists(const string &username);
bool password_correct(const string &username, const string &password);
string getParam(const string& body, const string& key);
vector<Event> get_user_event(const string& username);
int get_minutes(const string& timestamp);
string urlDecode(string str);
void add_new_event(const string& title, const string& id, const string& start, const string& end, const string& user, const string& description);
string loadHtmlTemplate(const string& filePath);
void delete_event(const string &id, const string &user);
void edit_event(const string& title, const string &id, const string& start, const string& end, const string &user, const string& description);
