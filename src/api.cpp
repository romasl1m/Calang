#include <crow/app.h>
#include <string>
#include <curl/curl.h>
#include "terminal.h"
#include <filesystem>
#include <fstream>
#include "functions.h"
#include "cookies.h"
#include <nlohmann/json.hpp>
#include "google_oauth.h"

using namespace std;
using json = nlohmann::json;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

void api_routes(crow::SimpleApp &app) {
    const char *env_key = std::getenv("GEMINI_API_KEY");
    string api_key = env_key ? env_key : "";
    cout << "API KEY ";
    for (char c : api_key) {
        cout << c << " ";
    }
    cout << endl;
    if (!env_key) {
        cerr << "GEMINI_API_KEY not found\n";
    }

    CROW_ROUTE(app, "/api/new_event").methods("POST"_method)([](const crow::request &req) {
        string title = urlDecode(getParam(req.body, "title"));
        string start = urlDecode(getParam(req.body, "start"));
        string end = urlDecode(getParam(req.body, "end"));
        string description = urlDecode(getParam(req.body, "description"));
        string origin = urlDecode(getParam(req.body, "origin"));
        if (origin.empty())
            origin = "private";
        string recurrence = urlDecode(getParam(req.body, "recurrence"));
        if (recurrence.empty())
            recurrence = "none";
        string priority = urlDecode(getParam(req.body, "priority"));
        if (priority.empty())
            priority = "medium";

        string id = to_string(time(0)) + "_" + to_string(rand() % 1000);

        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty()) {
            return crow::response(401, "Użytkownik nie zalogowany");
        }

        string subgroup = urlDecode(getParam(req.body, "subgroup"));
        add_new_event(title, id, start, end, user, description, origin, recurrence, "", priority, subgroup);

        // Try to create event in Google Calendar if user has access token
        string access_token = get_user_access_token(user);
        if (!access_token.empty()) {
            string calendar_response = createGoogleCalendarEvent(access_token, title, start, end, description);
            if (!calendar_response.empty()) {
                try {
                    json cal_json = json::parse(calendar_response);
                    if (cal_json.contains("error")) {
                        cerr << "Google Calendar error: " << cal_json["error"]["message"].get<string>() << endl;
                    }
                } catch (...) {
                    cerr << "Failed to parse Google Calendar response" << endl;
                }
            }
        }

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/api/delete_event").methods("POST"_method)([](const crow::request &req) {
        string id = urlDecode(getParam(req.body, "id"));
        string origin = urlDecode(getParam(req.body, "origin"));
        string del_all_str = urlDecode(getParam(req.body, "delete_all"));
        bool delete_all = (del_all_str == "true");

        if (origin.empty())
            origin = "private";

        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Użytkownik nie zalogowany");

        delete_event(id, user, origin, delete_all);

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/api/edit_event").methods("POST"_method)([](const crow::request &req) {
        string title = urlDecode(getParam(req.body, "title"));
        string id = urlDecode(getParam(req.body, "id"));
        string start = urlDecode(getParam(req.body, "start"));
        string end = urlDecode(getParam(req.body, "end"));
        string description = urlDecode(getParam(req.body, "description"));
        string origin = urlDecode(getParam(req.body, "origin"));
        bool edit_all = getParam(req.body, "edit_all") == "true";
        if (origin.empty())
            origin = "private";
        string recurrence = urlDecode(getParam(req.body, "recurrence"));
        if (recurrence.empty())
            recurrence = "none";
        string priority = urlDecode(getParam(req.body, "priority"));
        if (priority.empty())
            priority = "medium";
        string subgroup = urlDecode(getParam(req.body, "subgroup"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Nie zalogowano");

        edit_event(title, id, start, end, user, description, origin, recurrence, edit_all, priority, subgroup);

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/api/my_groups")([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty()) {
            return crow::response(401, "Unauthorized");
        }

        auto groups = get_user_groups(user);
        return crow::response(nlohmann::json(groups).dump());
    });

    CROW_ROUTE(app, "/api/create_group").methods("POST"_method)([](const crow::request &req) {
        string name = urlDecode(getParam(req.body, "name"));
        string max_people_str = urlDecode(getParam(req.body, "max_people"));
        string is_private_str = urlDecode(getParam(req.body, "is_private"));
        string password = urlDecode(getParam(req.body, "password"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Unauthorized");

        if (name.empty())
            return crow::response(400, json({{"error", "Group name cannot be empty"}}).dump());

        int max_people = 0;
        if (!max_people_str.empty()) max_people = stoi(max_people_str);
        bool is_private = (is_private_str == "true" || is_private_str == "1");

        string generated_id;
        create_group(name, user, generated_id, max_people, is_private, password);

        json response = {
            {"success", true},
            {"group_id", generated_id},
            {"message", "Group created successfully"}
        };

        crow::response res(200, response.dump());
        res.add_header("Content-Type", "application/json");
        return res;
    });

    CROW_ROUTE(app, "/api/join_group").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string password = urlDecode(getParam(req.body, "password"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Unauthorized");

        string path = "groups/" + group_id + "/members.json";
        string info_path = "groups/" + group_id + "/info.json";
        if (!filesystem::exists(path) || !filesystem::exists(info_path)) {
            return crow::response(404, "Group not found");
        }

        json info;
        ifstream finfo(info_path);
        if (finfo.is_open())
            finfo >> info;
        finfo.close();

        json members;
        ifstream fin(path);
        if (fin.is_open())
            fin >> members;
        fin.close();

        for (const auto &m : members) {
            if (m == user)
                return crow::response(200, "Already a member");
        }

        if (info.value("is_private", false)) {
            string expected_pass = info.value("password", "");
            if (password != expected_pass) {
                return crow::response(403, "Incorrect password");
            }
        }

        int max_people = info.value("max_people", 0);
        if (max_people > 0 && members.size() >= max_people) {
            return crow::response(403, "Group is full");
        }

        members.push_back(user);
        ofstream fout(path);
        fout << members.dump(4);
        fout.close();

        return crow::response(200, "Joined successfully");
    });

    CROW_ROUTE(app, "/api/delete_group").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Unauthorized");

        if (group_id.empty())
            return crow::response(400, "Group ID is required");

        // Check if user is the creator (first member)
        string members_path = "groups/" + group_id + "/members.json";
        if (!filesystem::exists(members_path)) {
            return crow::response(404, "Group not found");
        }

        ifstream fin(members_path);
        json members;
        fin >> members;
        fin.close();

        if (!members.is_array() || members.empty()) {
            return crow::response(404, "Group not found");
        }

        string creator = members[0].get<string>();
        if (creator != user) {
            return crow::response(403, json({{"error", "Only the group creator can delete the group"}}).dump());
        }

        // Delete the entire group directory
        string group_path = "groups/" + group_id;
        try {
            filesystem::remove_all(group_path);
            json response = {
                {"success", true},
                {"message", "Group deleted successfully"}
            };
            crow::response res(200, response.dump());
            res.add_header("Content-Type", "application/json");
            return res;
        } catch (const exception &e) {
            return crow::response(500, json({{"error", "Failed to delete group"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/add_subgroup").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string subgroup_path = urlDecode(getParam(req.body, "name"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Unauthorized");

        if (subgroup_path.empty())
            return crow::response(400, "Subgroup path cannot be empty");

        add_subgroup(group_id, subgroup_path);

        return crow::response(200, "Subgroup added successfully");
    });

    CROW_ROUTE(app, "/api/kick_member").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string username = urlDecode(getParam(req.body, "username"));
        string cookie_header = req.get_header_value("Cookie");
        string requester = get_logged_in_user(cookie_header);

        if (requester.empty())
            return crow::response(401, json({{"error", "Unauthorized"}}).dump());

        if (kick_member(group_id, username, requester)) {
            return crow::response(200, json({{"success", true}}).dump());
        } else {
            return crow::response(403, json({{"error", "Only the group creator can kick members"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/update_group_settings").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string max_people_str = urlDecode(getParam(req.body, "max_people"));
        string is_private_str = urlDecode(getParam(req.body, "is_private"));
        string password = urlDecode(getParam(req.body, "password"));
        string cookie_header = req.get_header_value("Cookie");
        string requester = get_logged_in_user(cookie_header);

        if (requester.empty())
            return crow::response(401, json({{"error", "Unauthorized"}}).dump());

        int max_people = max_people_str.empty() ? 0 : stoi(max_people_str);
        bool is_private = (is_private_str == "true" || is_private_str == "1");

        if (update_group_settings(group_id, requester, max_people, is_private, password)) {
            return crow::response(200, json({{"success", true}}).dump());
        } else {
            return crow::response(403, json({{"error", "Only the group creator can update settings"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/send_join_request").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);

        if (username.empty())
            return crow::response(401, json({{"error", "Unauthorized"}}).dump());

        if (send_join_request(group_id, username)) {
            return crow::response(200, json({{"success", true, "message", "Join request sent"}}).dump());
        } else {
            return crow::response(400, json({{"error", "Request already sent or group not found"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/approve_join_request").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string username = urlDecode(getParam(req.body, "username"));
        string cookie_header = req.get_header_value("Cookie");
        string requester = get_logged_in_user(cookie_header);

        if (requester.empty())
            return crow::response(401, json({{"error", "Unauthorized"}}).dump());

        if (approve_join_request(group_id, requester, username)) {
            return crow::response(200, json({{"success", true}}).dump());
        } else {
            return crow::response(403, json({{"error", "Only the group creator can approve requests"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/reject_join_request").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string username = urlDecode(getParam(req.body, "username"));
        string cookie_header = req.get_header_value("Cookie");
        string requester = get_logged_in_user(cookie_header);

        if (requester.empty())
            return crow::response(401, json({{"error", "Unauthorized"}}).dump());

        if (reject_join_request(group_id, requester, username)) {
            return crow::response(200, json({{"success", true}}).dump());
        } else {
            return crow::response(403, json({{"error", "Only the group creator can reject requests"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/send_invite").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string invitee = urlDecode(getParam(req.body, "invitee"));
        string cookie_header = req.get_header_value("Cookie");
        string inviter = get_logged_in_user(cookie_header);

        if (inviter.empty())
            return crow::response(401, json({{"error", "Unauthorized"}}).dump());

        if (send_invite(group_id, inviter, invitee)) {
            return crow::response(200, json({{"success", true, "message", "Invite sent"}}).dump());
        } else {
            return crow::response(400, json({{"error", "Cannot send invite"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/my_invites").methods("GET"_method)([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);

        if (username.empty())
            return crow::response(401, "Unauthorized");

        vector<string> group_ids = get_user_invites(username);
        json invites = json::array();

        for (const auto &gid : group_ids) {
            string info_path = "groups/" + gid + "/info.json";
            if (filesystem::exists(info_path)) {
                ifstream fin(info_path);
                json info;
                fin >> info;
                fin.close();
                invites.push_back({
                    {"group_id", gid},
                    {"group_name", info.value("name", "")}
                });
            }
        }

        crow::response res(200, invites.dump());
        res.add_header("Content-Type", "application/json");
        return res;
    });

    CROW_ROUTE(app, "/api/accept_invite").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);

        if (username.empty())
            return crow::response(401, json({{"error", "Unauthorized"}}).dump());

        if (accept_invite(group_id, username)) {
            return crow::response(200, json({{"success", true}}).dump());
        } else {
            return crow::response(400, json({{"error", "Invite not found"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/reject_invite").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);

        if (username.empty())
            return crow::response(401, json({{"error", "Unauthorized"}}).dump());

        if (reject_invite(group_id, username)) {
            return crow::response(200, json({{"success", true}}).dump());
        } else {
            return crow::response(400, json({{"error", "Invite not found"}}).dump());
        }
    });

    CROW_ROUTE(app, "/api/group_info").methods("GET"_method)([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty()) {
            return crow::response(401, "Unauthorized");
        }

        string group_id = req.url_params.get("id") ? req.url_params.get("id") : "";
        if (group_id.empty()) {
            return crow::response(400, "Missing group id");
        }

        string info_path = "groups/" + group_id + "/info.json";
        string members_path = "groups/" + group_id + "/members.json";
        string events_path = "groups/" + group_id + "/events.json";

        if (!filesystem::exists(info_path)) {
            return crow::response(404, "Group not found");
        }

        // Read info
        json info;
        ifstream f_info(info_path);
        if (f_info.is_open()) {
            f_info >> info;
            f_info.close();
        }

        // Read members
        json members = json::array();
        if (filesystem::exists(members_path)) {
            ifstream f_mem(members_path);
            if (f_mem.is_open()) {
                f_mem >> members;
                f_mem.close();
            }
        }

        // Count events
        int event_count = 0;
        if (filesystem::exists(events_path)) {
            ifstream f_ev(events_path);
            json events;
            if (f_ev.is_open()) {
                f_ev >> events;
                f_ev.close();
            }
            if (events.is_array())
                event_count = events.size();
        }

        // Get subgroups (nested structure)
        json subgroups_raw = json::object();
        if (info.contains("subgroups")) {
            subgroups_raw = info["subgroups"];
        }

        // Get creator (first member)
        string creator = "";
        if (members.is_array() && !members.empty()) {
            creator = members[0].get<string>();
        }

        // Get join requests (only for creator)
        json join_requests = json::array();
        if (creator == user) {
            vector<string> requests = get_join_requests(group_id);
            for (const auto &req_user : requests) {
                join_requests.push_back(req_user);
            }
        }

        json result = {
            {"id", info.value("id", group_id)},
            {"name", info.value("name", "")},
            {"members", members},
            {"member_count", members.is_array() ? (int)members.size() : 0},
            {"event_count", event_count},
            {"subgroups", subgroups_raw},
            {"creator", creator},
            {"max_people", info.value("max_people", 0)},
            {"is_private", info.value("is_private", false)},
            {"join_requests", join_requests}};

        crow::response res(200, result.dump());
        res.add_header("Content-Type", "application/json");
        return res;
    });

    CROW_ROUTE(app, "/api/terminal").methods("POST"_method)([](const crow::request &req) {
        string command = urlDecode(getParam(req.body, "command"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Unauthorized");

        // Odtwarzanie grupy z ciasteczka
        string currentgroup = "";
        size_t pos = cookie_header.find("TerminalGroup=");
        if (pos != string::npos) {
            size_t start = pos + 14; // długość słowa "TerminalGroup="
            size_t end = cookie_header.find(';', start);
            currentgroup = cookie_header.substr(start, end - start);
        }

        string original_group = currentgroup;
        string output = process_terminal_command(command, user, currentgroup);

        string group_name = get_group_name(currentgroup);
        json res_json = {
            {"output", output},
            {"currentGroup", currentgroup},
            {"groupName", group_name}};
        crow::response res(200, res_json.dump());

        // Jeśli komenda 'cd' zmieniła grupę, aktualizujemy ciasteczko
        if (currentgroup != original_group) {
            res.add_header("Set-Cookie", "TerminalGroup=" + currentgroup + "; Path=/");
        }

        return res;
    });

    CROW_ROUTE(app, "/api/sync_google").methods("POST"_method)([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Unauthorized");

        int imported = syncGoogleEvents(user);

        json res_json;
        if (imported >= 0) {
            res_json = {
                {"success", true},
                {"imported", imported},
                {"message", "Successfully synced " + to_string(imported) + " events from Google Calendar"}
            };
        } else {
            res_json = {
                {"success", false},
                {"message", "Failed to sync Google Calendar events"}
            };
        }

        crow::response res(200, res_json.dump());
        res.add_header("Content-Type", "application/json");
        return res;
    });

    // Przechwytujemy api_key przez referencję za pomocą [&]
    CROW_ROUTE(app, "/api/debug_models").methods("GET"_method)([&]() {
        string response_data;
        string url = "https://generativelanguage.googleapis.com/v1/models?key=" + api_key;

        CURL *curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);
            curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
        return response_data;
    });

    // CROW_ROUTE(app, "/api/generate_command").methods("POST"_method)[&](const crow::request &req) {
    CROW_ROUTE(app, "/api/generate_command").methods("POST"_method)([api_key](const crow::request &req) {
        crow::response res;
        res.add_header("Content-Type", "application/json");

        string prompt = urlDecode(getParam(req.body, "prompt"));
        if (prompt.empty()) {
            res.code = 400;
            res.body = json({{"error", "Prompt cannot be empty"}}).dump();
            return res;
        }

        string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent";

        json openai_request;
        string sys_prompt = "Create a single Linux/Bash terminal command for the following request. Return ONLY the command as plain text. Do not use markdown, no explanations, no comments. The available commands are:\n\n"
                           "- cat YYYY-MM-DD : show events for a specific date\n"
                           "- cat N : show next N events and store dates\n"
                           "- grep \"text\" : find events containing text in title or description\n"
                           "- touch \"event name\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup] [O=origin]\n"
                           "- touch \"event name\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup] [O=origin]\n"
                           "- cd <group_name_or_id> : change to a specific group\n"
                           "- cd ~ or cd private : change to private calendar\n"
                           "- dates : show stored dates from searches\n"
                           "- $DATE or $DATE[n] : use stored dates in commands (n=index, 0=first)\n\n"
                           "Request: " + prompt;

        openai_request["contents"][0]["parts"][0]["text"] = sys_prompt;

        string request_data = openai_request.dump();
        string response_data;

        CURL *curl = curl_easy_init();
        if (curl) {
            struct curl_slist *headers = nullptr;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            headers = curl_slist_append(headers, ("X-goog-api-key: " + api_key).c_str());

            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_data.size());

            // Odkomentowane poprawki: przekazanie nagłówków
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

            // Odkomentowane poprawki: callback i zmienna do zapisu odpowiedzi
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

            cerr << "REQUEST:\n"
                 << request_data << endl;
            cerr << "API KEY = [" << api_key << "]" << endl;
            cerr << "LEN = " << api_key.size() << endl;
            CURLcode cres = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            curl_slist_free_all(headers);

            if (cres != CURLE_OK) {
                res.code = 500;
                res.body = json({{"error", "Failed to contact AI API"}}).dump();
                return res;
            }
        }

        // Dodane wypisywanie odpowiedzi przed próbą jej parsowania
        cerr << "RESPONSE:\n"
             << response_data << endl;

        try {
            json google_json = json::parse(response_data);

            if (google_json.contains("error")) {
                cerr << "GOOGLE API ERROR: " << google_json["error"].dump() << endl;
                res.code = 500;
                res.body = json({{"error", "API Error: " + google_json["error"]["message"].get<string>()}}).dump();
                return res;
            }

            string generated_command = google_json["candidates"][0]["content"]["parts"][0]["text"];

            if (not generated_command.empty() and generated_command.back() == '\n') {
                generated_command.pop_back();
            }

            res.code = 200;
            res.body = json({{"command", generated_command}}).dump();
            return res;

        } catch (const exception &e) {
            cerr << "PARSING ERROR: " << e.what() << endl;
            cerr << "RAW RESPONSE: " << response_data << endl;

            res.code = 500;
            res.body = json({{"error", "Błąd parsowania: " + string(e.what())}}).dump();
            return res;
        }
    });

    // New AI chat endpoint
    CROW_ROUTE(app, "/api/ai_chat").methods("POST"_method)([api_key](const crow::request &req) {
        crow::response res;
        res.add_header("Content-Type", "application/json");

        try {
            json request_json = json::parse(req.body);
            string message = request_json["message"].get<string>();
            json history = request_json.value("history", json::array());

            // Get user and group from cookie
            string cookie_header = req.get_header_value("Cookie");
            string user = get_logged_in_user(cookie_header);
            string currentgroup = "";
            size_t pos = cookie_header.find("TerminalGroup=");
            if (pos != string::npos) {
                size_t start = pos + 14;
                size_t end = cookie_header.find(';', start);
                currentgroup = cookie_header.substr(start, end - start);
            }

            if (message.empty()) {
                res.code = 400;
                res.body = json({{"error", "Message cannot be empty"}}).dump();
                return res;
            }

            // Check if AI needs to execute a command
            bool shouldExecuteCommand = false;
            string commandToExecute = "";

            // First, check if the message is asking for information that requires a command
            vector<string> infoKeywords = {"show", "find", "list", "what", "when", "get", "search", "next", "upcoming"};
            string lowerMessage = message;
            transform(lowerMessage.begin(), lowerMessage.end(), lowerMessage.begin(), ::tolower);

            for (const auto &keyword : infoKeywords) {
                if (lowerMessage.find(keyword) != string::npos) {
                    shouldExecuteCommand = true;
                    break;
                }
            }

            // If AI should get information, generate and execute a command
            json commandExecutions = json::array();
            if (shouldExecuteCommand && !user.empty()) {
                // Generate a command using AI
                string cmdPrompt = "Based on this user request, generate the appropriate terminal command: " + message;

                string cmdUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent";
                json cmd_request;
                string cmd_sys = "Create a single Linux/Bash terminal command for the following request. Return ONLY the command as plain text. Do not use markdown, no explanations, no comments. The available commands are:\n\n"
                                 "- cat YYYY-MM-DD : show events for a specific date\n"
                                 "- cat N : show next N events and store dates in vector\n"
                                 "- grep \"text\" : find events containing text in title or description\n"
                                 "- touch \"event name\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup] [O=origin]\n"
                                 "- dates : show stored dates from searches\n"
                                 "- $DATE or $DATE[n] : use stored dates in commands (n=index)\n\n"
                                 "Request: " +
                                 message;

                cmd_request["contents"][0]["parts"][0]["text"] = cmd_sys;
                string cmd_request_data = cmd_request.dump();
                string cmd_response_data;

                CURL *cmd_curl = curl_easy_init();
                if (cmd_curl) {
                    struct curl_slist *cmd_headers = nullptr;
                    cmd_headers = curl_slist_append(cmd_headers, "Content-Type: application/json");
                    cmd_headers = curl_slist_append(cmd_headers, ("X-goog-api-key: " + api_key).c_str());

                    curl_easy_setopt(cmd_curl, CURLOPT_URL, cmdUrl.c_str());
                    curl_easy_setopt(cmd_curl, CURLOPT_POST, 1L);
                    curl_easy_setopt(cmd_curl, CURLOPT_POSTFIELDS, cmd_request_data.c_str());
                    curl_easy_setopt(cmd_curl, CURLOPT_POSTFIELDSIZE, cmd_request_data.size());
                    curl_easy_setopt(cmd_curl, CURLOPT_HTTPHEADER, cmd_headers);
                    curl_easy_setopt(cmd_curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                    curl_easy_setopt(cmd_curl, CURLOPT_WRITEDATA, &cmd_response_data);

                    CURLcode cmd_res = curl_easy_perform(cmd_curl);
                    curl_easy_cleanup(cmd_curl);
                    curl_slist_free_all(cmd_headers);

                    if (cmd_res == CURLE_OK) {
                        try {
                            json cmd_json = json::parse(cmd_response_data);
                            if (!cmd_json.contains("error")) {
                                commandToExecute = cmd_json["candidates"][0]["content"]["parts"][0]["text"];
                                if (!commandToExecute.empty() && commandToExecute.back() == '\n') {
                                    commandToExecute.pop_back();
                                }

                                // Execute the command
                                string commandOutput = process_terminal_command(commandToExecute, user, currentgroup);
                                commandExecutions.push_back({{"command", commandToExecute}, {"output", commandOutput}});
                            }
                        } catch (...) {
                        }
                    }
                }
            }

            string url = "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent";

            json gemini_request;
            json &contents = gemini_request["contents"];

            string system_context = "You are a helpful AI assistant for a calendar app called Calang. "
                                    "Be concise, friendly, and helpful. Keep responses under 200 characters when possible. "
                                    "You can help with: general questions, calendar tips, explaining features, and casual conversation. "
                                    "You can execute terminal commands to get information about events. When you execute a command, its output will be provided to you.";

            contents = json::array();
            contents.push_back({{"role", "user"},
                                {"parts", {{{"text", system_context}}}}});

            for (const auto &msg : history) {
                string role = msg["role"].get<string>();
                string content = msg["content"].get<string>();

                if (role == "user") {
                    contents.push_back({{"role", "user"},
                                        {"parts", {{{"text", content}}}}});
                } else if (role == "assistant") {
                    contents.push_back({{"role", "model"},
                                        {"parts", {{{"text", content}}}}});
                }
            }

            // Add command execution results to the context if any
            string finalMessage = message;
            if (!commandExecutions.empty()) {
                finalMessage += "\n\n[System: I executed these commands for you]\n";
                for (const auto &exec : commandExecutions) {
                    finalMessage += "Command: " + exec["command"].get<string>() + "\n";
                    finalMessage += "Output:\n" + exec["output"].get<string>() + "\n\n";
                }
            }

            contents.push_back({{"role", "user"},
                                {"parts", {{{"text", finalMessage}}}}});

            string request_data = gemini_request.dump();
            string response_data;

            CURL *curl = curl_easy_init();
            if (curl) {
                struct curl_slist *headers = nullptr;
                headers = curl_slist_append(headers, "Content-Type: application/json");
                headers = curl_slist_append(headers, ("X-goog-api-key: " + api_key).c_str());

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_POST, 1L);
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
                curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_data.size());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

                CURLcode cres = curl_easy_perform(curl);
                curl_easy_cleanup(curl);
                curl_slist_free_all(headers);

                if (cres != CURLE_OK) {
                    res.code = 500;
                    res.body = json({{"error", "Failed to contact AI API"}}).dump();
                    return res;
                }
            }

            json gemini_response = json::parse(response_data);

            if (gemini_response.contains("error")) {
                res.code = 500;
                res.body = json({{"error", "AI API Error: " + gemini_response["error"]["message"].get<string>()}}).dump();
                return res;
            }

            string ai_response = gemini_response["candidates"][0]["content"]["parts"][0]["text"];

            if (!ai_response.empty() && ai_response.back() == '\n') {
                ai_response.pop_back();
            }

            // Also provide the generated command as a suggestion if we executed one
            json response_json = {{"response", ai_response}, {"commands", commandExecutions}};
            if (!commandToExecute.empty()) {
                response_json["command"] = commandToExecute;
            }

            res.code = 200;
            res.body = response_json.dump();
            return res;

        } catch (const exception &e) {
            cerr << "AI Chat Error: " << e.what() << endl;
            res.code = 500;
            res.body = json({{"error", string(e.what())}}).dump();
            return res;
        }
    });
}
