#include "functions.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>
using namespace std;
using json = nlohmann::json;
tm parse_datetime(const string& dt){
    tm time_struct = {};
    stringstream ss(dt);
    ss >> get_time(&time_struct, "%Y-%m-%d %H:%M");
    time_struct.tm_isdst = -1; 
    return time_struct;
}

string format_datetime(const tm& time_struct){
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M", &time_struct);
    return string(buffer);
}

bool user_exists(const string &username) {
  if (not filesystem::exists("users.json"))
    return false;
  ifstream fin("users.json");
  json users;
  fin >> users;
  return users.contains(username);
}

bool password_correct(const string &username, const string &password) {
  if (not filesystem::exists("users.json"))
    return false;
  ifstream fin("users.json");
  json users;
  fin >> users;
  if (users.contains(username))
    return users[username] == password;
  return false;
}

string getParam(const string &body, const string &key) {
  size_t pos = body.find(key + "=");
  if (pos == string::npos)
    return "";
  size_t start = pos + key.length() + 1;
  size_t end = body.find("&", start);
  if (end == string::npos)
    end = body.length();
  return body.substr(start, end - start);
}

string urlDecode(string str) {
  string res;
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '+')
      res += ' ';
    else if (str[i] == '%' && i + 2 < str.length()) {
      res += (char)stoi(str.substr(i + 1, 2), nullptr, 16);
      i += 2;
    } else
      res += str[i];
  }
  return res;
}
void join_group(const string &username, const string &group_id) {
  string path = "groups/" + group_id + "/members.json";
  if (not filesystem::exists(path))
    return;

  ifstream fin(path);
  json members;
  if (fin.is_open()) {
    fin >> members;
    fin.close();
  }

  for (const auto &m : members) {
    if (m == username)
      return;
  }

  members.push_back(username);

  ofstream fout(path);
  fout << members.dump(4);
}

void add_new_event(const string& title, const string& id, const string& start, const string& end, const string& user, const string& description, const string& origin, const string& recurrence){
    string path = (origin == "private") ? "users/" + user + "/events.json" : "groups/" + origin + "/events.json";

    filesystem::path p(path);
    if(not filesystem::exists(p.parent_path())) filesystem::create_directories(p.parent_path());

    json data = json::array();
    if(filesystem::exists(path)){
        ifstream fin(path);
        if(fin.is_open()) fin >> data;
    }

    tm current_start = parse_datetime(start);
    tm current_end = parse_datetime(end);
    int current_year = current_start.tm_year; 
    int i = 0;

    while(current_start.tm_year == current_year){
        string cur_start_str = format_datetime(current_start);
        string cur_end_str = format_datetime(current_end);
        
        string cur_id = id + "_" + to_string(i);

        json new_ev = {
            {"id", cur_id}, {"title", title}, {"start", cur_start_str}, {"end", cur_end_str},
            {"user", user}, {"description", description}, {"origin", origin},
            {"recurrence", recurrence}, {"recurrence_id", id}
        };
        data.push_back(new_ev);

        if(recurrence == "none") break;
        else if(recurrence == "daily"){
            current_start.tm_mday++;
            current_end.tm_mday++;
        }
        else if(recurrence == "weekly"){
            current_start.tm_mday += 7;
            current_end.tm_mday += 7;
        }
        else if(recurrence == "monthly"){
            current_start.tm_mon++;
            current_end.tm_mon++;
        }
        else if(recurrence == "yearly"){
            current_start.tm_year++;
            current_end.tm_year++;
        }
        else break;

        mktime(&current_start);
        mktime(&current_end);
        i++;
    }

    ofstream fout(path);
    fout << data.dump(4);
}

void delete_event(const string& id, const string& user, const string& origin, bool delete_all){
    string path = (origin == "private") ? "users/" + user + "/events.json" : "groups/" + origin + "/events.json";
    if(not filesystem::exists(path)) return;

    json data;
    ifstream fin(path);
    fin >> data;

    string rec_id = "";
    for(const auto& item : data){
        if(item["id"] == id){
            rec_id = item.value("recurrence_id", "");
            break;
        }
    }

    json filtered = json::array();
    for(const auto& item : data){
        if(delete_all and not rec_id.empty() and item.value("recurrence_id", "") == rec_id){
            continue;
        }
        if(item["id"] == id){
            continue;
        }
        filtered.push_back(item);
    }

    ofstream fout(path);
    fout << filtered.dump(4);
}
void edit_event(const string& title, const string& id, const string& start, const string& end, const string& user, const string& description, const string& origin, const string& recurrence, bool edit_all){
    string path = (origin == "private") ? "users/" + user + "/events.json" : "groups/" + origin + "/events.json";
    if(not filesystem::exists(path)) return;

    json data;
    ifstream fin(path);
    fin >> data;

    string recurrence_id = "";
    bool found = false;
    for(const auto& item : data){
        if(item["id"] == id){
            recurrence_id = item.value("recurrence_id", "");
            found = true;
            break;
        }
    }
    if(!found) return;

    if(edit_all and not recurrence_id.empty()){
        json filtered = json::array();
        for(const auto& item : data){
            if(item.value("recurrence_id", "") == recurrence_id){
                continue;
            }
            filtered.push_back(item);
        }
        ofstream fout(path);
        fout << filtered.dump(4);
        add_new_event(title, recurrence_id, start, end, user, description, origin, recurrence);
        return;
    }

    bool updated = false;
    for(auto& item : data){
        if(item["id"] == id){
            item["title"] = title;
            item["start"] = start;
            item["end"] = end;
            item["description"] = description;
            item["recurrence"] = recurrence;
            if(recurrence == "none"){
                item["recurrence_id"] = item["id"];
            } else if(recurrence_id.empty()){
                item["recurrence_id"] = item["id"];
            }
            updated = true;
            break;
        }
    }

    if(updated){
        ofstream fout(path);
        fout << data.dump(4);
    }
}

vector<json> get_user_groups(const string &username) {
  vector<json> groups;
  if (not filesystem::exists("groups"))
    return groups;
  for (const auto &entry : filesystem::directory_iterator("groups")) {
    string g_id = entry.path().filename().string();
    ifstream f_mem("groups/" + g_id + "/members.json");
    if (not f_mem.is_open())
      continue;
    json members;
    f_mem >> members;
    bool is_m = false;
    for (auto &m : members)
      if (m == username)
        is_m = true;
    if (is_m) {
      ifstream f_info("groups/" + g_id + "/info.json");
      json info;
      f_info >> info;
      groups.push_back(info);
    }
  }
  return groups;
}
vector<Event> get_user_event(const string &username){
    string path = "users/" + username + "/events.json";
    if(not filesystem::exists(path)) return {};
    ifstream fin(path);
    json data;
    fin >> data;
    vector<Event> events;
    for(const auto &item : data){
        events.emplace_back(item.value("title", ""), item.value("id", ""),
                            item.value("start", ""), item.value("end", ""),
                            item.value("user", ""), item.value("description", ""),
                            item.value("origin", "private"), item.value("recurrence", "none"));
    }
    return events;
}

void get_group_events(const json &g, vector<Event> &events){
    string g_id = g.value("id", "");
    string group_events_path = "groups/" + g_id + "/events.json";

    if(not filesystem::exists(group_events_path)) return;

    ifstream g_fin(group_events_path);
    json g_data;
    g_fin >> g_data;

    for(const auto &item : g_data){
        events.emplace_back(item.value("title", ""), item.value("id", ""),
                            item.value("start", ""), item.value("end", ""),
                            item.value("user", ""), item.value("description", ""),
                            g_id, item.value("recurrence", "none"));
    }
}
vector<Event> get_all_events(const string &username) {
  vector<Event> events = get_user_event(username);
  vector<json> groups = get_user_groups(username);
  for (const auto &g : groups) {
    get_group_events(g, events);
  }
  return events;
}
void create_group(const string &group_name, const string &creator, string &id) {
  id = to_string(time(0));
  string path = "groups/" + id;
  filesystem::create_directories(path);
  json members = {creator};
  json info = {{"name", group_name}, {"id", id}};
  ofstream f1(path + "/members.json");
  f1 << members.dump(4);
  ofstream f2(path + "/info.json");
  f2 << info.dump(4);
}
string loadHtmlTemplate(const string &filePath) {
  ifstream file(filePath);
  if (not file.is_open()) {
    return "404: Template not found (" + filePath + ")";
  }
  stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}
