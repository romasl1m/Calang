#pragma once
#include <crow/app.h>
#include <string>

void register_google_oauth_routes(crow::SimpleApp &app);
std::string createGoogleCalendarEvent(const std::string &access_token,
                                      const std::string &title,
                                      const std::string &start_datetime,
                                      const std::string &end_datetime,
                                      const std::string &description);
std::string get_user_access_token(const std::string &email);
