#include <log.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <redback/gunit.h>

#include <event2/event.h>

/* # */

/// @brief Structure responsible for keeping track of pending and active I/O events.
static struct event_base *evbase = NULL;

/* # */

static struct redback_gunit *gunit = NULL;

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

    gunit = redback_gunit_new(evbase);
    if (NULL == gunit) {
        log_fatal("Couldn't setup the graphical unit");
        return 1;
    }
    rv = redback_gunit_setup(gunit, NULL);
    if (-1 == rv) {
        log_fatal("Couldn't setup the graphical unit");
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
    if (gunit) {
        redback_gunit_restore(gunit);
        redback_gunit_free(gunit);
    }
    if (evbase)
        event_base_free(evbase);
}
