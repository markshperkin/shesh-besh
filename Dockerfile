# Stage 1: builder
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    python3-pip \
    python3-venv \
    pkg-config \
    libssl-dev \
    zlib1g-dev \
    libjsoncpp-dev \
    uuid-dev \
    libbrotli-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Conan via pip (isolated venv to avoid PEP 668 restrictions)
RUN python3 -m venv /opt/conan-venv && \
    /opt/conan-venv/bin/pip install --upgrade pip conan

ENV PATH="/opt/conan-venv/bin:${PATH}"

# Init default Conan profile (detects compiler from environment)
RUN conan profile detect --force

WORKDIR /src
COPY conanfile.txt ./

# Install Conan dependencies into /src/build
RUN conan install . \
    --output-folder=build \
    --build=missing \
    -s build_type=Release

# Copy source
COPY CMakeLists.txt ./
COPY apps/ apps/
COPY src/ src/
COPY include/ include/

# Configure + build (only the server target; skip tests)
RUN cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \
    -DCMAKE_PREFIX_PATH=build && \
    cmake --build build --target server --config Release -j"$(nproc)"

# Stage 2: runtime
FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/apps/server/server /usr/local/bin/sheshbesh-server

EXPOSE 3000

CMD ["/usr/local/bin/sheshbesh-server"]
