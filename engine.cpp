#include "engine.h"
#include <thread>
#include <atomic>

extern std::atomic<bool> running;
extern std::thread logic_thread;

int app_main();
void app_begin_auth();

static std::thread app_thread;
static std::atomic<bool> engine_running{false};

namespace engine {

void start_idle() {
    if (engine_running.exchange(true)) return;
    running.store(true);

    app_thread = std::thread([]{
        app_main();
        engine_running.store(false);
    });
}

void authenticate() {
    app_begin_auth();
}

void stop() {
    if (!engine_running.exchange(false)) return;
    running.store(false);

    if (logic_thread.joinable()) logic_thread.join();
    if (app_thread.joinable()) app_thread.join();
}

}
