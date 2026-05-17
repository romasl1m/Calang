#pragma once
#include <string>
#include <vector>
#include "event.h"
using namespace std;

bool user_exists(const string &username);
bool password_correct(const string &username, const string &password);
string getParam(const string& body, const string& key);
vector<Event> get_user_event(const string& username);
int get_minutes(const string& timestamp);
string urlDecode(string str);
void add_new_event(const string& title, const string& id, const string& start, const string& end, const string& user, const string& description, const string& origin);
string loadHtmlTemplate(const string& filePath);
void delete_event(const string &id, const string &user, const string& origin);
void edit_event(const string& title, const string &id, const string& start, const string& end, const string &user, const string& description, const string& origin);
void add_group_event(const string& group_name, Event e);
vector<json> get_user_groups(const string& username);
void create_group(const string& group_name, const string& creator, string& id);
void join_group(const string& username, const string& group_id);
vector<Event> get_all_events(const string& username);
vector<Event> get_user_event(const string& username);
void get_group_events(const json& g, vector<Event>& events);
