#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Event {
    std::string id;
    std::string title;
    std::string start;
    std::string end;
    std::string user;
    std::string description;
    std::string origin;
    std::string recurrence;

    Event(std::string t, std::string i, std::string s, std::string e, std::string u, std::string d, std::string o, std::string r)
        : title(t), id(i), start(s), end(e), user(u), description(d), origin(o), recurrence(r) {}
    
    Event() = default;
};

inline void to_json(json& j, const Event& e){
    j = json{{"id", e.id}, {"title", e.title}, {"start", e.start}, {"end", e.end}, {"user", e.user}, {"description", e.description}, {"origin", e.origin}, {"recurrence", e.recurrence}};
}

inline void from_json(const json& j, Event& e){
    e.id = j.value("id", "");
    e.title = j.value("title", "");
    e.start = j.value("start", "");
    e.end = j.value("end", "");
    e.user = j.value("user", "");
    e.description = j.value("description", "");
    e.origin = j.value("origin", "private");
    e.recurrence = j.value("recurrence", "none");
}

#endif