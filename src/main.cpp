#include <pthread.h>
#include <signal.h>

#include <thread>

#include "tsg-billing.hpp"

int main(int argc, char *argv[]) {
    try {
        // SIGINT и SIGTERM принимает отдельный поток через sigwait и останавливает сервер
        sigset_t signals;
        sigemptyset(&signals);
        sigaddset(&signals, SIGINT);
        sigaddset(&signals, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &signals, nullptr);

        TSGBilling billing(argc > 1 ? argv[1] : "/etc/tsg-billing/cfg.json");
        std::jthread signal_thread{[&billing, &signals] {
            int sig{};
            sigwait(&signals, &sig);
            billing.stop();
        }};
        try {
            billing.run();
        } catch (...) {
            pthread_kill(signal_thread.native_handle(), SIGTERM);
            throw;
        }
        // будим поток ожидания, если сервер завершился не по сигналу
        pthread_kill(signal_thread.native_handle(), SIGTERM);
    } catch (std::exception &ex) {
        std::cerr << "Ошибка во время выполнения: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
