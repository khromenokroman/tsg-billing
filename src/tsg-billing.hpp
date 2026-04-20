#pragma once
#include <httplib.h>
#include <nlohmann/json.hpp>

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

struct Config {
    std::string path_db{}; // 32
    int port{}; // 4
    int log_level{}; // 4

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, path_db, port, log_level)
};
using members = std::vector<Member>;

class TSGBilling {
public:
    TSGBilling();
    void run();

private:
    void load_users();
    void save_data();
    [[nodiscard]] std::string build_index_page() const;
    [[nodiscard]] std::string html_escape(std::string_view s) const;
    [[nodiscard]] std::string format_money(double x) const;
    [[nodiscard]] std::string build_edit_page(Member const &m);
    [[nodiscard]] double to_double(std::string_view s);


    httplib::Server m_server; // 752
    Config m_config; // 40
    members m_members; // 24
    std::string_view m_file_cfg{"/etc/tsg-billing/cfg.json"}; // 16
};
