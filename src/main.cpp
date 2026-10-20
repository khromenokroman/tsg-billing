#include "tsg-billing.hpp"

int main(int argc, char *argv[]) {
    try {
        TSGBilling(argc > 1 ? argv[1] : "/etc/tsg-billing/cfg.json").run();
    } catch (std::exception &ex) {
        std::cerr << "Ошибка во время выполнения: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
