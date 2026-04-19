#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <httplib.h>
#include <nlohmann/json.hpp>


struct Member {
    int id{};
    std::string fio;
    double area{};
    std::string address;
    std::string account;
    double contribution{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Member, id, fio, area, address, account, contribution)
};

static const std::string kDataFile = "members.json";

static std::vector<Member> g_members;
static int g_next_id = 1;

static std::string html_escape(const std::string &s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c; break;
        }
    }
    return out;
}

static std::string url_decode(std::string s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') {
            out += ' ';
        } else if (s[i] == '%' && i + 2 < s.size()) {
            std::string hex = s.substr(i + 1, 2);
            char ch = static_cast<char>(std::strtol(hex.c_str(), nullptr, 16));
            out += ch;
            i += 2;
        } else {
            out += s[i];
        }
    }
    return out;
}

static double to_double(const std::string &s) {
    std::string t = s;
    for (char &c : t) {
        if (c == ',') c = '.';
    }
    return std::stod(t);
}

static std::string format_money(double x) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << x;
    std::string s = out.str();
    for (char &c : s) {
        if (c == '.') c = ',';
    }
    return s;
}


static void save_data() {
    ::nlohmann::json j = ::nlohmann::json::array();
    for (const auto &m : g_members) {
        j.push_back(::nlohmann::json(m));
    }

    std::ofstream file(kDataFile);
    if (file) {
        file << std::setw(4) << j;
    }
}

static void load_data() {
    std::ifstream file(kDataFile);
    if (!file) return;

    ::nlohmann::json j;
    file >> j;

    g_members.clear();
    for (const auto &item : j) {
        g_members.push_back(::nlohmann::json(item));
    }

    int max_id = 0;
    for (const auto &m : g_members) {
        max_id = std::max(max_id, m.id);
    }
    g_next_id = max_id + 1;
}

static Member* find_member(int id) {
    for (auto &m : g_members) {
        if (m.id == id) return &m;
    }
    return nullptr;
}

static std::string build_index_page() {
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
        --card: rgba(255,255,255,0.08);
        --text: #f5f5f5;
        --accent: #ffd166;
        --accent2: #06d6a0;
        --danger: #ef476f;
        --shadow: 0 12px 30px rgba(0,0,0,0.28);
    }
    * { box-sizing: border-box; }
    body {
        margin: 0;
        min-height: 100vh;
        font-family: Cambria, serif;
        color: var(--text);
        background:
            radial-gradient(circle at top left, rgba(255,209,102,0.20), transparent 28%),
            radial-gradient(circle at bottom right, rgba(6,214,160,0.18), transparent 30%),
            linear-gradient(135deg, var(--bg1), var(--bg2));
        padding: 24px;
    }
    .wrapper {
        width: 100%;
        max-width: 1280px;
        margin: 0 auto;
        background: rgba(255,255,255,0.06);
        backdrop-filter: blur(10px);
        border: 1px solid rgba(255,255,255,0.12);
        border-radius: 28px;
        box-shadow: var(--shadow);
        padding: 34px;
    }
    .header {
        display: flex;
        justify-content: space-between;
        align-items: flex-end;
        gap: 20px;
        flex-wrap: wrap;
        margin-bottom: 24px;
    }
    h1 { margin: 0; font-size: 40px; }
    .subtitle { margin: 8px 0 0; font-size: 18px; opacity: 0.88; }
    .top-actions { display: flex; gap: 12px; flex-wrap: wrap; }
    .nav-btn, button {
        display: inline-flex;
        align-items: center;
        justify-content: center;
        min-height: 54px;
        padding: 0 20px;
        border-radius: 18px;
        text-decoration: none;
        font-family: Cambria, serif;
        font-size: 18px;
        font-weight: 700;
        cursor: pointer;
        border: none;
    }
    .nav-btn {
        color: var(--text);
        background: rgba(255,255,255,0.10);
        border: 1px solid rgba(255,255,255,0.14);
    }
    .add-btn {
        background: linear-gradient(135deg, var(--accent2), #118ab2);
        color: #fff;
    }
    .table-wrap {
        overflow-x: auto;
        border-radius: 22px;
        border: 1px solid rgba(255,255,255,0.10);
    }
    table {
        width: 100%;
        border-collapse: collapse;
        min-width: 1000px;
        background: rgba(255,255,255,0.05);
    }
    th, td {
        padding: 14px 16px;
        border-bottom: 1px solid rgba(255,255,255,0.08);
        text-align: left;
        font-size: 17px;
    }
    th { background: rgba(255,255,255,0.12); }
    tr:hover { background: rgba(255,255,255,0.08); }
    .actions { display: flex; gap: 10px; flex-wrap: wrap; }
    .small-btn {
        min-height: 40px;
        padding: 0 14px;
        border-radius: 14px;
        font-size: 15px;
        color: #fff;
        text-decoration: none;
    }
    .doc { background: linear-gradient(135deg, #8ecae6, #219ebc); }
    .del { background: linear-gradient(135deg, var(--danger), #c9184a); }
    .empty { padding: 24px; text-align: center; opacity: 0.9; }
    .form-grid {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
        gap: 14px;
        margin-bottom: 18px;
    }
    .field { display: grid; gap: 8px; }
    label { font-size: 16px; font-weight: 700; }
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
</style>
</head>
<body>
<div class="wrapper">
    <div class="header">
        <div>
            <h1>Участники ТСЖ</h1>
            <p class="subtitle">Добавление, просмотр, удаление и генерация платёжного документа.</p>
        </div>
        <div class="top-actions">
            <a class="nav-btn" href="/">Главная</a>
            <a class="nav-btn" href="/members">Обновить список</a>
        </div>
    </div>

    <form action="/add" method="post">
        <div class="form-grid">
            <div class="field"><label>ФИО</label><input name="fio" required placeholder="КУЗНЕЦОВ ВЛАДИМИР КОНСТАНТИНОВИЧ"></div>
            <div class="field"><label>Площадь квартиры</label><input name="area" required placeholder="78.6"></div>
            <div class="field"><label>Адрес</label><input name="address" required placeholder="238050. ..."></div>
            <div class="field"><label>Лицевой счёт</label><input name="account" required placeholder="269088081"></div>
            <div class="field"><label>Размер взноса</label><input name="contribution" required placeholder="8.9"></div>
        </div>
        <button class="add-btn" type="submit">Создать участника</button>
    </form>

    <div style="height: 24px;"></div>

    <div class="table-wrap">
        <table>
            <thead>
            <tr>
                <th>ID</th>
                <th>ФИО</th>
                <th>Площадь</th>
                <th>Адрес</th>
                <th>Лицевой счёт</th>
                <th>Взнос</th>
                <th>Действия</th>
            </tr>
            </thead>
            <tbody>
)html";

    if (g_members.empty()) {
        out << R"html(<tr><td class="empty" colspan="7">Список участников пока пуст.</td></tr>)html";
    } else {
        for (const auto &m : g_members) {
            out << "<tr>";
            out << "<td>" << m.id << "</td>";
            out << "<td>" << html_escape(m.fio) << "</td>";
            out << "<td>" << m.area << "</td>";
            out << "<td>" << html_escape(m.address) << "</td>";
            out << "<td>" << html_escape(m.account) << "</td>";
            out << "<td>" << m.contribution << "</td>";
            out << "<td><div class=\"actions\">";
            out << "<a class=\"small-btn doc\" href=\"/document?id=" << m.id << "\">Документ</a>";
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

static std::string build_member_document(const Member &m) {
    std::ostringstream out;

    auto now = std::chrono::system_clock::now();
    std::time_t tt = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
    tm = *std::localtime(&tt);

    int month = tm.tm_mon + 1;
    int year = tm.tm_year + 1900;
    int next_month = month + 1;
    int next_year = year;
    if (next_month == 13) {
        next_month = 1;
        ++next_year;
    }

    std::ostringstream date_out;
    date_out << std::setfill('0') << std::setw(2) << month << "." << year;

    std::ostringstream next_date_out;
    next_date_out << std::setfill('0') << std::setw(2) << next_month << "." << next_year;

    auto build_one_document = [&](const std::string &period) {
        std::ostringstream doc;

        doc << R"html(<div class="paper">
<div class="topline"><div>Адрес: )html";
        doc << html_escape(m.address);
        doc << R"html(</div><div class="date-line">Платежный документ за )html";
        doc << period;
        doc << R"html(</div></div>

<table class="meta">
<tr>
    <th>№ лицевого счета</th>
    <th>Период</th>
    <th>К оплате</th>
</tr>
<tr>
    <td>)html";
        doc << html_escape(m.account);
        doc << R"html(</td>
    <td>)html";
        doc << period;
        doc << R"html(</td>
    <td>)html";
        doc << format_money(m.area * m.contribution);
        doc << R"html(</td>
</tr>
</table>

<div class="doc-info"><b>Плательщик:</b> )html";
        doc << html_escape(m.fio);
        doc << R"html(</div>
<div class="doc-info"><b>Адрес объекта собственности:</b> )html";
        doc << html_escape(m.address);
        doc << R"html(</div>
<div class="doc-info"><b>Получатель платежа:</b> ТСЖ "Школьная, 17"</div>
<div class="doc-info"><b>Реквизиты:</b> ИНН 3902800956, КПП 390201001, р\с 40705810755050000170, к\с 30101810500000000878, БИК 042748878</div>
<div class="doc-info"><b>Назначение платежа:</b> Взнос на капитальный ремонт общего имущества в многоквартирном доме</div>

<table class="calc">
<tr>
    <th>Название услуги</th>
    <th>Ед. изм.</th>
    <th>Площадь</th>
    <th>Размер взноса</th>
    <th>Сумма</th>
    <th>Перерасчет</th>
    <th>Итого</th>
</tr>
<tr>
    <td>Взнос на капитальный ремонт общего имущества в многоквартирном доме</td>
    <td class="center">кв.м</td>
    <td class="right">)html";
        doc << std::fixed << std::setprecision(1) << m.area;
        doc << R"html(</td>
    <td class="right">)html";
        doc << std::fixed << std::setprecision(1) << m.contribution;
        doc << R"html(</td>
    <td class="right">)html";
        doc << format_money(m.area * m.contribution);
        doc << R"html(</td>
    <td class="right">-</td>
    <td class="right">)html";
        doc << format_money(m.area * m.contribution);
        doc << R"html(</td>
</tr>
</table>

<div>Задолженность: -</div>
<div><b>Итого к оплате:</b> )html";
        doc << format_money(m.area * m.contribution);
        doc << R"html(</div>

<div class="note-strong">УВАЖАЕМЫЕ СОБСТВЕННИКИ ПОМЕЩЕНИЙ!</div><p class="note">В соответствии с Законом Калининградской области от 26.12.2013 № 293, от 19.12.2016 № 42, минимальный размер взноса на капитальный ремонт на 2015,2016,2017 года установлен в размере 5,9 руб. за один квадратный метр общей площади помещения в многоквартирном доме. В соответствии с ч.14.1. ст.155 ЖК РФ собственники помещений в многоквартирном доме, несвоевременно и (или) не полностью уплатившие взносы на капитальный ремонт, обязаны уплатить пени в размере ставки рефинансирования ЦБ РФ. Если оплата не производится, то задолженность взыскивается в судебном порядке, при этом к сумме задолженности и пени прибавляется сумма понесенных судебных расходов. Телефон для справок: 89216190701 Прием звонков: понедельник – пятница с 08.00-17.00</p><div class="note-strong">ОПЛАТА ПРОИЗВОДИТСЯ ПО АДРЕСУ: Г. ГУСЕВ, УЛ. ПОБЕДЫ, 4</div><div class="note-strong">КАБИНЕТ №3 (ВТОРОЙ ЭТАЖ)</div><div class="note-strong">РО КАЛИНИНГРАДСКИЙ РФ АО РОССЕЛЬХОЗБАНК</div><hr class="separator">
</div>)html";

        return doc.str();
    };

    out << R"html(<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Платёжный документ</title>
<style>
    :root {
        --bg1: #1f2a44;
        --bg2: #243b55;
        --paper: rgba(255,255,255,0.95);
        --text: #1f2a44;
        --accent: #0b5ed7;
        --line: #d7dce3;
    }
    * { box-sizing: border-box; }
    body {
        margin: 0;
        background: linear-gradient(135deg, var(--bg1), var(--bg2));
        font-family: Cambria, serif;
        padding: 12px;
        color: var(--text);
    }
    .paper {
        max-width: 980px;
        margin: 0 auto;
        background: var(--paper);
        border-radius: 14px;
        padding: 16px 20px;
        box-shadow: 0 10px 24px rgba(0,0,0,0.22);
    }
    .topline, .bottom-note {
        white-space: pre-wrap;
        font-size: 15px;
        line-height: 1.15;
        font-family: Cambria, serif;
    }
    .topline {
        text-align: center;
        font-weight: 700;
        margin-bottom: 8px;
    }
    .topline .date-line {
        margin-top: 0;
        font-weight: 700;
    }
    h1 {
        text-align: center;
        margin: 10px 0 12px;
        font-size: 24px;
        color: var(--accent);
        font-family: Cambria, serif;
    }
    .meta {
        width: 72%;
        max-width: 720px;
        margin: 10px 0 10px 0;
        margin-right: auto;
        margin-left: 0;
        border-collapse: collapse;
        font-family: Cambria, serif;
    }
    .calc {
        width: 100%;
        border-collapse: collapse;
        margin: 10px 0;
        font-family: Cambria, serif;
    }
    .meta th, .meta td, .calc th, .calc td {
        border: 1px solid var(--line);
        padding: 6px 8px;
        font-size: 14px;
        vertical-align: middle;
        font-family: Cambria, serif;
        text-align: center;
    }
    .meta th, .calc th {
        background: #f2f6fb;
        text-align: center;
    }
    .right { text-align: right; }
    .center { text-align: center; }
    .actions {
        display: flex;
        gap: 10px;
        flex-wrap: wrap;
        margin-top: 10px;
    }
    a.btn {
        display: inline-flex;
        align-items: center;
        justify-content: center;
        min-height: 40px;
        padding: 0 14px;
        border-radius: 12px;
        text-decoration: none;
        color: #fff;
        background: linear-gradient(135deg, #0b5ed7, #0a58ca);
        font-family: Cambria, serif;
        font-size: 14px;
    }
    .print { background: linear-gradient(135deg, #198754, #146c43); }
    .note {
        font-size: 11px;
        line-height: 1.05;
        font-family: Cambria, serif;
        margin: 0;
    }
    .note-strong {
        text-align: center;
        font-weight: 700;
        margin: 0;
        line-height: 1.05;
        font-size: 11px;
    }
    .separator {
        border: 0;
        border-top: 1px dashed #666;
        margin: 6px 0 0;
        height: 0;
    }
    .doc-info {
        font-size: 10px;
        line-height: 1.05;
        font-family: Cambria, serif;
        margin: 0;
    }
    @media print {
        body { background: #fff; padding: 0; }
        .paper { box-shadow: none; border-radius: 0; max-width: none; padding: 10px 14px; }
        .actions { display: none; }
    }
</style>
</head>
<body>
<div class="paper">)html";

    out << build_one_document(date_out.str());
    out << build_one_document(next_date_out.str());

    out << R"html(
<div class="actions">
    <a class="btn" href="/members">Назад</a>
    <a class="btn print" href="javascript:window.print()">Печать</a>
</div>
</div>
</body>
</html>)html";

    return out.str();
}

int main() {
    load_data();

    httplib::Server server;

    server.Get("/", [](const httplib::Request &, httplib::Response &res) {
        res.set_content(build_index_page(), "text/html; charset=utf-8");
    });

    server.Get("/members", [](const httplib::Request &, httplib::Response &res) {
        res.set_content(build_index_page(), "text/html; charset=utf-8");
    });

    server.Post("/add", [](const httplib::Request &req, httplib::Response &res) {
        Member m;
        m.id = g_next_id++;
        m.fio = req.get_param_value("fio");
        m.area = to_double(req.get_param_value("area"));
        m.address = req.get_param_value("address");
        m.account = req.get_param_value("account");
        m.contribution = to_double(req.get_param_value("contribution"));

        g_members.push_back(m);
        save_data();

        res.set_redirect("/members");
    });

    server.Get("/delete", [](const httplib::Request &req, httplib::Response &res) {
        if (req.has_param("id")) {
            int id = std::stoi(req.get_param_value("id"));
            g_members.erase(std::remove_if(g_members.begin(), g_members.end(),
                                           [id](const Member &m) { return m.id == id; }),
                            g_members.end());
            save_data();
        }
        res.set_redirect("/members");
    });

    server.Get("/document", [](const httplib::Request &req, httplib::Response &res) {
        if (!req.has_param("id")) {
            res.status = 404;
            res.set_content("Участник не найден", "text/plain; charset=utf-8");
            return;
        }

        int id = std::stoi(req.get_param_value("id"));
        Member *m = find_member(id);
        if (!m) {
            res.status = 404;
            res.set_content("Участник не найден", "text/plain; charset=utf-8");
            return;
        }

        res.set_content(build_member_document(*m), "text/html; charset=utf-8");
    });

    std::cout << "Сервер запущен: http://127.0.0.1:8080\n";
    server.listen("0.0.0.0", 8080);
    return 0;
}