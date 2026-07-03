# ============================================================
# RiemannFHE Production Docker Image
# Noise-Free FHE on the Riemann Critical Line
# φΩ0 — Primordial Omega Zero
# ============================================================

FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN make prod -j$(nproc)

# Runtime stage
FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/ /app/build/
COPY --from=builder /app/*.hpp /app/

WORKDIR /app

EXPOSE 8443

CMD ["/app/build/riemann_enterprise"]
