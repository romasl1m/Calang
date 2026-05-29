#include <crow/app.h>
#include "registration.h"
#include "dashboard.h"
#include "core_routes.h"
#include "api.h"
#include "login.h"
#include <cstdlib>
// #include <string>
int main() {
    crow::SimpleApp app;

    core_routes(app);
    register_auth_routes(app);
    login_logout_routes(app);
    dashboard(app);
    api_routes(app);

    // app.port(8080).multithreaded().run();
    // app.port(8080).bindaddr("0.0.0.0").multithreaded().run();
    int port = 8080;

    if (const char *p = getenv("PORT")) {
        port = std::stoi(p);
    }

    app.port(port).multithreaded().run();

    return 0;
}
