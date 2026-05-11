#include <string>
#include <unordered_map>
#include <random>
using namespace std;
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
