#include <log.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <event2/event.h>

/* # */

/// @brief Structure responsible for keeping track of pending and active I/O events.
static struct event_base *evbase = NULL;

/* # */

/// @brief Start the application.
static int run() {
    int rv;
    evbase = event_base_new();
    if (NULL == evbase) {
        rv = errno;
        log_fatal("Couldn't initialize I/O : %s", strerror(rv));
        return 1;
    }

    return 0;
}

/* # */

static void cleanup();

int main(int argc, char *argv[]) {
    (void)argc, (void)argv;
    atexit(cleanup);

    log_info("%s v%s [%s %s]",
             REDBACK_PROJECT_NAME,
             REDBACK_PROJECT_VERSION,
             REDBACK_BUILD_TYPE,
             REDBACK_BUILD_TIMESTAMP);

    return run();
}

/* # */

/// @brief Perform the necessary memory cleanup before the application terminates.
static void cleanup() {
    if (evbase)
        event_base_free(evbase);
}
