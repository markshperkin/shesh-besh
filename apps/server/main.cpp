#include <drogon/drogon.h>
#include <trantor/utils/Logger.h>
#include <cstdlib>
#include <string>

static std::string env(const char* key, const char* fallback = "") {
    const char* v = std::getenv(key);
    return v ? v : fallback;
}

int main() {
    auto& app = drogon::app();

    // Postgres — individual params required by Drogon API
    const std::string pg_host = env("POSTGRES_HOST", "localhost");
    const std::string pg_db   = env("POSTGRES_DB",   "sheshbesh");
    const std::string pg_user = env("POSTGRES_USER",  "sheshbesh");
    const std::string pg_pass = env("POSTGRES_PASSWORD", "");
    const unsigned short pg_port =
        static_cast<unsigned short>(std::stoi(env("POSTGRES_PORT", "5432")));

    app.createDbClient("postgresql", pg_host, pg_port, pg_db, pg_user, pg_pass,
                       /*connectionNum=*/10, /*filename=*/"", /*name=*/"default");
    LOG_INFO << "Postgres: pool configured for " << pg_host << ":" << pg_port;

    // Redis
    const std::string redis_host = env("REDIS_HOST", "127.0.0.1");
    const unsigned short redis_port =
        static_cast<unsigned short>(std::stoi(env("REDIS_PORT", "6379")));
    app.createRedisClient(redis_host, redis_port);
    LOG_INFO << "Redis: client configured for " << redis_host << ":" << redis_port;

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
