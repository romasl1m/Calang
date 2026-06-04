#include <crow/app.h>
#include <string>
#include <curl/curl.h>
#include "terminal.h"
#include <filesystem>
#include <fstream>
#include "functions.h"
#include "cookies.h"
#include <nlohmann/json.hpp>

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
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Nie zalogowano");

        edit_event(title, id, start, end, user, description, origin, recurrence, edit_all, priority);

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
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Unauthorized");

        string generated_id;
        create_group(name, user, generated_id);

        return crow::response(200, "Group created");
    });

    CROW_ROUTE(app, "/api/join_group").methods("POST"_method)([](const crow::request &req) {
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if (user.empty())
            return crow::response(401, "Unauthorized");

        string path = "groups/" + group_id + "/members.json";
        if (!filesystem::exists(path)) {
            return crow::response(404, "Group not found");
        }

        json members;
        ifstream fin(path);
        if (fin.is_open())
            fin >> members;
        fin.close();

        for (const auto &m : members) {
            if (m == user)
                return crow::response(200, "Already a member");
        }

        members.push_back(user);
        ofstream fout(path);
        fout << members.dump(4);
        fout.close();

        return crow::response(200, "Joined successfully");
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
        string sys_prompt = "Create a single Linux/Bash terminal command for the following request. Return ONLY the command as plain text. Do not use markdown, no explanations, no comments. The available commands are: help, whoami, clear, touch \"event name\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup] [O=origin] (default origin is private). Request: " + prompt;

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
}
