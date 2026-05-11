#include <crow.h>
#include <string>
#include <unordered_map>
#include "functions.h"
#include "cookies.h"
using namespace std;
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
        string response_text;
        if(user_found){
            string error_msg = "Użytkownik o takim loginie już istnieje.";
            string html = loadHtmlTemplate("templates/errsignin.html");
            
            size_t pos_user = html.find("{{username}}");
            if(pos_user != string::npos){
                html.replace(pos_user, 12, user);
            }

            size_t pos = html.find("{{error_msg}}");
            if(pos != string::npos){
                html.replace(pos, 13, error_msg);
            }

            crow::response res(html);
            res.add_header("Content-Type", "text/html; charset=utf-8");
            return res;        
        }else{
            ofstream fout("users.txt", ios::app);
            if(fout.is_open()){
                fout << user << ":" << pass << "\n";
                fout.close();

                string session_id = generate_session_id();
                active_sessions[session_id] = user;
                response_text = "Użytkownik został utworzony! Trwa przekierowywanie... <meta http-equiv=\"refresh\" content=\"0;url=/dashboard\">";
                crow::response res(response_text);
                res.add_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
                res.add_header("Content-Type", "text/html; charset=utf-8");
                return res;
            }else{
                response_text = "Błąd: Nie można zapisać danych użytkownika. <br><br> <a href='/register_page'><button>Wróć do rejestracji</button></a>";
                crow::response res(response_text);
                res.add_header("Content-Type", "text/html; charset=utf-8");
                return res;
            }
        }
        crow::response res(response_text);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });
}
