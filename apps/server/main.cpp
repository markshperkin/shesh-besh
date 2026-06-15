#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <cstdlib>
#include <string>

int main() {
    const char* db_url_env    = std::getenv("DATABASE_URL");
    const char* redis_host_env = std::getenv("REDIS_HOST");
    const char* redis_port_env = std::getenv("REDIS_PORT");

    auto& app = drogon::app();

    // Postgres — Drogon 1.9 API: createDbClient with connection string
    if (db_url_env && *db_url_env) {
        app.createDbClient("postgresql", db_url_env, "default", 10, false);
        LOG_INFO << "Postgres: pool configured via DATABASE_URL";
    } else {
        LOG_WARN << "DATABASE_URL not set — skipping Postgres";
    }

    // Redis — Drogon 1.9 API: takes host + port separately
    {
        const std::string host = redis_host_env ? redis_host_env : "127.0.0.1";
        const unsigned short port = redis_port_env ? static_cast<unsigned short>(std::stoi(redis_port_env)) : 6379;
        app.createRedisClient(host, port);
        LOG_INFO << "Redis: client configured for " << host << ":" << port;
    }

    // Health endpoint
    app.registerHandler(
        "/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            Json::Value body;
            body["status"] = "ok";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
            resp->setStatusCode(drogon::k200OK);
            callback(resp);
        },
        {drogon::Get});

    app.setLogLevel(trantor::Logger::kInfo)
       .setThreadNum(4)
       .addListener("0.0.0.0", 3000)
       .run();

    return 0;
}
