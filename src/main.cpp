#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "tsg-billing.hpp"


// static int g_next_id = 1;
//
//
//

//
//
//

//
//
//
// static Member* find_member(int id) {
//     for (auto &m : g_members) {
//         if (m.id == id) return &m;
//     }
//     return nullptr;
// }
//
// static std::string build_edit_page(const Member &m) {
//     std::ostringstream out;
//     out << R"html(<!doctype html>
// <html lang="ru">
// <head>
// <meta charset="utf-8">
// <meta name="viewport" content="width=device-width, initial-scale=1.0">
// <title>Редактирование участника</title>
// <style>
//     :root {
//         --bg1: #1f2a44;
//         --bg2: #243b55;
//         --text: #f5f5f5;
//         --accent2: #06d6a0;
//         --shadow: 0 12px 30px rgba(0,0,0,0.28);
//     }
//     * { box-sizing: border-box; }
//     body {
//         margin: 0;
//         min-height: 100vh;
//         font-family: Cambria, serif;
//         color: var(--text);
//         background:
//             radial-gradient(circle at top left, rgba(255,209,102,0.20), transparent 28%),
//             radial-gradient(circle at bottom right, rgba(6,214,160,0.18), transparent 30%),
//             linear-gradient(135deg, var(--bg1), var(--bg2));
//         padding: 24px;
//     }
//     .wrapper {
//         width: 100%;
//         max-width: 980px;
//         margin: 0 auto;
//         background: rgba(255,255,255,0.06);
//         backdrop-filter: blur(10px);
//         border: 1px solid rgba(255,255,255,0.12);
//         border-radius: 28px;
//         box-shadow: var(--shadow);
//         padding: 34px;
//     }
//     h1 {
//         margin: 0 0 20px;
//         font-size: 36px;
//         text-align: center;
//     }
//     .form-grid {
//         display: grid;
//         grid-template-columns: 1fr;
//         gap: 14px;
//         margin-bottom: 18px;
//     }
//     .pair-grid {
//         display: grid;
//         grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
//         gap: 14px;
//     }
//     .field { display: grid; gap: 8px; }
//     label { font-size: 16px; font-weight: 700; }
//     input {
//         width: 100%;
//         padding: 13px 14px;
//         border-radius: 14px;
//         border: 1px solid rgba(255,255,255,0.14);
//         background: rgba(255,255,255,0.10);
//         color: #fff;
//         outline: none;
//         font-family: Cambria, serif;
//         font-size: 16px;
//     }
//     .btn {
//         display: inline-flex;
//         align-items: center;
//         justify-content: center;
//         min-height: 54px;
//         padding: 0 28px;
//         border-radius: 18px;
//         border: none;
//         cursor: pointer;
//         text-decoration: none;
//         font-family: Cambria, serif;
//         font-size: 18px;
//         font-weight: 700;
//         background: linear-gradient(135deg, var(--accent2), #118ab2);
//         color: #fff;
//     }
//     .back {
//         display: inline-flex;
//         align-items: center;
//         justify-content: center;
//         min-height: 54px;
//         padding: 0 28px;
//         border-radius: 18px;
//         text-decoration: none;
//         font-family: Cambria, serif;
//         font-size: 18px;
//         font-weight: 700;
//         color: #fff;
//         background: rgba(255,255,255,0.10);
//         border: 1px solid rgba(255,255,255,0.14);
//     }
//     .actions { display: flex; gap: 12px; flex-wrap: wrap; margin-top: 8px; }
// </style>
// </head>
// <body>
// <div class="wrapper">
//     <h1>Редактирование участника</h1>
//     <form action="/update" method="post">
//         <input type="hidden" name="id" value=")html";
//     out << m.id;
//     out << R"html(">
//         <div class="form-grid">
//             <div class="field">
//                 <label>ФИО</label>
//                 <input name="fio" required value=")html";
//     out << html_escape(m.fio);
//     out << R"html(">
//             </div>
//             <div class="field">
//                 <label>Адрес</label>
//                 <input name="address" required value=")html";
//     out << html_escape(m.address);
//     out << R"html(">
//             </div>
//         </div>
//
//         <div class="pair-grid">
//             <div class="field">
//                 <label>Площадь квартиры</label>
//                 <input name="area" required value=")html";
//     out << m.area;
//     out << R"html(">
//             </div>
//             <div class="field">
//                 <label>Лицевой счёт</label>
//                 <input name="account" required value=")html";
//     out << html_escape(m.account);
//     out << R"html(">
//             </div>
//             <div class="field">
//                 <label>Размер взноса</label>
//                 <input name="contribution" required value=")html";
//     out << m.contribution;
//     out << R"html(">
//             </div>
//             <div class="field">
//                 <label>Перерасчёт</label>
//                 <input name="recalculation" required value=")html";
//     out << m.recalculation;
//     out << R"html(">
//             </div>
//             <div class="field">
//                 <label>Задолженность</label>
//                 <input name="debt" required value=")html";
//     out << m.debt;
//     out << R"html(">
//             </div>
//         </div>
//
//         <div class="actions">
//             <button class="btn" type="submit">Сохранить</button>
//             <a class="back" href="/members">Отмена</a>
//         </div>
//     </form>
// </div>
// </body>
// </html>)html";
//     return out.str();
// }
//
//
//
// static std::string build_member_document(const Member &m) {
//     std::ostringstream out;
//
//     auto now = std::chrono::system_clock::now();
//     std::time_t tt = std::chrono::system_clock::to_time_t(now);
//     std::tm tm{};
//     tm = *std::localtime(&tt);
//
//     int month = tm.tm_mon + 1;
//     int year = tm.tm_year + 1900;
//     int next_month = month + 1;
//     int next_year = year;
//     if (next_month == 13) {
//         next_month = 1;
//         ++next_year;
//     }
//
//     std::ostringstream date_out;
//     date_out << std::setfill('0') << std::setw(2) << month << "." << year;
//
//     std::ostringstream next_date_out;
//     next_date_out << std::setfill('0') << std::setw(2) << next_month << "." << next_year;
//
//     auto build_one_document = [&](const std::string &period) {
//         std::ostringstream doc;
//
//         doc << R"html(<div class="paper">
// <div class="topline"><div>Адрес: )html";
//         doc << html_escape(m.address);
//         doc << R"html(</div><div class="date-line">Платежный документ за )html";
//         doc << period;
//         doc << R"html(</div></div>
//
// <table class="meta">
// <tr>
//     <th>№ лицевого счета</th>
//     <th>Период</th>
//     <th>К оплате</th>
// </tr>
// <tr>
//     <td>)html";
//         doc << html_escape(m.account);
//         doc << R"html(</td>
//     <td>)html";
//         doc << period;
//         doc << R"html(</td>
//     <td>)html";
//         doc << format_money(m.area * m.contribution);
//         doc << R"html(</td>
// </tr>
// </table>
//
// <div class="doc-info"><b>Плательщик:</b> )html";
//         doc << html_escape(m.fio);
//         doc << R"html(</div>
// <div class="doc-info"><b>Адрес объекта собственности:</b> )html";
//         doc << html_escape(m.address);
//         doc << R"html(</div>
// <div class="doc-info"><b>Получатель платежа:</b> ТСЖ "Школьная, 17"</div>
// <div class="doc-info"><b>Реквизиты:</b> ИНН 3902800956, КПП 390201001, р\с 40705810755050000170, к\с
// 30101810500000000878, БИК 042748878</div> <div class="doc-info"><b>Назначение платежа:</b> Взнос на капитальный
// ремонт общего имущества в многоквартирном доме</div>
//
// <table class="calc">
// <tr>
//     <th>Название услуги</th>
//     <th>Ед. изм.</th>
//     <th>Площадь</th>
//     <th>Размер взноса</th>
//     <th>Сумма</th>
//     <th>Перерасчет</th>
//     <th>Итого</th>
// </tr>
// <tr>
//     <td>Взнос на капитальный ремонт общего имущества в многоквартирном доме</td>
//     <td class="center">кв.м</td>
//     <td class="right">)html";
//         doc << std::fixed << std::setprecision(1) << m.area;
//         doc << R"html(</td>
//     <td class="right">)html";
//         doc << std::fixed << std::setprecision(1) << m.contribution;
//         doc << R"html(</td>
//     <td class="right">)html";
//         doc << format_money(m.area * m.contribution);
//         doc << R"html(</td>
//     <td class="right">)html";
//         doc << format_money(m.recalculation);
//         doc << R"html(</td>
//     <td class="right">)html";
//         doc << format_money(m.area * m.contribution + m.recalculation - m.debt);
//         doc << R"html(</td>
// </tr>
// </table>
//
// <div>Задолженность: )html";
//         doc << format_money(m.debt);
//         doc << R"html(</div>
// <div><b>Итого к оплате:</b> )html";
//         doc << format_money(m.area * m.contribution + m.recalculation - m.debt);
//         doc << R"html(</div>
//
// <div class="note-strong">УВАЖАЕМЫЕ СОБСТВЕННИКИ ПОМЕЩЕНИЙ!</div><p class="note">В соответствии с Законом
// Калининградской области от 26.12.2013 № 293, от 19.12.2016 № 42, минимальный размер взноса на капитальный ремонт на
// 2015,2016,2017 года установлен в размере 5,9 руб. за один квадратный метр общей площади помещения в многоквартирном
// доме. В соответствии с ч.14.1. ст.155 ЖК РФ собственники помещений в многоквартирном доме, несвоевременно и (или) не
// полностью уплатившие взносы на капитальный ремонт, обязаны уплатить пени в размере ставки рефинансирования ЦБ РФ.
// Если оплата не производится, то задолженность взыскивается в судебном порядке, при этом к сумме задолженности и пени
// прибавляется сумма понесенных судебных расходов. Телефон для справок: 89216190701 Прием звонков: понедельник –
// пятница с 08.00-17.00</p><div class="note-strong">ОПЛАТА ПРОИЗВОДИТСЯ ПО АДРЕСУ: Г. ГУСЕВ, УЛ. ПОБЕДЫ, 4</div><div
// class="note-strong">КАБИНЕТ №3 (ВТОРОЙ ЭТАЖ)</div><div class="note-strong">РО КАЛИНИНГРАДСКИЙ РФ АО
// РОССЕЛЬХОЗБАНК</div><hr class="separator">
// </div>)html";
//
//         return doc.str();
//     };
//
//     out << R"html(<!doctype html>
// <html lang="ru">
// <head>
// <meta charset="utf-8">
// <meta name="viewport" content="width=device-width, initial-scale=1.0">
// <title>Платёжный документ</title>
// <style>
//     :root {
//         --bg1: #1f2a44;
//         --bg2: #243b55;
//         --paper: rgba(255,255,255,0.95);
//         --text: #1f2a44;
//         --accent: #0b5ed7;
//         --line: #d7dce3;
//     }
//     * { box-sizing: border-box; }
//     body {
//         margin: 0;
//         background: linear-gradient(135deg, var(--bg1), var(--bg2));
//         font-family: Cambria, serif;
//         padding: 12px;
//         color: var(--text);
//     }
//     .paper {
//         max-width: 980px;
//         margin: 0 auto;
//         background: var(--paper);
//         border-radius: 14px;
//         padding: 16px 20px;
//         box-shadow: 0 10px 24px rgba(0,0,0,0.22);
//         margin-bottom: 20px;
//     }
//     .topline, .bottom-note {
//         white-space: pre-wrap;
//         font-size: 15px;
//         line-height: 1.15;
//         font-family: Cambria, serif;
//     }
//     .topline {
//         text-align: center;
//         font-weight: 700;
//         margin-bottom: 8px;
//     }
//     .topline .date-line {
//         margin-top: 0;
//         font-weight: 700;
//     }
//     .meta {
//         width: 72%;
//         max-width: 720px;
//         margin: 10px 0 10px 0;
//         margin-right: auto;
//         margin-left: 0;
//         border-collapse: collapse;
//         font-family: Cambria, serif;
//     }
//     .calc {
//         width: 100%;
//         border-collapse: collapse;
//         margin: 10px 0;
//         font-family: Cambria, serif;
//     }
//     .meta th, .meta td, .calc th, .calc td {
//         border: 1px solid var(--line);
//         padding: 6px 8px;
//         font-size: 14px;
//         vertical-align: middle;
//         font-family: Cambria, serif;
//         text-align: center;
//     }
//     .meta th, .calc th {
//         background: #f2f6fb;
//         text-align: center;
//     }
//     .doc-info {
//         font-size: 10px;
//         line-height: 1.05;
//         font-family: Cambria, serif;
//         margin: 0;
//     }
//     .note {
//         font-size: 11px;
//         line-height: 1.05;
//         font-family: Cambria, serif;
//         margin: 0;
//     }
//     .note-strong {
//         text-align: center;
//         font-weight: 700;
//         margin: 0;
//         line-height: 1.05;
//         font-size: 11px;
//     }
//     .separator {
//         border: 0;
//         border-top: 1px dashed #666;
//         margin: 6px 0 0;
//         height: 0;
//     }
//     .actions {
//         display: flex;
//         gap: 10px;
//         flex-wrap: wrap;
//         margin-top: 10px;
//     }
//     a.btn {
//         display: inline-flex;
//         align-items: center;
//         justify-content: center;
//         min-height: 40px;
//         padding: 0 14px;
//         border-radius: 12px;
//         text-decoration: none;
//         color: #fff;
//         background: linear-gradient(135deg, #0b5ed7, #0a58ca);
//         font-family: Cambria, serif;
//         font-size: 14px;
//     }
//     .print { background: linear-gradient(135deg, #198754, #146c43); }
//     @media print {
//         body { background: #fff; padding: 0; }
//         .paper { box-shadow: none; border-radius: 0; max-width: none; padding: 10px 14px; margin-bottom: 0; }
//         .actions { display: none; }
//     }
// </style>
// </head>
// <body>
// )html";
//
//     out << build_one_document(date_out.str());
//     out << build_one_document(next_date_out.str());
//
//     out << R"html(
// <div class="paper">
// <div class="actions">
//     <a class="btn" href="/members">Назад</a>
//     <a class="btn print" href="javascript:window.print()">Печать</a>
// </div>
// </div>
// </body>
// </html>)html";
//     return out.str();
// }

int main() {
    TSGBilling().run();
    return 0;
}
