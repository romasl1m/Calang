#include <crow.h>
#include "registration.h"
#include "dashboard.h"
#include "core_routes.h"
#include "api.h"
#include "login.h"

int main(){
    crow::SimpleApp app;

    core_routes(app);
    register_auth_routes(app);
    login_logout_routes(app);
    dashboard(app);        
    api_routes(app);

    app.port(8080).multithreaded().run();
    
    return 0;
}
