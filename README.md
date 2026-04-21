# TSG billing

![TSG preview](logo/logo.png)

**TSG billing** — это веб-приложение для управления участниками ТСЖ и формирования платёжных документов.  
Система позволяет добавлять, редактировать и удалять пользователей, хранить данные в JSON-базе и печатать квитанции на основе конфигурации.

## Требования

### Для сборки

- CMake 3.26+
- C++20
- `fmt`
- `nlohmann-json`
- `libcpp-httplib`

````bash
apt install -y build-essential cmake libfmt-dev nlohmann-json3-dev dpkg-dev libcpp-httplib-dev
````

## Сборка из исходников

```bash 
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ..
cmake --build . -j$(nproc)
cmake --install .
```

## Сборка DEB

Пакет собирается через CPack:

```bash 
cd build cpack -G DEB
apt install -y ./tsg-billing_<версия>_amd64.deb
```

После установки будут размещены:

- бинарник: `/usr/bin/tsg-billing`
- конфиг(Настройки): `/etc/tsg-billing/cfg.json`
- unit-файл systemd: `/usr/lib/systemd/system/tsg-billing.service`
- БД: `/var/tsg-billing/tsg-billing.json`

### БД tsg-billing.json
````json
[
  {
    "account": "269088079",
    "address": "238050, Гусевский р-н, г. Гусев, ул.Пушкина, д.1, кв.1",
    "area": 55.5,
    "contribution": 9.9,
    "debt": 200.0,
    "fio": "Иванов Иван Иванович",
    "id": 1,
    "recalculation": 100.0
  },
  {
    "account": "269088080",
    "address": "238050, Гусевский р-н, г. Гусев, ул.Пушкина, д.1, кв.2",
    "area": 75.5,
    "contribution": 9.9,
    "debt": 100.0,
    "fio": "Иванов Иван Иванович2",
    "id": 2,
    "recalculation": 20.0
  },
  {
    "account": "269088081",
    "address": "238050, Гусевский р-н, г. Гусев, ул.Пушкина, д.1, кв.3",
    "area": 65.5,
    "contribution": 9.9,
    "debt": 10.0,
    "fio": "Иванов Иван Иванович",
    "id": 3,
    "recalculation": 50.0
  }
]
````
- `id` — номер участника
- `fio` — ФИО участника
- `area` — площадь квартиры, кв. м
- `address` — адрес объекта собственности
- `account` — лицевой счёт
- `contribution` — размер взноса за 1 кв. м
- `recalculation` — сумма перерасчёта
- `debt` — задолженность

### Параметры cfg.json

````json
{
  "port": 80,
  "log_level": 6,
  "path_db": "/var/tsg-billing/tsg-billing.json",
  "receiver_name": "ТСЖ \"Рога и Копыта\"",
  "receiver_details": "ИНН 3902202236, КПП 390201001, р\\с 40305810822050000170, к\\с 20101210500777000878, БИК 042748900"
}
````

- `port` — порт, на котором запускается HTTP-сервер
- `log_level` — уровень логирования для `syslog`
- `path_db` — путь до БД
- `receiver_name` — наименование получателя платежей
- `receiver_details` — детали получателя платежей

Возможные уровнилогирования:

- `LOG_EMERG` — 0
- `LOG_ALERT` — 1
- `LOG_CRIT` — 2
- `LOG_ERR` — 3
- `LOG_WARNING` — 4
- `LOG_NOTICE` — 5
- `LOG_INFO` — 6
- `LOG_DEBUG` — 7

## Запуск

После установки сервис можно запускать так:

```bash 
systemctl daemon-reload
systemctl enable tsg-billing
systemctl start tsg-billing
```

Проверка статуса:

```bash 
systemctl status tsg-billing
```

По умолчанию сервер доступен на: http://localhost
