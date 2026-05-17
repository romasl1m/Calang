#include <crow.h>
#include <string>
#include <filesystem>
#include <fstream>
#include "functions.h" 
#include "cookies.h"
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

void api_routes(crow::SimpleApp& app){
    CROW_ROUTE(app, "/api/new_event").methods("POST"_method)([](const crow::request& req){
        string title = urlDecode(getParam(req.body, "title"));
        string start = urlDecode(getParam(req.body, "start"));
        string end = urlDecode(getParam(req.body, "end"));
        string description = urlDecode(getParam(req.body, "description"));
        string origin = urlDecode(getParam(req.body, "origin"));
        if(origin.empty()) origin = "private";

        string id = to_string(time(0)) + "_" + to_string(rand()%1000);

        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if(user.empty()){
            return crow::response(401, "Użytkownik nie zalogowany");
        }

        add_new_event(title, id, start, end, user, description, origin);

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/api/delete_event").methods("POST"_method)([](const crow::request& req){
        string id = urlDecode(getParam(req.body, "id"));
        string origin = urlDecode(getParam(req.body, "origin")); 
        if(origin.empty()) origin = "private";

        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if(user.empty())
            return crow::response(401, "Użytkownik nie zalogowany");

        delete_event(id, user, origin);

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/api/edit_event").methods("POST"_method)([](const crow::request& req){
        string title = urlDecode(getParam(req.body, "title"));
        string id = urlDecode(getParam(req.body, "id"));
        string start = urlDecode(getParam(req.body, "start"));
        string end = urlDecode(getParam(req.body, "end"));
        string description = urlDecode(getParam(req.body, "description"));
        string origin = urlDecode(getParam(req.body, "origin"));
        if(origin.empty()) origin = "private";

        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if(user.empty()) return crow::response(401, "Nie zalogowano");

        delete_event(id, user, origin);
        add_new_event(title, id, start, end, user, description, origin);

        crow::response res;
        res.code = 302;
        res.add_header("Location", "/dashboard");
        return res;
    });

    CROW_ROUTE(app, "/api/my_groups")([](const crow::request& req){
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);
        
        if(user.empty()){
            return crow::response(401, "Unauthorized");
        }

        auto groups = get_user_groups(user);
        return crow::response(nlohmann::json(groups).dump());
    });

    CROW_ROUTE(app, "/api/create_group").methods("POST"_method)([](const crow::request& req){
        string name = urlDecode(getParam(req.body, "name"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if(user.empty()) return crow::response(401, "Unauthorized");

        string generated_id;
        create_group(name, user, generated_id);

        return crow::response(200, "Group created");
    });

    // Single, merged /api/join_group: validates group exists, avoids duplicates, returns proper codes
    CROW_ROUTE(app, "/api/join_group").methods("POST"_method)([](const crow::request& req){
        string group_id = urlDecode(getParam(req.body, "group_id"));
        string cookie_header = req.get_header_value("Cookie");
        string user = get_logged_in_user(cookie_header);

        if(user.empty()) return crow::response(401, "Unauthorized");

        string path = "groups/" + group_id + "/members.json";
        if(!filesystem::exists(path)){
            return crow::response(404, "Group not found");
        }

        json members;
        ifstream fin(path);
        if(fin.is_open()) fin >> members;
        fin.close();

        for(const auto& m : members){
            if(m == user) return crow::response(200, "Already a member");
        }

        members.push_back(user);
        ofstream fout(path);
        fout << members.dump(4);
        fout.close();

        return crow::response(200, "Joined successfully");
    });
}
