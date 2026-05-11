#include <crow.h>
#include "functions.h"
#include "cookies.h"
void login_logout_routes(crow::SimpleApp& app){
    CROW_ROUTE(app, "/login").methods("POST"_method)([](const crow::request& req)
    {
        string user = getParam(req.body, "username");
        string pass = getParam(req.body, "password");
        ifstream fin("users.txt");
        bool user_found = user_exists(user), pass_correct = password_correct(user,pass);
        string response_text;

        if(user_found and pass_correct){
            string session_id = generate_session_id();
            active_sessions[session_id] = user;

            response_text = "Zalogowano pomyślnie! Trwa przekierowywanie... <meta http-equiv=\"refresh\" content=\"0;url=/dashboard\">";
            crow::response res(response_text);
            res.add_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
            res.add_header("Content-Type", "text/html; charset=utf-8");
            return res;
        }else{
            string error_msg = "";
            if(not user_found){
                error_msg = "Użytkownik nie istnieje.";
            }else{
                error_msg = "Podano nieprawidłowe hasło.";
            }
            string html = loadHtmlTemplate("templates/errlogin.html");

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
        }
        crow::response res(response_text);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });
    CROW_ROUTE(app, "/logout").methods("GET"_method)([](const crow::request& req){
        string cookie_header = req.get_header_value("Cookie");
        size_t pos = cookie_header.find("session_id=");
        if(pos != string::npos){
            size_t start = pos + 11;
            size_t end = cookie_header.find(";", start);
            string session_id = cookie_header.substr(start, end - start);
            active_sessions.erase(session_id);
        }
        crow::response res;
        res.code = 302;
        res.add_header("Location", "/");
        res.add_header("Set-Cookie", "session_id=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
        return res;
    });
}
