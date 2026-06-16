#include "profile.h"
#include "functions.h"
#include "cookies.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <iostream>

using namespace std;
using json = nlohmann::json;

void profile_routes(crow::SimpleApp &app) {
    CROW_ROUTE(app, "/profile").methods("GET"_method)([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);

        if (username == "") {
            crow::response res;
            res.code = 302;
            res.add_header("Location", "/");
            return res;
        }

        string html = loadHtmlTemplate("templates/profile.html");
        
        string name = username;
        string profile_picture = ""; // Or some default base64 if needed. We'll handle it in JS or template.

        string profile_path = "users/" + username + "/profile.json";
        if (filesystem::exists(profile_path)) {
            ifstream fin(profile_path);
            json p;
            fin >> p;
            fin.close();
            name = p.value("name", username);
            profile_picture = p.value("profile_picture", "");
        }

        auto replace_tag = [&](string tag, string val) {
            size_t pos;
            while ((pos = html.find(tag)) != string::npos) {
                html.replace(pos, tag.length(), val);
            }
        };

        replace_tag("{{username}}", username);
        replace_tag("{{name}}", name);
        replace_tag("{{profile_picture}}", profile_picture);

        crow::response res(html);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/api/profile/update").methods("POST"_method)([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string current_username = get_logged_in_user(cookie_header);

        if (current_username == "") {
            return crow::response(401, "Unauthorized");
        }

        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded()) {
            return crow::response(400, "Invalid JSON");
        }

        string new_name = body.value("name", "");
        string new_username = body.value("username", "");
        string old_username = current_username;

        if (new_username != current_username && new_username != "") {
            if (user_exists(new_username)) {
                return crow::response(400, "Username already exists");
            }

            ifstream fin("users.json");
            json users;
            if (fin.is_open()) {
                fin >> users;
                fin.close();
            }

            if (users.contains(current_username)) {
                string pwd = users[current_username];
                users.erase(current_username);
                users[new_username] = pwd;

                ofstream fout("users.json");
                fout << users.dump(4);
                fout.close();
            }

            if (filesystem::exists("users/" + current_username)) {
                filesystem::rename("users/" + current_username, "users/" + new_username);
            } else {
                filesystem::create_directories("users/" + new_username);
            }

            string priv_events = "users/" + new_username + "/events.json";
            if (filesystem::exists(priv_events)) {
                ifstream fe(priv_events);
                json evs;
                fe >> evs;
                fe.close();
                bool changed = false;
                for (auto& e : evs) {
                    if (e.value("user", "") == current_username) {
                        e["user"] = new_username;
                        changed = true;
                    }
                }
                if (changed) {
                    ofstream fe_out(priv_events);
                    fe_out << evs.dump(4);
                }
            }

            if (filesystem::exists("groups")) {
                for (const auto &entry : filesystem::directory_iterator("groups")) {
                    string g_id = entry.path().filename().string();
                    string m_path = "groups/" + g_id + "/members.json";
                    if (filesystem::exists(m_path)) {
                        ifstream fm(m_path);
                        json mems;
                        fm >> mems;
                        fm.close();
                        bool changed = false;
                        for (auto& m : mems) {
                            if (m == current_username) {
                                m = new_username;
                                changed = true;
                            }
                        }
                        if (changed) {
                            ofstream fm_out(m_path);
                            fm_out << mems.dump(4);
                        }
                    }
                    string e_path = "groups/" + g_id + "/events.json";
                    if (filesystem::exists(e_path)) {
                        ifstream fe(e_path);
                        json evs;
                        fe >> evs;
                        fe.close();
                        bool changed = false;
                        for (auto& e : evs) {
                            if (e.value("user", "") == current_username) {
                                e["user"] = new_username;
                                changed = true;
                            }
                        }
                        if (changed) {
                            ofstream fe_out(e_path);
                            fe_out << evs.dump(4);
                        }
                    }
                }
            }

            current_username = new_username;
        }

        string profile_path = "users/" + current_username + "/profile.json";
        json p;
        if (filesystem::exists(profile_path)) {
            ifstream f(profile_path);
            f >> p;
            f.close();
        }
        if (new_name != "") {
            p["name"] = new_name;
        }
        ofstream f_out(profile_path);
        f_out << p.dump(4);
        f_out.close();

        crow::response res(200, "OK");
        if (new_username != "" && new_username != old_username) {
            res.add_header("Set-Cookie", "session=" + new_username + "; Path=/; HttpOnly");
        }
        return res;
    });

    CROW_ROUTE(app, "/api/profile/picture").methods("POST"_method)([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);
        if (username == "") return crow::response(401, "Unauthorized");

        auto body = json::parse(req.body, nullptr, false);
        if (body.is_discarded() || !body.contains("picture")) {
            return crow::response(400, "Invalid payload");
        }

        string picture_data = body["picture"];
        string profile_path = "users/" + username + "/profile.json";
        json p;
        if (filesystem::exists(profile_path)) {
            ifstream f(profile_path);
            f >> p;
            f.close();
        }
        p["profile_picture"] = picture_data;
        ofstream f_out(profile_path);
        f_out << p.dump(4);
        f_out.close();
        
        return crow::response(200, "OK");
    });

    CROW_ROUTE(app, "/api/profile/delete").methods("POST"_method)([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);
        if (username == "") return crow::response(401, "Unauthorized");

        if (filesystem::exists("users.json")) {
            ifstream fin("users.json");
            json users;
            fin >> users;
            fin.close();
            if (users.contains(username)) {
                users.erase(username);
                ofstream fout("users.json");
                fout << users.dump(4);
            }
        }

        if (filesystem::exists("users/" + username)) {
            filesystem::remove_all("users/" + username);
        }

        if (filesystem::exists("groups")) {
            for (const auto &entry : filesystem::directory_iterator("groups")) {
                string g_id = entry.path().filename().string();
                string m_path = "groups/" + g_id + "/members.json";
                if (filesystem::exists(m_path)) {
                    ifstream fm(m_path);
                    json mems;
                    fm >> mems;
                    fm.close();
                    json new_mems = json::array();
                    bool changed = false;
                    for (auto& m : mems) {
                        if (m != username) {
                            new_mems.push_back(m);
                        } else {
                            changed = true;
                        }
                    }
                    if (changed) {
                        ofstream fm_out(m_path);
                        fm_out << new_mems.dump(4);
                    }
                }
                string e_path = "groups/" + g_id + "/events.json";
                if (filesystem::exists(e_path)) {
                    ifstream fe(e_path);
                    json evs;
                    fe >> evs;
                    fe.close();
                    json new_evs = json::array();
                    bool changed = false;
                    for (auto& e : evs) {
                        if (e.value("user", "") != username) {
                            new_evs.push_back(e);
                        } else {
                            changed = true;
                        }
                    }
                    if (changed) {
                        ofstream fe_out(e_path);
                        fe_out << new_evs.dump(4);
                    }
                }
            }
        }

        crow::response res(200, "OK");
        res.add_header("Set-Cookie", "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
        return res;
    });
}
