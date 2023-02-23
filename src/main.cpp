#include <zephyr/logging/log.h>
#include "app_task.hpp"

LOG_MODULE_REGISTER(app);

int main() {
	auto& task = AppTask::shared();

    auto err = task.init();

    if (err != 0) {
        LOG_ERR("Failed to initialize. Exiting...");
        return EXIT_FAILURE;
    }

    err = task.start();

    return err == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
