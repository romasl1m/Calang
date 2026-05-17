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

    // Konstruktor
    Event(std::string t, std::string i, std::string s, std::string e, std::string u, std::string d, std::string o)
        : title(t), id(i), start(s), end(e), user(u), description(d), origin(o) {}
    
    // Pusty konstruktor (wymagany przez niektóre operacje nlohmann::json)
    Event() = default;
};

// Używamy inline, aby uniknąć błędów multiple definition
inline void to_json(json& j, const Event& e){
    j = json{{"id", e.id}, {"title", e.title}, {"start", e.start}, {"end", e.end}, {"user", e.user}, {"description", e.description}, {"origin", e.origin}};
}

inline void from_json(const json& j, Event& e){
    j.at("id").get_to(e.id);
    j.at("title").get_to(e.title);
    j.at("start").get_to(e.start);
    j.at("end").get_to(e.end);
    j.at("user").get_to(e.user);
    j.at("description").get_to(e.description);
    j.at("origin").get_to(e.origin);
}

#endif
