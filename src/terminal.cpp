#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "functions.h"
#include "ai_assistant.h"
#include "google_oauth.h"

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

// Helper function to execute a single command
string execute_single_command(const string &cmd_line, const string &currentUsername, string &currentgroup);

string process_terminal_command(const string &fullLine, const string &currentUsername, string &currentgroup) {
    if (fullLine.empty())
        return "";

    stringstream finalOutput;

    // Handle command chaining with && and ||
    vector<string> commands;
    vector<string> operators;

    string current;
    bool inQuotes = false;

    for (size_t i = 0; i < fullLine.length(); i++) {
        char c = fullLine[i];

        if (c == '"') {
            inQuotes = !inQuotes;
            current += c;
        } else if (!inQuotes && i + 1 < fullLine.length() && c == '&' && fullLine[i + 1] == '&') {
            // Found &&
            if (!current.empty()) {
                commands.push_back(current);
                current.clear();
            }
            operators.push_back("&&");
            i++; // Skip next &
        } else if (!inQuotes && i + 1 < fullLine.length() && c == '|' && fullLine[i + 1] == '|') {
            // Found ||
            if (!current.empty()) {
                commands.push_back(current);
                current.clear();
            }
            operators.push_back("||");
            i++; // Skip next |
        } else if (!inQuotes && c == ';') {
            // Found ;
            if (!current.empty()) {
                commands.push_back(current);
                current.clear();
            }
            operators.push_back(";");
        } else {
            current += c;
        }
    }

    if (!current.empty()) {
        commands.push_back(current);
    }

    // If no operators found, just execute the single command
    if (commands.size() == 1) {
        return execute_single_command(fullLine, currentUsername, currentgroup);
    }

    // Execute commands with operators
    bool lastSuccess = true;

    for (size_t i = 0; i < commands.size(); i++) {
        // Trim whitespace
        string cmd = commands[i];
        cmd.erase(0, cmd.find_first_not_of(" \t"));
        cmd.erase(cmd.find_last_not_of(" \t") + 1);

        bool shouldExecute = true;

        if (i > 0) {
            string op = operators[i - 1];
            if (op == "&&" && !lastSuccess) {
                shouldExecute = false;
            } else if (op == "||" && lastSuccess) {
                shouldExecute = false;
            }
        }

        if (shouldExecute) {
            string result = execute_single_command(cmd, currentUsername, currentgroup);
            finalOutput << result;

            // Check if command was successful
            // Consider a command failed if it contains "Error:" or "not found" or "Syntax error"
            lastSuccess = (result.find("Error:") == string::npos &&
                          result.find("not found") == string::npos &&
                          result.find("Syntax error") == string::npos &&
                          result.find("Unknown command") == string::npos);

            // Handle special signals
            if (result.find("CLEAR_SIGNAL") != string::npos) {
                return result;
            }
        }
    }

    return finalOutput.str();
}

string execute_single_command(const string &fullLine, const string &currentUsername, string &currentgroup) {
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
    } else if (cmd == "sync") {
        output << "Syncing events from Google Calendar...\n";
        int imported = syncGoogleEvents(currentUsername);
        if (imported >= 0) {
            output << "Successfully imported " << imported << " events from Google Calendar\n";
            output << "RELOAD_CALENDAR\n";
        } else {
            output << "Failed to sync Google Calendar. Make sure you're logged in with Google.\n";
        }
    } else if (cmd == "help") {
        output << "Available commands:\n"
               << "  cat {YYYY-MM-DD} - show events for a specific date\n"
               << "  cat {number} - show next N events and set $DATE\n"
               << "  grep \"text\" - find events containing text\n"
               << "  touch \"event name\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup]\n"
               << "  touch \"event name\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence] [S=subgroup]\n"
               << "  cd <group_name_or_id>[/subgroup] - change to a group or subgroup\n"
               << "  cd ~ or cd private - change to private calendar\n"
               << "  ls - list available groups and subgroups\n"
               << "  clear\n"
               << "  whoami\n"
               << "  sync - import events from Google Calendar\n"
               << "  ai \"natural language request\" - use AI to execute commands\n"
               << "  rm <event_id> - delete an event\n"
               << "\nBash-like operators:\n"
               << "  cmd1 && cmd2 - execute cmd2 only if cmd1 succeeds\n"
               << "  cmd1 || cmd2 - execute cmd2 only if cmd1 fails\n"
               << "  cmd1 ; cmd2 - execute both commands regardless\n"
               << "  if <condition> then <cmd1> else <cmd2> - conditional execution\n"
               << "  test -n \"string\" or [ -n \"string\" ] - test if string is not empty\n";
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
                }
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
            }
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
        string recurrence_end = "";
        string priority = "medium";

        // Parse currentgroup to separate group_id and subgroup path
        string origin = "private";
        string subgroup = "";

        if (!currentgroup.empty()) {
            size_t slashPos = currentgroup.find('/');
            if (slashPos != string::npos) {
                // currentgroup contains "group_id/subgroup/path"
                origin = currentgroup.substr(0, slashPos);
                subgroup = currentgroup.substr(slashPos + 1);
            } else {
                // currentgroup is just group_id
                origin = currentgroup;
            }
        }

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
                    } else if (args[i].rfind("R=", 0) == 0) {
                        recurrence_end = args[i].substr(2);
                    } else if (args[i].rfind("P=", 0) == 0) {
                        priority = args[i].substr(2);
                    } else if (args[i].rfind("S=", 0) == 0) {
                        subgroup = args[i].substr(2);
                    } else if (args[i].rfind("O=", 0) == 0) {
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
                } else if (args[i].rfind("R=", 0) == 0) {
                    recurrence_end = args[i].substr(2);
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
                } else if (args[i].rfind("R=", 0) == 0) {
                    recurrence_end = args[i].substr(2);
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
                   << "  AI format: touch \"title\" \"YYYY-MM-DD HH:MM\" \"YYYY-MM-DD HH:MM\" \"description\" [P=priority] [T=recurrence] [R=YYYY-MM-DD] [O=origin]\n"
                   << "  Manual format: touch \"title\" DD.MM HH:MM HH:MM \"description\" [P=priority] [T=recurrence] [R=YYYY-MM-DD] [O=origin]\n"
                   << "  Length format: touch \"title\" in DD.MM HH:MM length HH:MM \"description\" [P=priority] [T=recurrence] [R=YYYY-MM-DD] [O=origin]\n";
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

        add_new_event(title, id, start, end, currentUsername, desc, resolved_origin, recurrence, "", recurrence_end, priority, subgroup);

        // Try to create event in Google Calendar if user has access token
        string access_token = get_user_access_token(currentUsername);
        if (!access_token.empty()) {
            string calendar_response = createGoogleCalendarEvent(access_token, title, start, end, desc);
            if (!calendar_response.empty()) {
                try {
                    json cal_json = json::parse(calendar_response);
                    if (cal_json.contains("id")) {
                        output << "-> Google Calendar: Event created successfully (ID: " << cal_json["id"].get<string>() << ")\n";
                    } else if (cal_json.contains("error")) {
                        output << "-> Google Calendar: Error - " << cal_json["error"]["message"].get<string>() << "\n";
                    }
                } catch (...) {
                    output << "-> Google Calendar: Failed to parse response\n";
                }
            }
        }

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
    } else if (cmd == "ls") {
        vector<json> groups = get_user_groups(currentUsername);

        if (groups.empty()) {
            output << "No groups found.\n";
        } else {
            output << "Available groups:\n";
            for (const auto &g : groups) {
                string groupName = g.value("name", "");
                string groupId = g.value("id", "");
                output << "  " << groupName << " (id: " << groupId << ")\n";

                // Show subgroups if any
                vector<string> subgroups = get_subgroups(groupId);
                if (!subgroups.empty()) {
                    output << "    Subgroups:\n";
                    for (const auto &sg : subgroups) {
                        output << "      " << sg << "\n";
                    }
                }
            }
        }
        output << "\nUse: cd <group_name>[/subgroup]\n";
    } else if (cmd == "ai") {
        string remaining;
        getline(ss, remaining);
        remaining = remaining.substr(remaining.find_first_not_of(" \t"));

        // Remove quotes if present
        if (!remaining.empty() && remaining.front() == '"' && remaining.back() == '"') {
            remaining = remaining.substr(1, remaining.length() - 2);
        }

        if (remaining.empty()) {
            output << "Usage: ai \"your natural language request\"\n"
                   << "Example: ai \"show me meetings next week\"\n"
                   << "Example: ai \"create a meeting tomorrow at 2pm for 1 hour\"\n";
            return output.str();
        }

        // Process AI command
        string result = process_ai_command(remaining, currentUsername, currentgroup);
        return result;
    } else if (cmd == "cd") {
        string groupNameOrId;
        getline(ss, groupNameOrId);
        groupNameOrId = groupNameOrId.substr(groupNameOrId.find_first_not_of(" \t"));

        if (groupNameOrId.empty() or groupNameOrId == "~" or groupNameOrId == "private") {
            currentgroup = "";
            output << "Changed to private calendar.\n";
            return output.str();
        }

        // Check if path contains a slash (indicating subgroup navigation)
        size_t slashPos = groupNameOrId.find('/');
        string groupPart = (slashPos != string::npos) ? groupNameOrId.substr(0, slashPos) : groupNameOrId;
        string subgroupPath = (slashPos != string::npos) ? groupNameOrId.substr(slashPos + 1) : "";

        string resolved_group = groupPart;
        string resolved_name = groupPart;
        bool found = false;

        vector<json> groups = get_user_groups(currentUsername);
        for (const auto &g : groups) {
            if (g.value("name", "") == groupPart or g.value("id", "") == groupPart) {
                resolved_group = g.value("id", "");
                resolved_name = g.value("name", "");
                found = true;
                break;
            }
        }

        if (not found) {
            output << "Error: Group '" << groupPart << "' not found or you are not a member.\n";
            return output.str();
        }

        // If subgroup specified, verify it exists
        if (!subgroupPath.empty()) {
            vector<string> availableSubgroups = get_subgroups(resolved_group);
            bool subgroupFound = false;
            for (const auto &sg : availableSubgroups) {
                if (sg == subgroupPath) {
                    subgroupFound = true;
                    break;
                }
            }
            if (!subgroupFound) {
                output << "Error: Subgroup '" << subgroupPath << "' not found in group '" << resolved_name << "'.\n";
                output << "Available subgroups:\n";
                for (const auto &sg : availableSubgroups) {
                    output << "  - " << sg << "\n";
                }
                return output.str();
            }
            // Store group with subgroup
            currentgroup = resolved_group + "/" + subgroupPath;
            output << "Changed to group: " << resolved_name << "/" << subgroupPath << "\n";
        } else {
            currentgroup = resolved_group;
            output << "Changed to group: " << resolved_name << "\n";
        }
        output << "RELOAD_CALENDAR\n";
    } else if (cmd == "rm") {
        string eventId;
        ss >> eventId;

        if (eventId.empty()) {
            output << "Usage: rm <event_id>\n";
            return output.str();
        }

        vector<Event> allEvents = get_all_events(currentUsername);
        bool found = false;

        for (const auto &evt : allEvents) {
            if (evt.id == eventId) {
                found = true;
                delete_event(eventId, currentUsername, evt.origin);
                output << "Event deleted: " << evt.title << " (ID: " << eventId << ")\n";
                output << "RELOAD_CALENDAR\n";
                break;
            }
        }

        if (!found) {
            output << "Error: Event with ID '" << eventId << "' not found.\n";
        }
    } else if (cmd == "if") {
        // Simple if statement: if [condition_cmd] then [cmd1] else [cmd2]
        string remaining;
        getline(ss, remaining);

        // Parse: if <condition> then <cmd1> else <cmd2>
        size_t thenPos = remaining.find(" then ");
        if (thenPos == string::npos) {
            output << "Syntax error. Usage: if <condition_cmd> then <true_cmd> else <false_cmd>\n";
            return output.str();
        }

        string condition = remaining.substr(0, thenPos);
        condition.erase(0, condition.find_first_not_of(" \t"));

        string afterThen = remaining.substr(thenPos + 6); // Skip " then "
        size_t elsePos = afterThen.find(" else ");

        string trueCmd, falseCmd;

        if (elsePos != string::npos) {
            trueCmd = afterThen.substr(0, elsePos);
            falseCmd = afterThen.substr(elsePos + 6); // Skip " else "
        } else {
            trueCmd = afterThen;
        }

        // Trim commands
        trueCmd.erase(0, trueCmd.find_first_not_of(" \t"));
        trueCmd.erase(trueCmd.find_last_not_of(" \t") + 1);
        falseCmd.erase(0, falseCmd.find_first_not_of(" \t"));
        falseCmd.erase(falseCmd.find_last_not_of(" \t") + 1);

        // Execute condition command
        string condResult = execute_single_command(condition, currentUsername, currentgroup);

        // Check if condition succeeded
        bool condSuccess = (condResult.find("Error:") == string::npos &&
                           condResult.find("not found") == string::npos &&
                           condResult.find("Syntax error") == string::npos &&
                           condResult.find("Unknown command") == string::npos &&
                           condResult.find("No events found") == string::npos);

        // Execute appropriate command
        if (condSuccess && !trueCmd.empty()) {
            output << execute_single_command(trueCmd, currentUsername, currentgroup);
        } else if (!condSuccess && !falseCmd.empty()) {
            output << execute_single_command(falseCmd, currentUsername, currentgroup);
        }
    } else if (cmd == "test" || cmd == "[") {
        // Simple test command: test -n "string" or [ -n "string" ]
        string remaining;
        getline(ss, remaining);

        // Remove trailing ] if present
        if (!remaining.empty() && remaining.back() == ']') {
            remaining.pop_back();
        }

        remaining.erase(0, remaining.find_first_not_of(" \t"));
        remaining.erase(remaining.find_last_not_of(" \t") + 1);

        if (remaining.empty()) {
            output << "Usage: test -n \"string\" or test -z \"string\"\n";
            return output.str();
        }

        vector<string> testArgs = parseArguments(remaining);

        if (testArgs.size() >= 2 && testArgs[0] == "-n") {
            // Test if string is not empty
            bool notEmpty = !testArgs[1].empty();
            if (!notEmpty) {
                output << "test: false\n";
            }
        } else if (testArgs.size() >= 2 && testArgs[0] == "-z") {
            // Test if string is empty
            bool isEmpty = testArgs[1].empty();
            if (!isEmpty) {
                output << "test: false\n";
            }
        } else {
            output << "Usage: test -n \"string\" or test -z \"string\"\n";
        }
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
