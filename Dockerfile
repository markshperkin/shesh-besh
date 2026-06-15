# Stage 1: build using official Drogon image (has Drogon + trantor pre-installed)
FROM drogonframework/drogon:latest AS builder

WORKDIR /src

COPY CMakeLists.txt ./
COPY apps/ apps/
COPY src/ src/
COPY include/ include/

RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF && \
    cmake --build build --target server --config Release -j"$(nproc)"

# Stage 2: runtime — ubuntu:24.04 with only the needed shared libs
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    libjsoncpp25 \
    libpq5 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/apps/server/server /usr/local/bin/sheshbesh-server

EXPOSE 3000

CMD ["/usr/local/bin/sheshbesh-server"]
