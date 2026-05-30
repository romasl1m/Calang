#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <nlohmann/json.hpp>
#include "functions.h"

using namespace std;
using json = nlohmann::json;

string getCurrentYear(){
    time_t t = time(nullptr);
    tm* now = localtime(&t);
    return to_string(now->tm_year + 1900);
}

vector<string> parseArguments(const string& argumentStr){
    vector<string> args;
    string current;
    bool inQuotes = false;

    for(char c : argumentStr){
        if(c == '"'){
            inQuotes = !inQuotes;
        } else if(c == ' ' and not inQuotes){
            if(!current.empty()){
                args.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if(not current.empty()){
        args.push_back(current);
    }
    return args;
}

string formatDateTime(const string& rawDateTime){
    if(rawDateTime.length() == 11 and rawDateTime[5] == ' '){ 
        return getCurrentYear() + "-" + rawDateTime; 
    }
    return rawDateTime; 
}

string parseAndFormatDateTime(const string& dateStr, const string& timeStr) {
    int day = 0, month = 0;
    char dot = 0;
    if (sscanf(dateStr.c_str(), "%d%c%d", &day, &dot, &month) == 3 && dot == '.') {
        char buf[64];
        snprintf(buf, sizeof(buf), "%s-%02d-%02d %s", getCurrentYear().c_str(), month, day, timeStr.c_str());
        return string(buf);
    }
    return dateStr + " " + timeStr;
}

string process_terminal_command(const string& fullLine, const string& currentUsername){
    if(fullLine.empty()) return "";

    stringstream ss(fullLine);
    string cmd;
    ss >> cmd;

    stringstream output;

    if(cmd == "clear"){
        return "CLEAR_SIGNAL"; 
    }
    else if(cmd == "whoami"){
        output << currentUsername << "\n";
    }
    else if(cmd == "help"){
        output << "Available commands:\n"
               << "  cat {YYYY-MM-DD}\n"
               << "  touch \"event name\" DD.MM HH:MM HH:MM \"description\" \"origin\"\n"
               << "  clear\n"
               << "  whoami\n";
    }
    else if(cmd == "cat"){
        string date;
        ss >> date;
        if(date.length() == 5){
            date = getCurrentYear() + "-" + date;
        }
        output << "--- This day events: " << date << " ---\n";
    }
    else if(cmd == "touch"){
        string remaining;
        getline(ss, remaining);
        vector<string> args = parseArguments(remaining);
        if(args.size() < 6){
            output << "Syntax error. Correct usage of 'touch':\n  touch \"event name\" DD.MM HH:MM HH:MM \"description\" \"origin\"\n";
        } else {
            string title  = args[0];
            string start  = parseAndFormatDateTime(args[1], args[2]);
            string end    = parseAndFormatDateTime(args[1], args[3]);
            string desc   = args[4];
            string origin = args[5];
            string recurrence = "none";
            for(size_t i = 6; i < args.size(); i++){
                if(args[i].rfind("T=", 0) == 0){
                    recurrence = args[i].substr(2);
                }
            }
            string resolved_origin = origin;
            
            if(origin != "private"){
                bool found = false;
                vector<json> groups = get_user_groups(currentUsername);
                for(const auto& g : groups){
                    if(g.value("name", "") == origin or g.value("id", "") == origin){
                        resolved_origin = g.value("id", "");
                        found = true;
                        break;
                    }
                }
                if(not found){
                    output << "Error: Group '" << origin << "' not found or you are not a member.\n";
                    return output.str();
                }
            }

            string id = to_string(time(0)) + "_" + to_string(rand()%1000);
            
            add_new_event(title, id, start, end, currentUsername, desc, resolved_origin, recurrence); 
            
            output << "An event created!\n"
                   << "-> Title: " << title << "\n"
                   << "-> Start: " << start << "\n"
                   << "-> End: "   << end << "\n"
                   << "-> Desc:   " << desc << "\n"
                   << "-> Type:   " << origin << " (ID: " << resolved_origin << ")\n"
                    << "-> Recur:  " << recurrence << "\n";
        }
    }
    else {
        output << "Unknown command. Try 'help'.\n";
    }

    return output.str();
}

void executeTerminal(){
    string line;
    string username = "user";

    while(true){
        cout << username << "@calang:~$ ";
        if(!getline(cin, line)) break;

        if(line.empty()) continue;

        stringstream ss(line);
        string cmd;
        ss >> cmd;

        if(cmd == "clear"){
            #ifdef _WIN32
                (void)system("cls");
            #else
                int result = system("clear");
                (void)result; // Ucisza kompilator
            #endif
        }
        else if(cmd == "whoami"){
            cout << username << endl;
        }
        else if(cmd == "exit"){
            break;
        }
        else if(cmd == "help"){
            cout << "Available commands:\n  cat {YYYY-MM-DD}\n  touch \"event name\" DD.MM HH:MM HH:MM \"description\" \"origin\"\n  clear\n  whoami" << endl;
        }
        else {
            cout << process_terminal_command(line, username);
        }
    }
}

