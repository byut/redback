#include <log.h>

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <redback/gunit.h>

#include <event2/event.h>

/* # */

/// @brief Structure responsible for keeping track of pending and active I/O events.
static struct event_base *evbase = NULL;
static struct event *evinput = NULL;

/* # */

static struct redback_gunit *gunit = NULL;
static FILE *gunit_out = NULL;
static FILE *gunit_in = NULL;

/* # */

static void on_input(int readfd, short events, void *token) {
    (void)readfd, (void)events, (void)token;
    assert(readfd == fileno(gunit_in));
    char readbuf[1024];
    long lrv;
    lrv = read(readfd, readbuf, sizeof(readbuf));
    assert(-1 != lrv);
    write(fileno(gunit_out), readbuf, lrv);
}

static void on_signal(struct redback_gunit *gunit, int signal) {
    (void)gunit, (void)signal;
    switch (signal) {
    case SIGINT: {
        event_base_loopbreak(evbase);
        break;
    }
    }
}

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
    rv = redback_gunit_setup(gunit, stdout, stdin, NULL);
    if (-1 == rv) {
        log_fatal("Couldn't setup the graphical unit");
        return 1;
    }

    redback_gunit_set_signal_callback(gunit, on_signal);

    gunit_in = redback_gunit_get_input_stream(gunit);
    gunit_out = redback_gunit_get_output_stream(gunit);

    evinput = event_new(
        evbase,
        fileno(gunit_in), EV_READ | EV_PERSIST,
        on_input, NULL);
    if (!evinput) {
        return 1;
    }
    event_add(evinput, 0);

    return event_base_loop(evbase, 0);
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
    if (evinput) {
        event_del(evinput);
        event_free(evinput);
    }
    if (evbase)
        event_base_free(evbase);
}
