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
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(
        headers,
        "Content-Type: application/x-www-form-urlencoded");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
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
    curl_slist_free_all(headers);
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
    // struct curl_slist *headers = nullptr;
    // headers = curl_slist_append(
    //     headers,
    //     "Content-Type: application/x-www-form-urlencoded");
    //
    // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
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

string createGoogleCalendarEvent(const string &access_token,
                                  const string &title,
                                  const string &start_datetime,
                                  const string &end_datetime,
                                  const string &description) {
    CURL *curl;
    CURLcode res;
    string response_string;

    curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    // Convert datetime format from "YYYY-MM-DD HH:MM" to RFC3339 "YYYY-MM-DDTHH:MM:00Z"
    string start_rfc3339 = start_datetime;
    string end_rfc3339 = end_datetime;

    // Replace space with 'T' and add seconds + timezone
    if (start_rfc3339.length() >= 16) {
        start_rfc3339[10] = 'T';
        start_rfc3339 += ":00Z";
    }
    if (end_rfc3339.length() >= 16) {
        end_rfc3339[10] = 'T';
        end_rfc3339 += ":00Z";
    }

    // Build JSON request body
    json event_data = {
        {"summary", title},
        {"description", description},
        {"start", {{"dateTime", start_rfc3339}, {"timeZone", "UTC"}}},
        {"end", {{"dateTime", end_rfc3339}, {"timeZone", "UTC"}}}
    };

    string post_data = event_data.dump();

    // Set up headers
    string auth_header = "Authorization: Bearer " + access_token;
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header.c_str());

    // Make request to Google Calendar API v3
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.googleapis.com/calendar/v3/calendars/primary/events");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        cerr << "Calendar API error: " << curl_easy_strerror(res) << endl;
        return "";
    }

    return response_string;
}

string get_user_access_token(const string &email) {
    string user_dir = "users/" + email;
    string token_file = user_dir + "/google_token.json";

    if (!filesystem::exists(token_file)) {
        return "";
    }

    ifstream fin(token_file);
    if (!fin.is_open()) {
        return "";
    }

    json token_data;
    try {
        fin >> token_data;
        fin.close();

        if (token_data.contains("access_token")) {
            return token_data["access_token"];
        }
    } catch (...) {
        return "";
    }

    return "";
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
            cerr << "GOOGLE_CLIENT_ID="
                 << (getenv("GOOGLE_CLIENT_ID") ? "FOUND" : "NULL") << endl;

            cerr << "GOOGLE_CLIENT_SECRET="
                 << (getenv("GOOGLE_CLIENT_SECRET") ? "FOUND" : "NULL") << endl;

            cerr << "GOOGLE_REDIRECT_URI="
                 << (getenv("GOOGLE_REDIRECT_URI") ? "FOUND" : "NULL") << endl;
            crow::response res("OAuth configuration error: Missing environment variables");
            res.code = 500;
            return res;
        }

        string state = generate_state_token();

        string auth_url = "https://accounts.google.com/o/oauth2/v2/auth?"
                          "client_id=" +
                          url_encode(client_id) +
                          "&redirect_uri=" + url_encode(redirect_uri) +
                          "&response_type=code" +
                          "&scope=" + url_encode("openid email profile https://www.googleapis.com/auth/calendar") +
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
        cerr << "TOKEN RESPONSE:\n"
             << token_response << endl;
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

        // Store access token for the user
        string user_dir = "users/" + email;
        string token_file = user_dir + "/google_token.json";
        json token_storage = {
            {"access_token", access_token},
            {"refresh_token", token_data.value("refresh_token", "")},
            {"expires_in", token_data.value("expires_in", 3600)},
            {"timestamp", time(0)}
        };
        ofstream token_out(token_file);
        token_out << token_storage.dump(4);
        token_out.close();

        string session_id = generate_session_id();
        active_sessions[session_id] = email;

        string response_text = "Logged in using Google!  <meta http-equiv=\"refresh\" content=\"0;url=/dashboard\">";
        crow::response res(response_text);
        res.add_header("Set-Cookie", "session_id=" + session_id + "; Path=/; HttpOnly");
        res.add_header("Set-Cookie", "oauth_state=; Path=/; Max-Age=0");
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });
}
