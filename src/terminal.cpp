#include <iostream>
#include <string>
#include <sstream>
#include "functions.h"
#include <vector>
#include <ctime>

using namespace std;

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
               << "  touch \"title\" \"start\" \"end\" \"desc\" \"origin\"\n"
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
        if(args.size() < 5){
            output << "Syntax error. Correct usage of 'touch':\n  touch \"title\" \"start\" \"end\" \"desc\" \"origin\"\n";
        } else {
            string title  = args[0];
            string start  = formatDateTime(args[1]);
            string end    = formatDateTime(args[2]);
            string desc   = args[3];
            string origin = args[4];
            string id = to_string(time(0)) + "_" + to_string(rand()%1000);
            string path = (origin == "private") ? "users/" + currentUsername + "/events.json" : "groups/" + origin + "/events.json";
            add_new_event(title, id,start,end,currentUsername, desc, origin); 
            output << "An event created!\n"
                   << "-> Title: " << title << "\n"
                   << "-> Start: " << start << "\n"
                   << "-> End: "   << end << "\n"
                   << "-> Desc:   " << desc << "\n"
                   << "-> Type:   " << origin << "\n";
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
                (void)system("clear");
            #endif
        }
        else if(cmd == "whoami"){
            cout << username << endl;
        }
        else if(cmd == "exit"){
            break;
        }
        else if(cmd == "help"){
            cout << "Available commands:\n  cat {YYYY-MM-DD}\n  touch \"title\" \"start\" \"end\" \"desc\" \"origin\"\n  clear\n  whoami" << endl;
        }
        else {
            cout << process_terminal_command(line, username);
        }
    }
}

