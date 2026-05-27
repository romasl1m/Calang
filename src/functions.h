#pragma once
#include <string>
#include <vector>
#include "event.h"
using namespace std;

bool user_exists(const string& username);
bool password_correct(const string& username, const string& password);
string getParam(const string& body, const string& key);
string urlDecode(string str);
string loadHtmlTemplate(const string& filePath);

void delete_event(const string& id, const string& user, const string& origin, bool delete_all = false);
void add_new_event(const std::string& title, const std::string& id, const std::string& start, const std::string& end, const std::string& user, const std::string& description, const std::string& origin, const std::string& recurrence);
void edit_event(const std::string& title, const std::string& id, const std::string& start, const std::string& end, const std::string& user, const std::string& description, const std::string& origin, const std::string& recurrence, bool edit_all = false);
vector<Event> get_user_event(const string& username);
vector<Event> get_all_events(const string& username);
void get_group_events(const json& g, vector<Event>& events);

vector<json> get_user_groups(const string& username);
void create_group(const string& group_name, const string& creator, string& id);
void join_group(const string& username, const string& group_id);
