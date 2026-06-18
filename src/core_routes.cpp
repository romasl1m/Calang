#include <crow/app.h>
#include <fstream>
#include <string>
#include <sstream>
#include "functions.h"
#include "cookies.h"
using namespace std;

void core_routes(crow::SimpleApp &app) {
    CROW_ROUTE(app, "/")([]() {
        string html = loadHtmlTemplate("templates/index.html");
        crow::response res(html);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/static/<string>")([](const crow::request &req, const std::string &filename) {
        std::string path = "static/" + filename;
        std::ifstream in(path, std::ios::binary);
        if (!in)
            return crow::response(404);
        std::ostringstream ss;
        ss << in.rdbuf();
        crow::response res;
        res.body = ss.str();

        if (path.find(".png") != std::string::npos)
            res.add_header("Content-Type", "image/png");
        else if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos)
            res.add_header("Content-Type", "image/jpeg");
        else
            res.add_header("Content-Type", "application/octet-stream");
        return res;
    });

    CROW_ROUTE(app, "/join/<string>")([](const crow::request &req, const std::string &group_id) {
        // Check if user is logged in
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);

        if (username.empty()) {
            // Redirect to login with return URL
            crow::response res(302);
            res.add_header("Location", "/login?return=/join/" + group_id);
            return res;
        }

        // Redirect to dashboard with auto-join parameter
        crow::response res(302);
        res.add_header("Location", "/dashboard?join=" + group_id);
        return res;
    });
}
