#include "functions.h"
#include <ctime>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>
using namespace std;
using json = nlohmann::json;
tm parse_datetime(const string &dt) {
    tm time_struct = {};
    stringstream ss(dt);
    ss >> get_time(&time_struct, "%Y-%m-%d %H:%M");
    time_struct.tm_isdst = -1;
    return time_struct;
}

string format_datetime(const tm &time_struct) {
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

void add_new_event(const string &title, const string &id, const string &start, const string &end, const string &user, const string &description, const string &origin, const string &recurrence, const string &recurrence_id, const string &priority, const string &subgroup) {
    string path = (origin == "private") ? "users/" + user + "/events.json" : "groups/" + origin + "/events.json";

    filesystem::path p(path);
    if (not filesystem::exists(p.parent_path()))
        filesystem::create_directories(p.parent_path());

    json data = json::array();
    if (filesystem::exists(path)) {
        ifstream fin(path);
        if (fin.is_open())
            fin >> data;
    }
    if (not data.is_array())
        data = json::array();

    int occurrences = 1;
    if (recurrence == "daily")
        occurrences = 30;
    else if (recurrence == "weekly")
        occurrences = 12;
    else if (recurrence == "monthly")
        occurrences = 6;
    else if (recurrence == "yearly")
        occurrences = 2;

    for (int i = 0; i < occurrences; i++) {
        struct tm tm_start = {0}, tm_end = {0};
        char sep_s = ' ', sep_e = ' ';

        if (sscanf(start.c_str(), "%d-%d-%d%c%d:%d", &tm_start.tm_year, &tm_start.tm_mon, &tm_start.tm_mday, &sep_s, &tm_start.tm_hour, &tm_start.tm_min) < 5) {
            tm_start.tm_hour = 0;
            tm_start.tm_min = 0;
            sscanf(start.c_str(), "%d-%d-%d", &tm_start.tm_year, &tm_start.tm_mon, &tm_start.tm_mday);
        }
        if (sscanf(end.c_str(), "%d-%d-%d%c%d:%d", &tm_end.tm_year, &tm_end.tm_mon, &tm_end.tm_mday, &sep_e, &tm_end.tm_hour, &tm_end.tm_min) < 5) {
            tm_end.tm_hour = 23;
            tm_end.tm_min = 59;
            sscanf(end.c_str(), "%d-%d-%d", &tm_end.tm_year, &tm_end.tm_mon, &tm_end.tm_mday);
        }

        tm_start.tm_year -= 1900;
        tm_start.tm_mon -= 1;
        tm_end.tm_year -= 1900;
        tm_end.tm_mon -= 1;

        if (recurrence == "daily") {
            tm_start.tm_mday += i;
            tm_end.tm_mday += i;
        } else if (recurrence == "weekly") {
            tm_start.tm_mday += i * 7;
            tm_end.tm_mday += i * 7;
        } else if (recurrence == "monthly") {
            tm_start.tm_mon += i;
            tm_end.tm_mon += i;
        } else if (recurrence == "yearly") {
            tm_start.tm_year += i;
            tm_end.tm_year += i;
        }

        tm_start.tm_isdst = -1;
        tm_end.tm_isdst = -1;
        time_t t_s = mktime(&tm_start);
        time_t t_e = mktime(&tm_end);

        char buf_s[64], buf_e[64];
        struct tm *format_s = localtime(&t_s);
        if (start.find('T') != string::npos)
            strftime(buf_s, sizeof(buf_s), "%Y-%m-%dT%H:%M", format_s);
        else
            strftime(buf_s, sizeof(buf_s), "%Y-%m-%d %H:%M", format_s);

        struct tm *format_e = localtime(&t_e);
        if (end.find('T') != string::npos)
            strftime(buf_e, sizeof(buf_e), "%Y-%m-%dT%H:%M", format_e);
        else
            strftime(buf_e, sizeof(buf_e), "%Y-%m-%d %H:%M", format_e);

        string instance_id = id;
        string rec_id = recurrence_id.empty() ? id : recurrence_id;
        if (recurrence != "none" and not recurrence.empty()) {
            instance_id = id + "_" + to_string(i);
        }

        json new_ev = {
            {"id", instance_id},
            {"title", title},
            {"start", string(buf_s)},
            {"end", string(buf_e)},
            {"user", user},
            {"description", description},
            {"origin", origin},
            {"recurrence", recurrence},
            {"recurrence_id", rec_id},
            {"priority", priority},
            {"subgroup", subgroup}};
        data.push_back(new_ev);
    }

    ofstream fout(path);
    fout << data.dump(4);
}

void delete_event(const string &id, const string &user, const string &origin, bool delete_all) {
    string path = (origin == "private") ? "users/" + user + "/events.json" : "groups/" + origin + "/events.json";
    if (not filesystem::exists(path))
        return;

    json data;
    ifstream fin(path);
    fin >> data;
    fin.close();

    string rec_id = "";
    for (const auto &item : data) {
        if (item["id"] == id) {
            rec_id = item.value("recurrence_id", "");
            break;
        }
    }

    json filtered = json::array();
    for (const auto &item : data) {
        if (delete_all and not rec_id.empty() and item.value("recurrence_id", "") == rec_id) {
            continue;
        }
        if (item["id"] == id) {
            continue;
        }
        filtered.push_back(item);
    }

    ofstream fout(path);
    fout << filtered.dump(4);
    fout.close();
}
void edit_event(const string &title, const string &id, const string &start, const string &end, const string &user, const string &description, const string &origin, const string &recurrence, bool edit_all, const string &priority) {
    string path = (origin == "private") ? "users/" + user + "/events.json" : "groups/" + origin + "/events.json";
    if (not filesystem::exists(path))
        return;

    json data;
    ifstream fin(path);
    fin >> data;

    if (not data.is_array())
        return;

    string recurrence_id = "";
    bool found = false;

    for (const auto &item : data) {
        if (item.is_object() and item.value("id", "") == id) {
            recurrence_id = item.value("recurrence_id", "");
            found = true;
            break;
        }
    }
    if (not found)
        return;

    if (edit_all and not recurrence_id.empty()) {
        json filtered = json::array();
        for (const auto &item : data) {
            if (item.is_object() and item.value("recurrence_id", "") == recurrence_id) {
                continue; // drop old occurrence
            }
            filtered.push_back(item);
        }
        ofstream fout(path);
        fout << filtered.dump(4);
        fout.close();

        string new_base_id = to_string(time(0)) + "_" + to_string(rand() % 1000);
        add_new_event(title, new_base_id, start, end, user, description, origin, recurrence, "", priority, "");
        return;
    }

    bool updated = false;
    for (auto &item : data) {
        if (item.is_object() and item.value("id", "") == id) {
            item["title"] = title;
            item["start"] = start;
            item["end"] = end;
            item["description"] = description;
            item["recurrence"] = recurrence;
            item["priority"] = priority;
            updated = true;
            break;
        }
    }

    if (updated) {
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
vector<Event> get_user_event(const string &username) {
    string path = "users/" + username + "/events.json";
    if (not filesystem::exists(path))
        return {};
    ifstream fin(path);
    json data;
    fin >> data;
    vector<Event> events;
    for (const auto &item : data) {
        events.emplace_back(
            item.value("title", ""), item.value("id", ""),
            item.value("start", ""), item.value("end", ""),
            item.value("user", ""), item.value("description", ""),
            item.value("origin", "private"),
            item.value("recurrence", "none"),
            item.value("recurrence_id", ""),
            item.value("priority", "medium"),
            item.value("subgroup", ""));
    }
    return events;
}

void get_group_events(const json &g, vector<Event> &events) {
    string g_id = g.value("id", "");
    string group_events_path = "groups/" + g_id + "/events.json";

    if (not filesystem::exists(group_events_path))
        return;

    ifstream g_fin(group_events_path);
    json g_data;
    g_fin >> g_data;

    for (const auto &item : g_data) {
        events.emplace_back(item.value("title", ""), item.value("id", ""),
                            item.value("start", ""), item.value("end", ""),
                            item.value("user", ""), item.value("description", ""),
                            g_id, item.value("recurrence", "none"), item.value("recurrence_id", ""),
                            item.value("priority", "medium"), item.value("subgroup", ""));
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
    json info = {
        {"name", group_name},
        {"id", id},
        {"subgroups", json::array()}
    };
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

void add_subgroup(const string &group_id, const string &subgroup_path) {
    string info_path = "groups/" + group_id + "/info.json";
    if (not filesystem::exists(info_path))
        return;

    ifstream fin(info_path);
    json info;
    fin >> info;
    fin.close();

    if (not info.contains("subgroups"))
        info["subgroups"] = json::object();

    // Parse subgroup path (e.g., "team1/subteam1")
    vector<string> parts;
    stringstream ss(subgroup_path);
    string part;
    while (getline(ss, part, '/')) {
        if (not part.empty())
            parts.push_back(part);
    }

    if (parts.empty())
        return;

    // Navigate/create nested structure
    json *current = &info["subgroups"];
    for (size_t i = 0; i < parts.size(); i++) {
        if (not current->contains(parts[i])) {
            (*current)[parts[i]] = json::object();
        }
        if (i < parts.size() - 1) {
            current = &(*current)[parts[i]];
        }
    }

    ofstream fout(info_path);
    fout << info.dump(4);
}

void flatten_subgroups(const json &node, const string &prefix, vector<string> &result) {
    if (node.is_object()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            string path = prefix.empty() ? it.key() : prefix + "/" + it.key();
            result.push_back(path);
            flatten_subgroups(it.value(), path, result);
        }
    }
}

vector<string> get_subgroups(const string &group_id) {
    string info_path = "groups/" + group_id + "/info.json";
    vector<string> subgroups;

    if (not filesystem::exists(info_path))
        return subgroups;

    ifstream fin(info_path);
    json info;
    fin >> info;

    if (info.contains("subgroups")) {
        if (info["subgroups"].is_array()) {
            // Legacy format: flat array
            for (const auto &sg : info["subgroups"]) {
                subgroups.push_back(sg);
            }
        } else if (info["subgroups"].is_object()) {
            // New format: nested object
            flatten_subgroups(info["subgroups"], "", subgroups);
        }
    }

    return subgroups;
}

string get_group_name(const string &group_id) {
    if (group_id.empty() or group_id == "private")
        return "private";

    string info_path = "groups/" + group_id + "/info.json";
    if (not filesystem::exists(info_path))
        return group_id;

    ifstream fin(info_path);
    json info;
    fin >> info;

    return info.value("name", group_id);
}

bool change_event_group(const string &event_id, const string &new_origin) {
    vector<string> search_paths = {"users"};
    if (filesystem::exists("groups")) {
        for (const auto &entry : filesystem::directory_iterator("groups")) {
            search_paths.push_back("groups/" + entry.path().filename().string());
        }
    }

    Event found_event;
    string old_path;
    bool event_found = false;

    for (const auto &base : search_paths) {
        string events_path;
        if (base == "users") {
            for (const auto &user_dir : filesystem::directory_iterator(base)) {
                events_path = user_dir.path().string() + "/events.json";
                if (not filesystem::exists(events_path)) continue;

                ifstream fin(events_path);
                json data;
                fin >> data;
                fin.close();

                for (const auto &item : data) {
                    if (item.value("id", "") == event_id) {
                        found_event = Event(
                            item.value("title", ""), item.value("id", ""),
                            item.value("start", ""), item.value("end", ""),
                            item.value("user", ""), item.value("description", ""),
                            item.value("origin", "private"),
                            item.value("recurrence", "none"),
                            item.value("recurrence_id", ""),
                            item.value("priority", "medium"),
                            item.value("subgroup", "")
                        );
                        old_path = events_path;
                        event_found = true;
                        break;
                    }
                }
                if (event_found) break;
            }
        } else {
            events_path = base + "/events.json";
            if (not filesystem::exists(events_path)) continue;

            ifstream fin(events_path);
            json data;
            fin >> data;
            fin.close();

            for (const auto &item : data) {
                if (item.value("id", "") == event_id) {
                    found_event = Event(
                        item.value("title", ""), item.value("id", ""),
                        item.value("start", ""), item.value("end", ""),
                        item.value("user", ""), item.value("description", ""),
                        item.value("origin", ""),
                        item.value("recurrence", "none"),
                        item.value("recurrence_id", ""),
                        item.value("priority", "medium"),
                        item.value("subgroup", "")
                    );
                    old_path = events_path;
                    event_found = true;
                    break;
                }
            }
        }
        if (event_found) break;
    }

    if (not event_found) return false;

    delete_event(event_id, found_event.user, found_event.origin, false);

    add_new_event(found_event.title, found_event.id, found_event.start, found_event.end,
                  found_event.user, found_event.description, new_origin,
                  found_event.recurrence, found_event.recurrence_id, found_event.priority, found_event.subgroup);

    return true;
}
