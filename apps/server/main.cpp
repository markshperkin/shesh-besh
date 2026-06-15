#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <cstdlib>
#include <string>

int main() {
    // Read connection config from environment
    const char* db_url_env  = std::getenv("DATABASE_URL");
    const char* redis_url_env = std::getenv("REDIS_URL");

    const std::string database_url = db_url_env  ? db_url_env  : "";
    const std::string redis_url    = redis_url_env ? redis_url_env : "";

    auto& app = drogon::app();

    // Postgres connection pool
    if (!database_url.empty()) {
        // DATABASE_URL format: postgresql://user:pass@host:port/dbname
        app.addDbClient(drogon::orm::DbClientBuilder{}
            .connectionString(database_url)
            .connectionNumber(10)
            .build());
        LOG_INFO << "Postgres: connecting via DATABASE_URL";
    } else {
        LOG_WARN << "DATABASE_URL not set — skipping Postgres connection";
    }

    // Redis client
    if (!redis_url.empty()) {
        app.addRedisClient(redis_url, "default", "", 5);
        LOG_INFO << "Redis: connecting via REDIS_URL";
    } else {
        LOG_WARN << "REDIS_URL not set — skipping Redis connection";
    }

    // Health endpoint
    app.registerHandler(
        "/health",
        [](const drogon::HttpRequestPtr&,
           std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(
                Json::Value{} );
            Json::Value body;
            body["status"] = "ok";
            resp = drogon::HttpResponse::newHttpJsonResponse(body);
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
