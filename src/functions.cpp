#include <string>
#include <fstream>
#include "event.cpp"
#include <algorithm>
#include <filesystem>
#include <vector>
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

bool comparator(const Event& a, const Event& b){
    return a.start < b.start;
}

vector<Event> get_user_event(const string& username){
    string directory = "users/" + username + "/events.txt";
    if(not filesystem::exists(directory)){
        return {};
    }
    ifstream fin(directory);
    if(not fin.is_open()) return {};
    vector<Event> user_events;
    string line;
    while(getline(fin,line)){
        stringstream ss(line);        
        string id, title, start, end, user, description;
        getline(ss,id,'|');
        getline(ss,title,'|');
        getline(ss,start,'|');
        getline(ss,end,'|');
        getline(ss,user,'|');
        getline(ss,description,'|');
        user_events.emplace_back(title,id,start,end,user,description);
    }
    fin.close();
    sort(user_events.begin(), user_events.end(), comparator);
    return user_events;
}

int get_minutes(const string& timestamp){
    if(timestamp.length() < 16) return 0;
    int h = stoi(timestamp.substr(11,2));
    int m = stoi(timestamp.substr(14,2));
    return h * 60 + m;
}

string urlDecode(string str) {
    string ret;
    char ch;
    int i, ii;
    for (i=0; i<str.length(); i++) {
        if (str[i] == '%') {
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        } else if (str[i] == '+') {
            ret += ' ';
        } else {
            ret += str[i];
        }
    }
    return ret;
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
        // extern vector<Event> events; 
        // events.push_back(newevent);
        fout.close();
    }

}

void delete_event(const string &id, const string& user){
    string directory = "users/" + user + "/events.txt";
    if(not filesystem::exists(directory)){
        return;
    }
    vector<string> lines;
    ifstream fin(directory, ios::app);
    string line;
    while(getline(fin,line)){
        stringstream ss(line);
        string current_id;
        getline(ss,current_id,'|');
        if(current_id == id or line.empty()){
           continue; 
        }
        lines.push_back(line);
    }
    fin.close();
    ofstream fout(directory,ios::trunc);
    for(const auto &l : lines){
        fout << line;
    }
    fout.close();
}

void edit_event(const string& title, const string& id, const string& user, const string& start, const string &end, const string& description){
    string directory = "users/" + user + "/events.txt";
    if(not filesystem::exists(directory)){
        return;
    }

    vector<string> lines;
    ifstream fin(directory);
    string line;

    while(getline(fin,line)){
        if(line.empty()) continue;
        
        stringstream ss(line);
        string current_id;
        getline(ss, current_id, '|');

        if(current_id == id){
            Event updated(title, id, start, end, user, description);
            lines.push_back(updated.serialize());
        } else {
            lines.push_back(line);
        }
    }
    fin.close();

    ofstream fout(directory, ios::trunc);
    for(const auto& l : lines){
        fout << l << "\n";
    }
    fout.close();
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
