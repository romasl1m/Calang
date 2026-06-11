#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <algorithm>
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

// Vector to store dates from searches for AI to reference
static vector<string> DATE_VECTOR;

string process_terminal_command(const string &fullLine, const string &currentUsername, string &currentgroup) {
    if (fullLine.empty())
        return "";

    // Replace $DATE[n] with stored dates from vector
    string processedLine = fullLine;
    size_t pos = 0;
    while ((pos = processedLine.find("$DATE", pos)) != string::npos) {
        // Check if there's a bracket like $DATE[0]
        if (pos + 5 < processedLine.length() && processedLine[pos + 5] == '[') {
            size_t endBracket = processedLine.find(']', pos + 6);
            if (endBracket != string::npos) {
                string indexStr = processedLine.substr(pos + 6, endBracket - pos - 6);
                try {
                    int index = stoi(indexStr);
                    if (index >= 0 && index < (int)DATE_VECTOR.size()) {
                        processedLine.replace(pos, endBracket - pos + 1, DATE_VECTOR[index]);
                    } else {
                        processedLine.replace(pos, endBracket - pos + 1, getCurrentYear() + "-01-01");
                    }
                } catch (...) {
                    processedLine.replace(pos, endBracket - pos + 1, getCurrentYear() + "-01-01");
                }
                pos += 10; // Move past the replacement
                continue;
            }
        }
        // $DATE without bracket - use first date if available
        if (!DATE_VECTOR.empty()) {
            processedLine.replace(pos, 5, DATE_VECTOR[0]);
            pos += DATE_VECTOR[0].length();
        } else {
            processedLine.replace(pos, 5, getCurrentYear() + "-01-01");
            pos += 10;
        }
    }

    stringstream ss(processedLine);
    string cmd;
    ss >> cmd;

    stringstream output;

    if (cmd == "clear") {
        return "CLEAR_SIGNAL";
    } else if (cmd == "dates") {
        output << "Stored dates in $DATE vector:\n";
        if (DATE_VECTOR.empty()) {
            output << "  (empty - use cat or grep to populate)\n";
        } else {
            for (size_t i = 0; i < DATE_VECTOR.size(); i++) {
                output << "  [" << i << "] " << DATE_VECTOR[i] << "\n";
            }
            output << "\nUse $DATE or $DATE[n] in commands\n";
        }
    } else if (cmd == "whoami") {
        output << currentUsername << "\n";
    } else if (cmd == "help") {
        output << "Available commands:\n"
               << "  cat {YYYY-MM-DD} - show events for a specific date\n"
               << "  cat {number} - show next N events and set $DATE\n"
               << "  grep \"text\" - find events containing text\n"
               << "  touch \"event name\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup]\n"
               << "  touch \"event name\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup]\n"
               << "  cd <group_name_or_id> - change to a specific group\n"
               << "  cd ~ or cd private - change to private calendar\n"
               << "  clear\n"
               << "  whoami\n"
               << "  dates - show stored dates from recent searches\n"
               << "  $DATE or $DATE[n] - use stored dates in commands (0=first)\n";
    } else if (cmd == "cat") {
        string date;
        ss >> date;

        // Check if it's a number (cat N for next N events)
        bool isNumber = !date.empty() && all_of(date.begin(), date.end(), ::isdigit);

        if (isNumber) {
            int count = stoi(date);
            vector<Event> allEvents = get_all_events(currentUsername);

            // Get current time
            time_t now = time(nullptr);
            tm *currentTime = localtime(&now);
            char currentDateStr[64];
            strftime(currentDateStr, sizeof(currentDateStr), "%Y-%m-%d %H:%M", currentTime);
            string nowStr = currentDateStr;

            // Filter and sort upcoming events
            vector<Event> upcomingEvents;
            for (const auto &evt : allEvents) {
                if (evt.start >= nowStr) {
                    upcomingEvents.push_back(evt);
                }
            }

            // Sort by start time
            sort(upcomingEvents.begin(), upcomingEvents.end(),
                 [](const Event &a, const Event &b) { return a.start < b.start; });

            // Take first N events
            int limit = min(count, (int)upcomingEvents.size());
            output << "--- Next " << limit << " events ---\n";

            // Clear and populate date vector
            DATE_VECTOR.clear();
            for (int i = 0; i < limit; i++) {
                const Event &evt = upcomingEvents[i];
                output << "ID: " << evt.id << "\n"
                       << "  Title: " << evt.title << "\n"
                       << "  Start: " << evt.start << "\n"
                       << "  End: " << evt.end << "\n"
                       << "  Description: " << evt.description << "\n";
                if (!evt.subgroup.empty()) {
                    output << "  Subgroup: " << evt.subgroup << "\n";
                }
                output << "\n";

                // Add each event's date to the vector
                DATE_VECTOR.push_back(evt.start.substr(0, 10)); // Extract YYYY-MM-DD
            }

            if (limit > 0) {
                output << "Stored " << DATE_VECTOR.size() << " date(s) in $DATE vector\n";
            }
        } else {
            // Original behavior - show events for specific date
            if (date.length() == 5) {
                date = getCurrentYear() + "-" + date;
            }

            vector<Event> allEvents = get_all_events(currentUsername);
            vector<Event> dayEvents;

            for (const auto &evt : allEvents) {
                if (evt.start.substr(0, 10) == date) {
                    dayEvents.push_back(evt);
                }
            }

            output << "--- Events on " << date << " ---\n";
            if (dayEvents.empty()) {
                output << "No events found.\n";
            } else {
                // Clear and populate date vector with event dates
                DATE_VECTOR.clear();
                for (const auto &evt : dayEvents) {
                    output << "ID: " << evt.id << "\n"
                           << "  Title: " << evt.title << "\n"
                           << "  Start: " << evt.start << "\n"
                           << "  End: " << evt.end << "\n"
                           << "  Description: " << evt.description << "\n";
                    if (!evt.subgroup.empty()) {
                        output << "  Subgroup: " << evt.subgroup << "\n";
                    }
                    output << "\n";

                    // Add each unique date to vector
                    string eventDate = evt.start.substr(0, 10);
                    if (find(DATE_VECTOR.begin(), DATE_VECTOR.end(), eventDate) == DATE_VECTOR.end()) {
                        DATE_VECTOR.push_back(eventDate);
                    }
                }

                output << "Stored " << DATE_VECTOR.size() << " date(s) in $DATE vector\n";
            }
        }
    } else if (cmd == "grep") {
        string remaining;
        getline(ss, remaining);

        // Remove leading/trailing whitespace
        remaining = remaining.substr(remaining.find_first_not_of(" \t"));

        // Remove quotes if present
        if (!remaining.empty() && remaining.front() == '"' && remaining.back() == '"') {
            remaining = remaining.substr(1, remaining.length() - 2);
        }

        if (remaining.empty()) {
            output << "Usage: grep \"search text\"\n";
            return output.str();
        }

        vector<Event> allEvents = get_all_events(currentUsername);
        vector<Event> matchedEvents;

        // Convert search text to lowercase for case-insensitive search
        string searchLower = remaining;
        transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);

        for (const auto &evt : allEvents) {
            string titleLower = evt.title;
            string descLower = evt.description;
            transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            transform(descLower.begin(), descLower.end(), descLower.begin(), ::tolower);

            if (titleLower.find(searchLower) != string::npos ||
                descLower.find(searchLower) != string::npos) {
                matchedEvents.push_back(evt);
            }
        }

        output << "--- Found " << matchedEvents.size() << " events matching \"" << remaining << "\" ---\n";

        if (matchedEvents.empty()) {
            output << "No events found.\n";
        } else {
            // Sort by start time
            sort(matchedEvents.begin(), matchedEvents.end(),
                 [](const Event &a, const Event &b) { return a.start < b.start; });

            // Clear and populate date vector with matched event dates
            DATE_VECTOR.clear();
            for (const auto &evt : matchedEvents) {
                output << "ID: " << evt.id << "\n"
                       << "  Title: " << evt.title << "\n"
                       << "  Start: " << evt.start << "\n"
                       << "  End: " << evt.end << "\n"
                       << "  Description: " << evt.description << "\n";
                if (!evt.subgroup.empty()) {
                    output << "  Subgroup: " << evt.subgroup << "\n";
                }
                output << "\n";

                // Add each unique date to vector
                string eventDate = evt.start.substr(0, 10);
                if (find(DATE_VECTOR.begin(), DATE_VECTOR.end(), eventDate) == DATE_VECTOR.end()) {
                    DATE_VECTOR.push_back(eventDate);
                }
            }

            output << "Stored " << DATE_VECTOR.size() << " date(s) in $DATE vector\n";
        }
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

        // KLUCZOWA ZMIANA: Domyślnie używa wejścia z komendy 'cd'
        string origin = currentgroup.empty() ? "private" : currentgroup;
        string subgroup = "";

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
                    } else if (args[i].rfind("S=", 0) == 0) {
                        subgroup = args[i].substr(2);
                    } else if (args[i].rfind("O=", 0) == 0) { // Pozwala AI nadpisać grupę 'cd'
                        origin = args[i].substr(2);
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
                } else if (args[i].rfind("S=", 0) == 0) {
                    subgroup = args[i].substr(2);
                } else if (args[i].rfind("O=", 0) == 0) {
                    origin = args[i].substr(2);
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
                } else if (args[i].rfind("S=", 0) == 0) {
                    subgroup = args[i].substr(2);
                } else if (args[i].rfind("O=", 0) == 0) {
                    origin = args[i].substr(2);
                }
            }
        } else {
            output << "Syntax error. Correct usage:\n"
                   << "  AI format: touch \"title\" \"YYYY-MM-DD HH:MM\" \"YYYY-MM-DD HH:MM\" \"description\" [P=priority] [T=recurrence] [O=origin]\n"
                   << "  Manual format: touch \"title\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence] [O=origin]\n"
                   << "  Length format: touch \"title\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence] [O=origin]\n";
            return output.str();
        }

        if (start.length() >= 10 and start[10] == 'T')
            start[10] = ' ';
        if (end.length() >= 10 and end[10] == 'T')
            end[10] = ' ';

        // Rozwiązywanie nazwy na ID, niezależnie czy pochodzi z CD czy z argumentu O=
        string resolved_origin = origin;
        if (origin != "private") {
            bool found = false;
            vector<json> groups = get_user_groups(currentUsername);
            for (const auto &g : groups) {
                if (g.value("name", "") == origin or g.value("id", "") == origin) {
                    resolved_origin = g.value("id", "");
                    found = true;
                    break;
                }
            }
            if (not found) {
                output << "Error: Group '" << origin << "' not found or you are not a member.\n";
                return output.str();
            }
        }

        string id = to_string(time(0)) + "_" + to_string(rand() % 1000);

        add_new_event(title, id, start, end, currentUsername, desc, resolved_origin, recurrence, "", priority, subgroup);

        // ... Kod outputu pozostaje bez zmian ...
        output << "An event created!\n"
               << "-> Title: " << title << "\n"
               << "-> Start: " << start << "\n"
               << "-> End:   " << end << "\n"
               << "-> Desc:  " << desc << "\n"
               << "-> Type:  " << origin << "\n"
               << "-> Prior: " << priority << "\n"
               << "-> Recur: " << recurrence << "\n";
        if (not subgroup.empty()) {
            output << "-> Subgr: " << subgroup << "\n";
        }
    } else if (cmd == "cd") {
        string groupNameOrId;
        getline(ss, groupNameOrId);
        groupNameOrId = groupNameOrId.substr(groupNameOrId.find_first_not_of(" \t"));

        if (groupNameOrId.empty() or groupNameOrId == "~" or groupNameOrId == "private") {
            currentgroup = "";
            output << "Changed to private calendar.\n";
            return output.str();
        }

        string resolved_group = groupNameOrId;
        string resolved_name = groupNameOrId;
        bool found = false;

        vector<json> groups = get_user_groups(currentUsername);
        for (const auto &g : groups) {
            if (g.value("name", "") == groupNameOrId or g.value("id", "") == groupNameOrId) {
                resolved_group = g.value("id", "");
                resolved_name = g.value("name", "");
                found = true;
                break;
            }
        }

        if (not found) {
            output << "Error: Group '" << groupNameOrId << "' not found or you are not a member.\n";
            return output.str();
        }

        currentgroup = resolved_group;
        output << "Changed to group: " << resolved_name << "\n";
        output << "RELOAD_CALENDAR\n";
    } else {
        output << "Unknown command. Try 'help'.\n";
    }

    return output.str();
}

void executeTerminal(const string &currentUsername) {
    string group = "";
    string line;
    string username = currentUsername;

    cout << username << "@calang/" << group << ":~$ ";
    while (true) {
        // if (!getline(cin, line))
        //     break;

        if (line.empty())
            continue;

        stringstream ss(line);
        string cmd;
        ss >> cmd;

        //         if (cmd == "clear") {
        // #ifdef _WIN32
        //             (void)system("cls");
        // #else
        //             int result = system("clear");
        //             (void)result; // Ucisza kompilator
        // #endif
        //} else if (cmd == "whoami") {
        if (cmd == "whoami") {
            cout << username << endl;
        } else if (cmd == "exit") {
            break;
        } else if (cmd == "help") {
            string output = "Available commands:\n  cat {YYYY-MM-DD}\n  touch \"event name\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup]\n cd <group_name_or_id>\n  cd ~ or cd private\n";
            cout << output << endl;
        } else {
            string output = process_terminal_command(line, username, group);
            cout << output;
        }
    }
}
