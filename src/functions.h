#pragma once
#include <string>
#include <vector>
#include "event.h"
using namespace std;

bool user_exists(const string &username);
bool password_correct(const string &username, const string &password);
string getParam(const string &body, const string &key);
string urlDecode(string str);
string loadHtmlTemplate(const string &filePath);

void delete_event(const string &id, const string &user, const string &origin, bool delete_all = false);
void add_new_event(const std::string &title, const std::string &id, const std::string &start, const std::string &end, const std::string &user, const std::string &description, const std::string &origin, const std::string &recurrence, const std::string &recurrence_id = "", const std::string &priority = "medium", const std::string &subgroup = "");
void edit_event(const std::string &title, const std::string &id, const std::string &start, const std::string &end, const std::string &user, const std::string &description, const std::string &origin, const std::string &recurrence, bool edit_all, const std::string &priority = "medium");
bool change_event_group(const std::string &event_id, const std::string &new_origin);
vector<Event> get_user_event(const string &username);
vector<Event> get_all_events(const string &username);
void get_group_events(const json &g, vector<Event> &events);

vector<json> get_user_groups(const string &username);
void create_group(const string &group_name, const string &creator, string &id);
void join_group(const string &username, const string &group_id);
void add_subgroup(const string &group_id, const string &subgroup_name);
vector<string> get_subgroups(const string &group_id);
string get_group_name(const string &group_id);
