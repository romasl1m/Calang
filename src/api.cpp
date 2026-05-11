#include <crow.h>
#include <string>
#include "functions.h" 
#include "cookies.h"
using namespace std;

void api_routes(crow::SimpleApp& app){
    CROW_ROUTE(app, "/api/new_event").methods("POST"_method)([](const crow::request& req){
        string title = urlDecode(getParam(req.body, "title"));
        string id = generate_session_id();
        string start = urlDecode(getParam(req.body, "start"));
        string end = urlDecode(getParam(req.body, "end"));
        string description = urlDecode(getParam(req.body, "description"));

        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if(user.empty()){
            return crow::response(401, "Użytkownik nie zalogowany");
        }

        add_new_event(title, id, start, end, user, description);

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/api/delete_event").methods("POST"_method)([](const crow::request& req){
        string id   = urlDecode(getParam(req.body, "id"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if(user.empty())
            return crow::response(401, "Użytkownik nie zalogowany");

        delete_event(id, user);

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/api/edit_event").methods("POST"_method)([](const crow::request& req){
        string title = urlDecode(getParam(req.body, "title"));
        string id          = urlDecode(getParam(req.body, "id"));
        string start       = urlDecode(getParam(req.body, "start"));
        string end         = urlDecode(getParam(req.body, "end"));
        string description = urlDecode(getParam(req.body, "description"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if(user.empty())
            return crow::response(401, "Użytkownik nie zalogowany");

        edit_event(title, id, user, start, end, description);

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });
}
