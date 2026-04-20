#include "tsg-billing.hpp"
#include <fmt/format.h>
#include <fstream>
#include <syslog.h>

TSGBilling::TSGBilling() {
    openlog("TSGbilling", LOG_PID | LOG_CONS, LOG_USER);

    std::ifstream file_cfg(m_file_cfg.data());
    if (!file_cfg.is_open()) {
        auto err = errno;
        syslog(LOG_ERR, "Не могу открыть настройки(%s): %s", m_file_cfg.data(), strerror(err));
        throw std::runtime_error(::fmt::format("Не могу открыть настройки({}): {}", m_file_cfg, strerror(err)));
    }
    ::nlohmann::json cfg;
    file_cfg >> cfg;
    file_cfg.close();
    m_config = cfg.get<Config>();
    m_config.port = cfg.value("port", 8080);
    m_config.path_db = cfg.at("path_db");
    setlogmask(LOG_UPTO(m_config.log_level));
    syslog(LOG_INFO, "Получены настройки:\n%s", cfg.dump(1).c_str());
}
void TSGBilling::run() {
    m_server.Get("/", [this](httplib::Request const &req, httplib::Response &res) {
        syslog(LOG_INFO, "Поступил запрос('/') от %s:%d на %s:%d", req.remote_addr.c_str(), req.remote_port,
               req.local_addr.c_str(), req.local_port);
        load_users();
        res.set_content(build_index_page(), "text/html; charset=utf-8");
    });

    // m_server.Get("/members", [this](const httplib::Request &, httplib::Response &res) {
    //     res.set_content(build_index_page(), "text/html; charset=utf-8");
    // });

    m_server.Post("/add", [this](const httplib::Request &req, httplib::Response &res) {
        Member m;
        auto const it =
                std::ranges::max_element(m_members, [](Member const &a, Member const &b) { return a.id < b.id; });
        if (it != m_members.end()) {
            m.id = it->id + 1;
        } else {
            m.id = 1;
        }

        m.fio = req.get_param_value("fio");
        m.area = to_double(req.get_param_value("area"));
        m.address = req.get_param_value("address");
        m.account = req.get_param_value("account");
        m.contribution = to_double(req.get_param_value("contribution"));
        m.recalculation = to_double(req.get_param_value("recalculation"));
        m.debt = to_double(req.get_param_value("debt"));

        m_members.push_back(m);
        save_data();
        res.set_redirect("/");
    });

    // server.Get("/edit", [this](const httplib::Request &req, httplib::Response &res) {
    //     if (!req.has_param("id")) {
    //         res.status = 404;
    //         res.set_content("Участник не найден", "text/plain; charset=utf-8");
    //         return;
    //     }
    //
    //     int id = std::stoi(req.get_param_value("id"));
    //     Member *m = find_member(id);
    //     if (!m) {
    //         res.status = 404;
    //         res.set_content("Участник не найден", "text/plain; charset=utf-8");
    //         return;
    //     }
    //
    //     res.set_content(build_edit_page(*m), "text/html; charset=utf-8");
    // });

    // server.Post("/update", [this](const httplib::Request &req, httplib::Response &res) {
    //     if (!req.has_param("id")) {
    //         res.status = 404;
    //         res.set_content("Участник не найден", "text/plain; charset=utf-8");
    //         return;
    //     }
    //
    //     int id = std::stoi(req.get_param_value("id"));
    //     Member *m = find_member(id);
    //     if (!m) {
    //         res.status = 404;
    //         res.set_content("Участник не найден", "text/plain; charset=utf-8");
    //         return;
    //     }
    //
    //     m->fio = req.get_param_value("fio");
    //     m->area = to_double(req.get_param_value("area"));
    //     m->address = req.get_param_value("address");
    //     m->account = req.get_param_value("account");
    //     m->contribution = to_double(req.get_param_value("contribution"));
    //     m->recalculation = to_double(req.get_param_value("recalculation"));
    //     m->debt = to_double(req.get_param_value("debt"));
    //
    //     save_data();
    //     res.set_redirect("/members");
    // });

    // server.Get("/delete", [this](const httplib::Request &req, httplib::Response &res) {
    //     if (req.has_param("id")) {
    //         int id = std::stoi(req.get_param_value("id"));
    //         g_members.erase(
    //                 std::remove_if(g_members.begin(), g_members.end(), [id](const Member &m) { return m.id == id; }),
    //                 g_members.end());
    //         save_data();
    //     }
    //     res.set_redirect("/members");
    // });

    // server.Get("/document", [this](const httplib::Request &req, httplib::Response &res) {
    //     if (!req.has_param("id")) {
    //         res.status = 404;
    //         res.set_content("Участник не найден", "text/plain; charset=utf-8");
    //         return;
    //     }
    //
    //     int id = std::stoi(req.get_param_value("id"));
    //     Member *m = find_member(id);
    //     if (!m) {
    //         res.status = 404;
    //         res.set_content("Участник не найден", "text/plain; charset=utf-8");
    //         return;
    //     }
    //
    //     res.set_content(build_member_document(*m), "text/html; charset=utf-8");
    // });

    syslog(LOG_NOTICE, "Запущен сервер на http://127.0.0.1:%d", m_config.port);
    m_server.listen("0.0.0.0", m_config.port);
}
void TSGBilling::load_users() {
    std::ifstream file(m_config.path_db);
    if (!file.is_open()) {
        auto err = errno;
        syslog(LOG_ERR, "Не могу открыть БД(%s): %s", m_config.path_db.c_str(), strerror(err));
        throw std::runtime_error(::fmt::format("Не могу открыть БД({}): {}", m_config.path_db.c_str(), strerror(err)));
    }

    ::nlohmann::json j;
    file >> j;
    syslog(LOG_INFO, "Получены пользователи из БД:\n%s", j.dump(2).c_str());

    m_members = j.get<members>();
}
void TSGBilling::save_data() {
    ::nlohmann::json j = ::nlohmann::json(m_members);

    std::ofstream file(m_config.path_db);
    if (!file.is_open()) {
        auto err = errno;
        syslog(LOG_ERR, "Не могу открыть БД(%s): %s", m_config.path_db.c_str(), strerror(err));
        throw std::runtime_error(::fmt::format("Не могу открыть БД({}): {}", m_config.path_db.c_str(), strerror(err)));
    }
    file << j;
    syslog(LOG_INFO, "Данные записаны в БД:\n%s", j.dump(2).c_str());
}
std::string TSGBilling::build_index_page() const {
    std::ostringstream out;
    out << R"html(<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ТСЖ — Управление участниками</title>
<style>
    :root {
        --bg1: #1f2a44;
        --bg2: #243b55;
        --text: #f5f5f5;
        --accent: #ffd166;
        --accent2: #06d6a0;
        --danger: #ef476f;
        --shadow: 0 12px 30px rgba(0,0,0,0.28);
        --card: rgba(255,255,255,0.08);
        --card2: rgba(255,255,255,0.06);
        --line: rgba(255,255,255,0.12);
    }
    * { box-sizing: border-box; }
    html, body {
        width: 100%;
        margin: 0;
        min-height: 100%;
        font-family: Cambria, serif;
        color: var(--text);
        overflow-x: hidden;
    }
    body {
        min-height: 100vh;
        background:
            radial-gradient(circle at top left, rgba(255,209,102,0.20), transparent 28%),
            radial-gradient(circle at bottom right, rgba(6,214,160,0.18), transparent 30%),
            linear-gradient(135deg, var(--bg1), var(--bg2));
        padding: 16px;
    }
    .wrapper {
        width: 100%;
        max-width: 100%;
        margin: 0 auto;
        background: rgba(255,255,255,0.05);
        backdrop-filter: blur(10px);
        border: 1px solid var(--line);
        border-radius: 28px;
        box-shadow: var(--shadow);
        padding: 24px;
    }
    .header {
        display: flex;
        justify-content: space-between;
        align-items: flex-end;
        gap: 20px;
        flex-wrap: wrap;
        margin-bottom: 18px;
    }
    h1 {
        margin: 0;
        font-size: 36px;
    }
    .subtitle {
        margin: 8px 0 0;
        font-size: 18px;
        opacity: 0.88;
    }

    .create-card {
        width: 100%;
        background: var(--card);
        border: 1px solid var(--line);
        border-radius: 24px;
        box-shadow: 0 10px 24px rgba(0,0,0,0.16);
        padding: 20px;
        margin-bottom: 20px;
    }
    .create-card-title {
        margin: 0 0 16px;
        font-size: 22px;
        font-weight: 700;
        color: #fff;
    }

    .form-grid {
        display: grid;
        grid-template-columns: 1fr;
        gap: 14px;
        margin-bottom: 14px;
    }
    .form-row {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
        gap: 14px;
    }
    .field {
        display: grid;
        gap: 8px;
    }
    label {
        font-size: 16px;
        font-weight: 700;
    }
    input {
        width: 100%;
        padding: 13px 14px;
        border-radius: 14px;
        border: 1px solid rgba(255,255,255,0.14);
        background: rgba(255,255,255,0.10);
        color: #fff;
        outline: none;
        font-family: Cambria, serif;
        font-size: 16px;
    }

    .create-actions {
        display: flex;
        justify-content: flex-start;
        align-items: center;
        margin-top: 6px;
    }
    .add-btn {
        display: inline-flex;
        align-items: center;
        justify-content: center;
        min-height: 54px;
        padding: 0 30px;
        border-radius: 18px;
        border: none;
        cursor: pointer;
        text-decoration: none;
        font-family: Cambria, serif;
        font-size: 18px;
        font-weight: 700;
        background: linear-gradient(135deg, var(--accent2), #118ab2);
        color: #fff;
        box-shadow: 0 8px 18px rgba(0,0,0,0.18);
    }
    .add-btn:hover { filter: brightness(1.05); }
    .add-btn:active { transform: translateY(1px); }

    .table-wrap {
        width: 100%;
        overflow: hidden;
        border-radius: 22px;
        border: 1px solid rgba(255,255,255,0.10);
    }
    table {
        width: 100%;
        border-collapse: collapse;
        table-layout: fixed;
        background: rgba(255,255,255,0.05);
        font-family: Cambria, serif;
    }
    th, td {
        padding: 12px 10px;
        border-bottom: 1px solid rgba(255,255,255,0.08);
        font-size: 16px;
        vertical-align: top;
        overflow-wrap: anywhere;
        word-break: break-word;
        font-family: Cambria, serif;
    }
    th {
        background: rgba(255,255,255,0.12);
        text-align: center;
    }
    .num-head,
    .num-cell { text-align: center; }
    .text-head,
    .text-cell { text-align: center; }
    tr:hover { background: rgba(255,255,255,0.08); }

    .actions {
        display: flex;
        gap: 6px;
        flex-wrap: nowrap;
        align-items: center;
        white-space: nowrap;
        justify-content: center;
    }
    .small-btn {
        min-height: 36px;
        padding: 0 10px;
        border-radius: 12px;
        font-size: 13px;
        color: #fff;
        text-decoration: none;
        display: inline-flex;
        align-items: center;
        justify-content: center;
        white-space: nowrap;
        flex: 0 0 auto;
        font-family: Cambria, serif;
    }
    .doc { background: linear-gradient(135deg, #8ecae6, #219ebc); }
    .edit { background: linear-gradient(135deg, #ffd166, #f4a261); color: #1f2a44; }
    .del { background: linear-gradient(135deg, var(--danger), #c9184a); }
    .empty { padding: 24px; text-align: center; opacity: 0.9; }

    .col-id { width: 70px; }
    .col-name { width: 20%; }
    .col-area { width: 85px; }
    .col-address { width: 26%; }
    .col-account { width: 120px; }
    .col-contribution { width: 85px; }
    .col-recalc { width: 90px; }
    .col-debt { width: 90px; }
    .col-actions { width: 260px; }

    @media (max-width: 1100px) {
        th, td { font-size: 14px; padding: 10px 8px; }
        .col-actions { width: 240px; }
    }
</style>
</head>
<body>
<div class="wrapper">
    <div class="header">
        <div>
            <h1>Участники ТСЖ</h1>
            <p class="subtitle">Добавление, просмотр, удаление, редактирование и генерация платёжного документа.</p>
        </div>
    </div>

    <div class="create-card">
        <h2 class="create-card-title">Создание нового участника</h2>
        <form action="/add" method="post">
            <div class="form-grid">
                <div class="field">
                    <label>ФИО</label>
                    <input name="fio" required placeholder="КУЗНЕЦОВ ВЛАДИМИР КОНСТАНТИНОВИЧ">
                </div>
                <div class="field">
                    <label>Адрес</label>
                    <input name="address" required placeholder="238050, Гусевский р-н, г. Гусев, ул.Школьная, д.17, кв.3">
                </div>
            </div>

            <div class="form-row">
                <div class="field"><label>Площадь квартиры</label><input name="area" required placeholder="78.6"></div>
                <div class="field"><label>Лицевой счёт</label><input name="account" required placeholder="269088081"></div>
                <div class="field"><label>Размер взноса</label><input name="contribution" required placeholder="8.9"></div>
                <div class="field"><label>Перерасчёт</label><input name="recalculation" required placeholder="0"></div>
                <div class="field"><label>Задолженность</label><input name="debt" required placeholder="0"></div>
            </div>

            <div class="create-actions">
                <button class="add-btn" type="submit">Создать участника</button>
            </div>
        </form>
    </div>

    <div class="table-wrap">
        <table>
            <thead>
            <tr>
                <th class="col-id num-head">Номер</th>
                <th class="col-name text-head">ФИО</th>
                <th class="col-area num-head">Площадь</th>
                <th class="col-address text-head">Адрес</th>
                <th class="col-account num-head">Лицевой счёт</th>
                <th class="col-contribution num-head">Взнос</th>
                <th class="col-recalc num-head">Перерасчёт</th>
                <th class="col-debt num-head">Долг</th>
                <th class="col-actions num-head">Действия</th>
            </tr>
            </thead>
            <tbody>
)html";

    if (m_members.empty()) {
        out << R"html(<tr><td class="empty" colspan="9">Список участников пока пуст.</td></tr>)html";
    } else {
        for (const auto &m: m_members) {
            out << "<tr>";
            out << "<td class=\"num-cell\">" << m.id << "</td>";
            out << "<td class=\"text-cell\">" << html_escape(m.fio) << "</td>";
            out << "<td class=\"num-cell\">" << m.area << "</td>";
            out << "<td class=\"text-cell\">" << html_escape(m.address) << "</td>";
            out << "<td class=\"num-cell\">" << html_escape(m.account) << "</td>";
            out << "<td class=\"num-cell\">" << m.contribution << "</td>";
            out << "<td class=\"num-cell\">" << format_money(m.recalculation) << "</td>";
            out << "<td class=\"num-cell\">" << format_money(m.debt) << "</td>";
            out << "<td class=\"num-cell\"><div class=\"actions\">";
            out << "<a class=\"small-btn doc\" href=\"/document?id=" << m.id << "\">Квитанция</a>";
            out << "<a class=\"small-btn edit\" href=\"/edit?id=" << m.id << "\">Редактировать</a>";
            out << "<a class=\"small-btn del\" href=\"/delete?id=" << m.id << "\">Удалить</a>";
            out << "</div></td>";
            out << "</tr>";
        }
    }

    out << R"html(
            </tbody>
        </table>
    </div>
</div>
</body>
</html>)html";
    return out.str();
}
std::string TSGBilling::html_escape(std::string_view s) const {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c: s) {
        switch (c) {
            case '&':
                out += "&amp;";
                break;
            case '<':
                out += "&lt;";
                break;
            case '>':
                out += "&gt;";
                break;
            case '"':
                out += "&quot;";
                break;
            case '\'':
                out += "&#39;";
                break;
            default:
                out += c;
                break;
        }
    }
    return out;
}
std::string TSGBilling::format_money(double x) const {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << x;
    std::string s = out.str();
    for (char &c: s) {
        if (c == '.')
            c = ',';
    }
    return s;
}
double TSGBilling::to_double(std::string_view s) {
    std::string t;
    t.reserve(s.size());
    for (auto const &symbol: s) {
        if (symbol == ',') {
            t.push_back('.');
        } else {
            t.push_back(symbol);
        }
    }
    return std::stod(t);
}
