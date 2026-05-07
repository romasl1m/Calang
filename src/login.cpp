#include <crow.h>
#include <fstream>
#include <linux/limits.h>
#include <string>
#include <random>
#include "event.cpp"
using namespace std;

bool user_exists(const string &username){
    ifstream fin("users.txt");
    string line = "";
    while(getline(fin, line)){
        size_t delim = line.find(":");
        if(delim != string::npos){
            string u = line.substr(0,delim);
            if(u == username){
                return true;
            }
        }
    }
    fin.close();
    return false;
}

string loadHtmlTemplate(const string& filePath){
    ifstream file(filePath);
    if(not file.is_open()){
        return "<html><body>Błąd: Nie znaleziono pliku " + filePath + "</body></html>";
    }
    stringstream fout;
    fout << file.rdbuf();
    return fout.str();
}

bool password_correct(const string &username, const string &password){
    ifstream fin("users.txt");
    string line = "";
    while(getline(fin, line)){
        size_t delim = line.find(":");
        if(delim != string::npos){
            string u = line.substr(0,delim);
            string p = line.substr(delim + 1);
            if(u == username){
                if(p == password) return true;
                return false;
            }
        }
    }
    fin.close();
    return false;
}

void add_new_event(const string& title, const string& id, const string& start, const string& end, const string& user, const string& description){
    string usrdir = "users/" + user; 
    string directory = usrdir + "/events.txt";
    if(not filesystem::exists(usrdir)){
        filesystem::create_directories(usrdir);
    }
    ofstream fout(directory, ios::app);        
    if(fout.is_open()){
        Event newevent(title, id, start, end, user, description);
        fout << newevent.serialize() << "\n";
        fout.close();
    }
}
//COOKIES
unordered_map<string,string> active_sessions;

string generate_session_id(){
    const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    string session_id = "";
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0,chars.size() - 1);
    for(int i = 0; i < 32; i++){
        session_id += chars[dis(gen)];
    }
    return session_id;
}
string get_logged_in_user(const string& cookie_header){
    size_t pos = cookie_header.find("session_id=");
    if(pos != string::npos){
        size_t start = pos + 11;
        size_t end = cookie_header.find(";", start);
        string session_id = cookie_header.substr(start, end - start);
        if(active_sessions.find(session_id) != active_sessions.end()){
            return active_sessions[session_id];
        }
    }
    return ""; // Pusty string oznacza, że nikt nie jest zalogowany
}
//END OF COOKIES
string getParam(const string& body, const string& key){
    size_t pos = body.find(key + "=");
    if(pos == string::npos){
        return "";
    }
    size_t start = pos + key.length() + 1;
    size_t end = body.find("&", start);
    if(end == string::npos){
        end = body.length();
    }
    return body.substr(start, end - start);
}

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([]()
    {
        string html = loadHtmlTemplate("templates/route.html");
        crow::response res(html);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/register_page")([](){
        string html = loadHtmlTemplate("templates/regpage.html");
        crow::response res(html);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/dashboard").methods("GET"_method)([](const crow::request& req){
        string  cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);

        if(username == ""){
            crow::response res;
            res.code = 302;
            res.add_header("Location", "/");
            return res;
        }

        string html = loadHtmlTemplate("templates/dashboard.html");
        size_t usrname = html.find("{{username}}");
        
        if(usrname != string::npos){
            html.replace(usrname,12, username);
        }
        crow::response res(html);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

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

    CROW_ROUTE(app, "/api/new_event").methods("POST"_method)([](const crow::request& req){
        string title = getParam(req.body, "title");
        string id = generate_session_id();
        string start = getParam(req.body, "start");
        string end = getParam(req.body, "end");
        string description = getParam(req.body, "description");

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
    app.port(8080).multithreaded().run();
    // app.bindaddr("0.0.0.0").port(8080).run();
    return 0;
}
