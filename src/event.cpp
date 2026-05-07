#include <string>
using namespace std;
class Event{
public:
    string title;
    string id;
    string start,end;
    string user;
    string description;
    string serialize() const{
        return id + "|" + title + "|" + start + "|" + end + "|" + user + "|" + description;
    }
    Event(string p_title, string p_id, string p_start, string p_end, string p_user, string p_description) :
        title(p_title), id(p_id),start(p_start), end(p_end), user(p_user), description(p_description) {} 
    Event() = default; 
};
