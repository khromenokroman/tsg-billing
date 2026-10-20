#!/bin/bash
# Проверка Docker-образа: запуск от непривилегированного пользователя, настройки с хоста, журнал в
# docker logs, база в томе /data, быстрая остановка по docker stop, данные после пересоздания контейнера.
#
# Использование: tests/docker_run.sh <образ>
set -u
IMAGE=$1
PORT=${PORT:-18096}
NAME=tsg-billing-test-$$
VOLUME=tsg-billing-test-$$
CFG=$(mktemp -d)
fail=0
ok() { echo "ok    $*"; }
bad() { echo "FAIL  $*"; fail=1; }
cleanup() { docker rm -f "$NAME" >/dev/null 2>&1; docker volume rm -f "$VOLUME" >/dev/null 2>&1; rm -rf "$CFG"; }
trap cleanup EXIT
sed 's/"receiver_name": ""/"receiver_name": "ТСЖ Проверка"/' "$(dirname "$0")/../docker/cfg.json" > "$CFG/cfg.json"
chmod 0644 "$CFG/cfg.json"
start() { docker run -d --name "$NAME" -p "127.0.0.1:$PORT:8080" -v "$VOLUME:/data" -v "$CFG/cfg.json:/etc/tsg-billing/cfg.json:ro" "$IMAGE" >/dev/null; }
wait_http() { for _ in $(seq 100); do curl -sf "http://127.0.0.1:$PORT/" >/dev/null && return 0; sleep 0.1; done; return 1; }
URL=http://127.0.0.1:$PORT

start
wait_http && ok "отвечает на пустом томе" || { bad "не отвечает"; docker logs "$NAME" | tail -5; exit 1; }
[ "$(docker exec "$NAME" id -u)" = 10001 ] && ok "работает не от root (uid 10001)" || bad "пользователь: $(docker exec "$NAME" id)"
docker logs "$NAME" 2>&1 | grep -q 'Запущен сервер' && ok "журнал в docker logs" || bad "журнал: $(docker logs "$NAME" 2>&1 | tail -3)"
curl -s -o /dev/null -X POST "$URL/add" --data-urlencode 'fio=Петров Пётр' \
    -d 'area=50&address=a&account=100&contribution=9.9&recalculation=0&debt=0'
docker exec "$NAME" grep -q 'Петров' /data/tsg-billing.json && ok "запись в /data/tsg-billing.json" || bad "запись в базу"
curl -s "$URL/documents" | grep -q 'ТСЖ Проверка' && ok "настройки с хоста в квитанции" || bad "нет реквизитов из cfg.json"

begin=$(date +%s)
docker stop -t 10 "$NAME" >/dev/null
spent=$(($(date +%s) - begin))
code=$(docker inspect -f '{{.State.ExitCode}}' "$NAME")
[ "$spent" -lt 5 ] && [ "$code" = 0 ] && ok "docker stop: остановлен за ${spent} с, код 0" || bad "docker stop: ${spent} с, код $code"
docker rm "$NAME" >/dev/null

start
wait_http || bad "после пересоздания не отвечает"
curl -s "$URL/" | grep -q 'Петров' && ok "данные на месте после пересоздания контейнера" || bad "после пересоздания нет данных"
exit $fail
