#pragma once
#include <httplib.h>

#include <nlohmann/json.hpp>

/**
 * @brief Модель участника ТСЖ.
 *
 * Содержит данные для отображения в списке, редактирования и формирования
 * платёжных документов.
 */
struct Member {
    int id{};
    std::string fio;
    double area{};
    std::string address;
    std::string account;
    double contribution{};
    double recalculation{};
    double debt{};

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Member, id, fio, area, address, account, contribution, recalculation, debt)
};

/**
 * @brief Конфигурация приложения.
 *
 * Загружается из JSON-файла настроек и содержит параметры запуска,
 * логирования и реквизиты получателя платежей.
 */
struct Config {
    std::string receiver_name{}; ///< Наименование получателя платежей.
    std::string path_db{};       ///< Путь к JSON-базе данных участников.
    int port{};                  ///< Порт HTTP-сервера.
    int log_level{};              ///< Уровень логирования syslog.

    /// Реквизиты получателя платежей. Используются как для отображения в квитанции ("Реквизиты:"),
    /// так и для формирования платёжного QR-кода (ГОСТ Р 56042-2014, формат ST00012).
    /// Необязательны: незаполненные поля просто не попадают в строку реквизитов; если qr_personal_acc
    /// или qr_bic не заполнены, QR-код на квитанции не печатается.
    std::string qr_bank_name{};    ///< Наименование банка получателя.
    std::string qr_bic{};          ///< БИК банка получателя.
    std::string qr_personal_acc{}; ///< Расчётный счёт получателя.
    std::string qr_corresp_acc{};  ///< Корреспондентский счёт банка.
    std::string qr_payee_inn{};    ///< ИНН получателя.
    std::string qr_payee_kpp{};    ///< КПП получателя.

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, path_db, port, log_level, receiver_name)
};
using members = std::vector<Member>;

/**
 * @brief Основной класс веб-приложения TSG billing.
 *
 * Отвечает за:
 * - загрузку и сохранение данных участников;
 * - обработку HTTP-запросов;
 * - генерацию HTML-страниц;
 * - формирование платёжных документов.
 */
class TSGBilling {
   public:
    /**
     * @brief Конструктор приложения.
     *
     * Загружает конфигурацию и инициализирует логирование.
     */
    TSGBilling();

    /**
     * @brief Запускает HTTP-сервер и регистрирует маршруты.
     */
    void run();

   private:
    /**
     * @brief Загружает список участников из JSON-базы.
     */
    void load_users();

    /**
     * @brief Сохраняет список участников в JSON-базу.
     */
    void save_data();

    /**
     * @brief Формирует главную страницу приложения.
     * @return HTML-код главной страницы.
     */
    [[nodiscard]] std::string build_index_page() const;

    /**
     * @brief Формирует страницу добавления нового участника.
     * @return HTML-код страницы добавления участника.
     */
    [[nodiscard]] std::string build_add_member_page() const;

    /**
     * @brief Экранирует HTML-символы в строке.
     * @param s Исходная строка.
     * @return Безопасная для HTML строка.
     */
    [[nodiscard]] std::string html_escape(std::string_view s) const;

    /**
     * @brief Форматирует денежное значение.
     * @param x Число для форматирования.
     * @return Строка с денежным значением.
     */
    [[nodiscard]] std::string format_money(double x) const;

    /**
     * @brief Формирует страницу редактирования участника.
     * @param m Участник.
     * @return HTML-код страницы редактирования.
     */
    [[nodiscard]] std::string build_edit_page(Member const &m) const;

    /**
     * @brief Возвращает CSS-стили для платёжных документов.
     * @return HTML-блок со стилями.
     */
    [[nodiscard]] std::string build_document_style() const;

    /**
     * @brief Формирует HTML-часть одного платёжного документа.
     * @param m Участник.
     * @return HTML-код документа.
     */
    [[nodiscard]] std::string build_member_document_body(Member const &m) const;

    /**
     * @brief Формирует блок кнопок на странице документа.
     * @return HTML-код блока кнопок.
     */
    [[nodiscard]] std::string build_document_buttons() const;

    /**
     * @brief Формирует строку реквизитов получателя платежа для отображения в квитанции.
     * @return Строка вида "ИНН ..., КПП ..., р/с ..., к/с ..., БИК ...". Незаполненные поля пропускаются.
     */
    [[nodiscard]] std::string build_receiver_details() const;

    /**
     * @brief Формирует строку платёжных данных для QR-кода по стандарту ГОСТ Р 56042-2014 (ST00012).
     * @param sum Сумма к оплате, руб.
     * @return Строка для кодирования в QR-код, либо пустая строка, если реквизиты для QR не заданы в конфигурации.
     */
    [[nodiscard]] std::string build_payment_qr_payload(double sum) const;

    /**
     * @brief Формирует SVG-разметку QR-кода по произвольным данным.
     * @param payload Данные для кодирования.
     * @return HTML-код SVG-изображения QR-кода, либо пустая строка, если payload пуст.
     */
    [[nodiscard]] std::string build_qr_svg(std::string const &payload) const;

    /**
     * @brief Формирует страницу платёжного документа для одного участника.
     * @param m Участник.
     * @return HTML-код страницы документа.
     */
    [[nodiscard]] std::string build_member_document(Member const &m) const;

    /**
     * @brief Формирует страницу со всеми платёжными документами.
     * @return HTML-код страницы со всеми документами.
     */
    [[nodiscard]] std::string build_all_members_documents() const;

    /**
     * @brief Преобразует строку в число типа double.
     * @param s Строка с числом.
     * @return Числовое значение.
     */
    [[nodiscard]] double to_double(std::string_view s) const;

    httplib::Server m_server;                                 // 752
    Config m_config;                                          // 104
    members m_members;                                        // 24
    std::string_view m_file_cfg{"/etc/tsg-billing/cfg.json"}; // 16
};
