#include "google_oauth.h"
#include "cookies.h"
#include "functions.h"
#include <crow/app.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <random>
#include <sstream>
#include <fstream>
#include <filesystem>

using json = nlohmann::json;
using namespace std;

extern unordered_map<string, string> active_sessions;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

string url_encode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char)c);
        escaped << nouppercase;
    }

    return escaped.str();
}

string generate_state_token() {
    const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    string token = "";
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, chars.size() - 1);
    for (int i = 0; i < 32; i++) {
        token += chars[dis(gen)];
    }
    return token;
}

string exchange_code_for_token(const string &code, const string &client_id,
                                const string &client_secret, const string &redirect_uri) {
    CURL *curl;
    CURLcode res;
    string response_string;

    curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    string post_fields = "code=" + url_encode(code) +
                        "&client_id=" + url_encode(client_id) +
                        "&client_secret=" + url_encode(client_secret) +
                        "&redirect_uri=" + url_encode(redirect_uri) +
                        "&grant_type=authorization_code";

    curl_easy_setopt(curl, CURLOPT_URL, "https://oauth2.googleapis.com/token");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response_string;
}

string get_user_info(const string &access_token) {
    CURL *curl;
    CURLcode res;
    string response_string;

    curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    string auth_header = "Authorization: Bearer " + access_token;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, "https://www.googleapis.com/oauth2/v2/userinfo");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return "";
    }

    return response_string;
}

bool create_user_if_not_exists(const string &email) {
    if (!filesystem::exists("users.json")) {
        json users = json::object();
        ofstream fout("users.json");
        fout << users.dump(4);
        fout.close();
    }

    ifstream fin("users.json");
    json users;
    fin >> users;
    fin.close();

    if (users.contains(email)) {
        return true;
    }

    string temp_password = generate_session_id();
    users[email] = temp_password;

    ofstream fout("users.json");
    fout << users.dump(4);
    fout.close();

    string user_dir = "users/" + email;
    if (!filesystem::exists(user_dir)) {
        filesystem::create_directories(user_dir);
    }

    string events_file = user_dir + "/events.json";
    if (!filesystem::exists(events_file)) {
        json empty_events = json::array();
        ofstream events_out(events_file);
        events_out << empty_events.dump(4);
        events_out.close();
    }

    return true;
}

void register_google_oauth_routes(crow::SimpleApp &app) {
    CROW_ROUTE(app, "/auth/google/login")([]() {
        const char *client_id = getenv("GOOGLE_CLIENT_ID");
        const char *redirect_uri = getenv("GOOGLE_REDIRECT_URI");

        if (!client_id || !redirect_uri) {
            crow::response res("OAuth configuration error: Missing environment variables");
            res.code = 500;
            return res;
        }

        string state = generate_state_token();

        string auth_url = "https://accounts.google.com/o/oauth2/v2/auth?"
                         "client_id=" + url_encode(client_id) +
                         "&redirect_uri=" + url_encode(redirect_uri) +
                         "&response_type=code" +
                         "&scope=" + url_encode("openid email profile") +
                         "&state=" + state;

        crow::response res;
        res.code = 302;
        res.add_header("Location", auth_url);
        res.add_header("Set-Cookie", "oauth_state=" + state + "; Path=/; HttpOnly; Max-Age=600");
        return res;
    });

    CROW_ROUTE(app, "/auth/google/callback")([](const crow::request &req) {
        auto params = req.url_params;
        string code = params.get("code") ? params.get("code") : "";
        string state = params.get("state") ? params.get("state") : "";
        string error = params.get("error") ? params.get("error") : "";

        if (!error.empty()) {
            crow::response res("Authentication failed: " + error);
            res.code = 400;
            return res;
        }

        if (code.empty()) {
            crow::response res("Authentication failed: No authorization code received");
            res.code = 400;
            return res;
        }

        string cookie_header = req.get_header_value("Cookie");
        size_t pos = cookie_header.find("oauth_state=");
        string stored_state = "";
        if (pos != string::npos) {
            size_t start = pos + 12;
            size_t end = cookie_header.find(";", start);
            stored_state = cookie_header.substr(start, end - start);
        }

        if (state.empty() || stored_state.empty() || state != stored_state) {
            crow::response res("Authentication failed: Invalid state parameter");
            res.code = 400;
            return res;
        }

        const char *client_id = getenv("GOOGLE_CLIENT_ID");
        const char *client_secret = getenv("GOOGLE_CLIENT_SECRET");
        const char *redirect_uri = getenv("GOOGLE_REDIRECT_URI");

        if (!client_id || !client_secret || !redirect_uri) {
            crow::response res("OAuth configuration error: Missing environment variables");
            res.code = 500;
            return res;
        }

        string token_response = exchange_code_for_token(code, client_id, client_secret, redirect_uri);
        if (token_response.empty()) {
            crow::response res("Authentication failed: Could not exchange code for token");
            res.code = 500;
            return res;
        }

        json token_data;
        try {
            token_data = json::parse(token_response);
        } catch (...) {
            crow::response res("Authentication failed: Invalid token response");
            res.code = 500;
            return res;
        }

        if (!token_data.contains("access_token")) {
            crow::response res("Authentication failed: No access token in response");
            res.code = 500;
            return res;
        }

        string access_token = token_data["access_token"];
        string user_info_response = get_user_info(access_token);

        if (user_info_response.empty()) {
            crow::response res("Authentication failed: Could not retrieve user information");
            res.code = 500;
            return res;
        }

        json user_info;
        try {
            user_info = json::parse(user_info_response);
        } catch (...) {
            crow::response res("Authentication failed: Invalid user info response");
            res.code = 500;
            return res;
        }

        if (!user_info.contains("email")) {
            crow::response res("Authentication failed: No email in user information");
            res.code = 500;
            return res;
        }

        string email = user_info["email"];

        if (!create_user_if_not_exists(email)) {
            crow::response res("Authentication failed: Could not create user account");
            res.code = 500;
            return res;
        }

        string session_id = generate_session_id();
        active_sessions[session_id] = email;

        string response_text = "Zalogowano pomyślnie przez Google! Trwa przekierowywanie... <meta http-equiv=\"refresh\" content=\"0;url=/dashboard\">";
        crow::response res(response_text);
        res.add_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
        res.add_header("Set-Cookie", "oauth_state=; Path=/; Max-Age=0");
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });
}
