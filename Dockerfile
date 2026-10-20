# Образ tsg-billing: сборка в debian:trixie, запуск в debian:trixie-slim от непривилегированного пользователя.
# База - в томе /data, настройки /etc/tsg-billing/cfg.json (монтируются с хоста, пример docker/cfg.json).
FROM debian:trixie AS build
RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake pkg-config ca-certificates nlohmann-json3-dev libqrencode-dev \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY . .
RUN cmake -S . -B /build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
    && cmake --build /build -j"$(nproc)" \
    && DESTDIR=/out cmake --install /build --strip

FROM debian:trixie-slim
RUN apt-get update \
    && apt-get install -y --no-install-recommends libqrencode4 \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --system --uid 10001 --home-dir /data --shell /usr/sbin/nologin tsg-billing \
    && install -d -o tsg-billing -g tsg-billing -m 0750 /data
COPY --from=build /out/usr/bin/tsg-billing /usr/bin/tsg-billing
COPY docker/cfg.json /etc/tsg-billing/cfg.json
COPY LICENSE /usr/share/doc/tsg-billing/LICENSE
USER tsg-billing
VOLUME /data
EXPOSE 8080
ENTRYPOINT ["/usr/bin/tsg-billing", "/etc/tsg-billing/cfg.json"]
