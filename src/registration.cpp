#include <crow.h>
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include "functions.h"
#include "cookies.h"

using namespace std;
using json = nlohmann::json;

extern unordered_map<string, string> active_sessions;

void register_auth_routes(crow::SimpleApp& app){
    CROW_ROUTE(app, "/register_page")([](){
        string html = loadHtmlTemplate("templates/regpage.html");
        crow::response res(html);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/register").methods("POST"_method)([](const crow::request& req)
    {
        string user = getParam(req.body, "username");
        string pass = getParam(req.body, "password");
        
        bool user_found = user_exists(user);
        
        if(user_found){
            string error_msg = "Użytkownik o takim loginie już istnieje.";
            string html = loadHtmlTemplate("templates/errsignin.html");
            
            size_t pos_user = html.find("{{username}}");
            if(pos_user != string::npos) html.replace(pos_user, 12, user);

            size_t pos = html.find("{{error_msg}}");
            if(pos != string::npos) html.replace(pos, 13, error_msg);

            crow::response res(html);
            res.add_header("Content-Type", "text/html; charset=utf-8");
            return res;        
        }else{
            // Logika zapisu do users.json
            json users = json::object();
            if(filesystem::exists("users.json")){
                ifstream fin("users.json");
                if(fin.is_open()){
                    fin >> users;
                    fin.close();
                }
            }

            users[user] = pass; // Dodanie nowego użytkownika

            ofstream fout("users.json", ios::trunc);
            if(fout.is_open()){
                fout << users.dump(4);
                fout.close();

                string session_id = generate_session_id();
                active_sessions[session_id] = user;
                
                string response_text = "Użytkownik został utworzony! Trwa przekierowywanie... <meta http-equiv=\"refresh\" content=\"0;url=/dashboard\">";
                crow::response res(response_text);
                res.add_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
                res.add_header("Content-Type", "text/html; charset=utf-8");
                return res;
            }else{
                string response_text = "Błąd: Nie można zapisać danych użytkownika. <br><br> <a href='/register_page'><button>Wróć do rejestracji</button></a>";
                crow::response res(response_text);
                res.add_header("Content-Type", "text/html; charset=utf-8");
                return res;
            }
        }
    });
}
