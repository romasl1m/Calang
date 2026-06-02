#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>
#include "functions.h"

using namespace std;
using json = nlohmann::json;

string getCurrentYear() {
    time_t t = time(nullptr);
    tm *now = localtime(&t);
    return to_string(now->tm_year + 1900);
}

vector<string> parseArguments(const string &argumentStr) {
    vector<string> args;
    string current;
    bool inQuotes = false;

    for (char c : argumentStr) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' and not inQuotes) {
            if (!current.empty()) {
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (not current.empty()) {
        args.push_back(current);
    }
    return args;
}

string formatDateTime(const string &rawDateTime) {
    if (rawDateTime.length() == 11 and rawDateTime[5] == ' ') {
        return getCurrentYear() + "-" + rawDateTime;
    }
    return rawDateTime;
}

string parseAndFormatDateTime(const string &dateStr, const string &timeStr) {
    int day = 0, month = 0;
    char dot = 0;
    if (sscanf(dateStr.c_str(), "%d%c%d", &day, &dot, &month) == 3 && dot == '.') {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s-%02d-%02d %s", getCurrentYear().c_str(), month, day, timeStr.c_str());
        return string(buf);
    }
    return dateStr + " " + timeStr;
}

string process_terminal_command(const string &fullLine, const string &currentUsername) {
    if (fullLine.empty())
        return "";

    stringstream ss(fullLine);
    string cmd;
    ss >> cmd;

    stringstream output;

    if (cmd == "clear") {
        return "CLEAR_SIGNAL";
    } else if (cmd == "whoami") {
        output << currentUsername << "\n";
    } else if (cmd == "help") {
        output << "Available commands:\n"
               << "  cat {YYYY-MM-DD}\n"
               << "  touch \"event name\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence]\n"
               << "  touch \"event name\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence]\n"
               << "  ccd <event_id> <new_group>\n"
               << "  clear\n"
               << "  whoami\n";
    } else if (cmd == "cat") {
        string date;
        ss >> date;
        if (date.length() == 5) {
            date = getCurrentYear() + "-" + date;
        }
        output << "--- This day events: " << date << " ---\n";
    } else if (cmd == "touch") {
        string remaining;
        getline(ss, remaining);
        vector<string> args = parseArguments(remaining);

        if (args.empty()) {
            output << "Syntax error. Type 'help' for usage.\n";
            return output.str();
        }

        string title, start, end, desc;
        string recurrence = "none";
        string priority = "medium";
        string origin = "private";

        bool isLengthFormat = false;
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i] == "in" or args[i] == "length") {
                isLengthFormat = true;
                break;
            }
        }

        if (isLengthFormat) {
            if (args.size() >= 7 and args[1] == "in" and args[4] == "length") {
                title = args[0];
                string startDate = args[2];
                string startTime = args[3];
                string durationHours = args[5];
                desc = args[6];

                start = parseAndFormatDateTime(startDate, startTime);

                int hours = 0, minutes = 0;
                if (sscanf(durationHours.c_str(), "%d:%d", &hours, &minutes) == 2) {
                    tm timeStart = {};
                    if (strptime(start.c_str(), "%Y-%m-%d %H:%M", &timeStart) != nullptr) {
                        timeStart.tm_hour += hours;
                        timeStart.tm_min += minutes;
                        mktime(&timeStart);

                        char buf[64];
                        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &timeStart);
                        end = string(buf);
                    }
                }

                for (size_t i = 7; i < args.size(); i++) {
                    if (args[i].rfind("T=", 0) == 0) {
                        recurrence = args[i].substr(2);
                    } else if (args[i].rfind("P=", 0) == 0) {
                        priority = args[i].substr(2);
                    }
                }
            } else {
                output << "Syntax error. Length format usage:\n"
                       << "  touch \"title\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence]\n";
                return output.str();
            }
        } else if (args.size() >= 5 and args[1].length() >= 10) {
            title = args[0];
            start = args[1];
            end = args[2];
            desc = args[3];
            for (size_t i = 4; i < args.size(); i++) {
                if (args[i].rfind("T=", 0) == 0) {
                    recurrence = args[i].substr(2);
                } else if (args[i].rfind("P=", 0) == 0) {
                    priority = args[i].substr(2);
                }
            }
        } else if (args.size() >= 5) {
            title = args[0];
            start = parseAndFormatDateTime(args[1], args[2]);
            end = parseAndFormatDateTime(args[1], args[3]);
            desc = args[4];
            for (size_t i = 5; i < args.size(); i++) {
                if (args[i].rfind("T=", 0) == 0) {
                    recurrence = args[i].substr(2);
                } else if (args[i].rfind("P=", 0) == 0) {
                    priority = args[i].substr(2);
                }
            }
        } else {
            output << "Syntax error. Correct usage:\n"
                   << "  AI format: touch \"title\" \"YYYY-MM-DD HH:MM\" \"YYYY-MM-DD HH:MM\" \"description\" [P=priority] [T=recurrence]\n"
                   << "  Manual format: touch \"title\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence]\n"
                   << "  Length format: touch \"title\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence]\n";
            return output.str();
        }

        if (start.length() >= 10 and start[10] == 'T')
            start[10] = ' ';
        if (end.length() >= 10 and end[10] == 'T')
            end[10] = ' ';

        string id = to_string(time(0)) + "_" + to_string(rand() % 1000);

        add_new_event(title, id, start, end, currentUsername, desc, origin, recurrence, "", priority);

        output << "An event created!\n"
               << "-> Title: " << title << "\n"
               << "-> Start: " << start << "\n"
               << "-> End:   " << end << "\n"
               << "-> Desc:  " << desc << "\n"
               << "-> Type:  private\n"
               << "-> Prior: " << priority << "\n"
               << "-> Recur: " << recurrence << "\n";
    } else if (cmd == "ccd") {
        string eventId, groupNameOrId;
        ss >> eventId >> groupNameOrId;

        if (eventId.empty() or groupNameOrId.empty()) {
            output << "Syntax error. Usage: ccd <event_id> <group_name_or_id>\n";
            return output.str();
        }

        string resolved_group = groupNameOrId;
        if (groupNameOrId != "private") {
            bool found = false;
            vector<json> groups = get_user_groups(currentUsername);
            for (const auto &g : groups) {
                if (g.value("name", "") == groupNameOrId or g.value("id", "") == groupNameOrId) {
                    resolved_group = g.value("id", "");
                    found = true;
                    break;
                }
            }
            if (not found) {
                output << "Error: Group '" << groupNameOrId << "' not found or you are not a member.\n";
                return output.str();
            }
        }

        if (change_event_group(eventId, resolved_group)) {
            output << "Event " << eventId << " moved to group: " << groupNameOrId << "\n";
        } else {
            output << "Error: Could not change event group. Event may not exist or you lack permissions.\n";
        }
    } else {
        output << "Unknown command. Try 'help'.\n";
    }

    return output.str();
}

void executeTerminal() {
    string line;
    string username = "user";

    while (true) {
        cout << username << "@calang:~$ ";
        if (!getline(cin, line))
            break;

        if (line.empty())
            continue;

        stringstream ss(line);
        string cmd;
        ss >> cmd;

        if (cmd == "clear") {
#ifdef _WIN32
            (void)system("cls");
#else
            int result = system("clear");
            (void)result; // Ucisza kompilator
#endif
        } else if (cmd == "whoami") {
            cout << username << endl;
        } else if (cmd == "exit") {
            break;
        } else if (cmd == "help") {
            cout << "Available commands:\n  cat {YYYY-MM-DD}\n  touch \"event name\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence]\n  touch \"event name\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence]\n  ccd <event_id> <new_group>\n  clear\n  whoami" << endl;
        } else {
            cout << process_terminal_command(line, username);
        }
    }
}
