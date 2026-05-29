#include <crow/app.h>
#include "functions.h"
#include "cookies.h"
string escape_html(const string &text) {
    string escaped;
    escaped.reserve(text.size());
    for (char c : text) {
        if (c == '"')
            escaped += "&quot;";
        else if (c == '\'')
            escaped += "&#39;";
        else if (c == '<')
            escaped += "&lt;";
        else if (c == '>')
            escaped += "&gt;";
        else if (c == '&')
            escaped += "&amp;";
        else
            escaped += c;
    }
    return escaped;
}
void dashboard(crow::SimpleApp &app) {
    CROW_ROUTE(app, "/dashboard").methods("GET"_method)([](const crow::request &req) {
        string cookie_header = req.get_header_value("Cookie");
        string username = get_logged_in_user(cookie_header);

        if (username == "") {
            crow::response res;
            res.code = 302;
            res.add_header("Location", "/");
            return res;
        }

        string date_param = req.url_params.get("date") ? req.url_params.get("date") : "";

        time_t now = time(0);
        tm *ltm = localtime(&now);

        if (!date_param.empty()) {
            tm t = {};
            stringstream ss(date_param);
            ss >> get_time(&t, "%Y-%m-%d");
            if (!ss.fail()) {
                t.tm_hour = 12; // Avoid DST issues
                now = mktime(&t);
                ltm = localtime(&now);
            }
        }

        // Calculate Monday of the current week
        int day_of_week = ltm->tm_wday; // 0 = Sunday, 1 = Monday...
        int days_to_monday = (day_of_week == 0) ? 6 : (day_of_week - 1);

        time_t monday_time = now - (days_to_monday * 24 * 3600);
        tm monday_tm = *localtime(&monday_time);
        monday_tm.tm_hour = 0;
        monday_tm.tm_min = 0;
        monday_tm.tm_sec = 0;
        monday_time = mktime(&monday_tm);

        string html = loadHtmlTemplate("templates/dashboard.html");
        size_t usrname = html.find("{{username}}");
        if (usrname != string::npos)
            html.replace(usrname, 12, username);

        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d", &monday_tm);
        string monday_str = buf;

        time_t prev_week_time = monday_time - (7 * 24 * 3600);
        tm prev_week_tm = *localtime(&prev_week_time);
        strftime(buf, sizeof(buf), "%Y-%m-%d", &prev_week_tm);
        string prev_week_str = buf;

        time_t next_week_time = monday_time + (7 * 24 * 3600);
        tm next_week_tm = *localtime(&next_week_time);
        strftime(buf, sizeof(buf), "%Y-%m-%d", &next_week_tm);
        string next_week_str = buf;

        time_t sunday_time = monday_time + (6 * 24 * 3600);
        tm sunday_tm = *localtime(&sunday_time);
        char buf2[64];
        strftime(buf, sizeof(buf), "%d.%m", &monday_tm);
        strftime(buf2, sizeof(buf2), "%d.%m.%Y", &sunday_tm);
        string week_range = string(buf) + " - " + string(buf2);

        auto replace_tag = [&](string tag, string val) {
            size_t pos = html.find(tag);
            if (pos != string::npos)
                html.replace(pos, tag.length(), val);
        };

        replace_tag("{{prev_week}}", prev_week_str);
        replace_tag("{{next_week}}", next_week_str);
        replace_tag("{{week_range}}", week_range);

        // POPRAWKA: Pobieranie widoku przeniesione NA SAMĄ GÓRĘ, przed generowanie HTML kalendarza
        string view = req.url_params.get("view") != nullptr ? req.url_params.get("view") : "all";
        replace_tag("{{current_view}}", view);

        vector<Event> events;
        if (view == "private") {
            events = get_user_event(username);
        } else if (view == "all") {
            events = get_all_events(username);
        } else {
            json g = {{"id", view}};
            get_group_events(g, events);
        }

        string events_html;
        string days[] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

        for (int i = 0; i < 7; i++) {
            time_t current_day_time = monday_time + (i * 24 * 3600);
            tm current_day_tm = *localtime(&current_day_time);
            strftime(buf, sizeof(buf), "%Y-%m-%d", &current_day_tm);
            string current_day_str = buf;
            strftime(buf, sizeof(buf), "%d.%m", &current_day_tm);
            string day_label = buf;

            events_html += "<div class='day-column' data-day='" + current_day_str + "'>";
            events_html += "  <div class='day-header'>" + days[i] + "<br><small>" + day_label + "</small></div>";

            for (const auto &e : events) {
                string dec_start = urlDecode(e.start);
                string dec_end = urlDecode(e.end);
                string dec_title = urlDecode(e.title);
                string dec_desc = urlDecode(e.description);

                if (dec_start.size() >= 10 and dec_start.substr(0, 10) == current_day_str) {
                    string start_time = "00:00";
                    string end_time = "23:59";

                    if (dec_start.size() >= 16)
                        start_time = dec_start.substr(11, 5);
                    if (dec_end.size() >= 16)
                        end_time = dec_end.substr(11, 5);

                    events_html += "<div class=\"event-card\" ";
                    events_html += "data-id=\"" + e.id + "\" ";
                    events_html += "data-title=\"" + dec_title + "\" ";
                    events_html += "data-user=\"" + e.user + "\" ";
                    events_html += "data-description=\"" + dec_desc + "\" ";
                    events_html += "data-start=\"" + start_time + "\" ";
                    events_html += "data-end=\"" + end_time + "\" ";
                    events_html += "data-origin=\"" + e.origin + "\" ";
                    events_html += "data-recurrence=\"" + e.recurrence + "\">";
                    events_html += "  <span class='time'>" + start_time + " - " + end_time + "</span>";
                    events_html += "  <span class='title'>" + dec_title + "</span>";
                    events_html += "</div>";
                }
            }
            events_html += "</div>";
        }
        replace_tag("{{calendar_content}}", events_html);

        crow::response res(html);
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });
}

// ==================== NOWA TRASA W API_ROUTES ====================
// Dodaj tę trasę wewnątrz funkcji api_routes(crow::SimpleApp& app):
