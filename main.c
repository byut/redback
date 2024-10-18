#include <log.h>

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <redback/term.h>

#include <event2/event.h>

/* # */

/// @brief Structure responsible for keeping track of pending and active I/O events.
static struct event_base *evbase = NULL;
static struct event *evinput = NULL;

/* # */

static struct redback_term *term = NULL;

/* # */

static void on_input(int readfd, short events, void *token) {
    (void)readfd, (void)events, (void)token;
    assert(readfd == fileno(redback_term_input(term)));
    char readbuf[1024];
    long lrv;
    lrv = read(readfd, readbuf, sizeof(readbuf));
    assert(-1 != lrv);
    write(fileno(redback_term_output(term)), readbuf, lrv);
}

static void on_signal(struct redback_term *gunit, int signal) {
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

    term = redback_term_new(evbase, stdout, stdin);
    if (NULL == term) {
        log_fatal("Couldn't setup the terminal");
        return 1;
    }
    redback_term_setup(term);
    log_set_log_stream(redback_term_output(term));
    redback_term_input_enable(term);
    redback_term_set_signal_callback(term, on_signal);

    evinput = event_new(
        evbase,
        fileno(redback_term_input(term)),
        EV_READ | EV_PERSIST,
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
    log_set_log_stream(stdout);

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
    log_info("Cleaning up ...");

    if (term) {
        redback_term_input_disable(term);
        redback_term_restore(term);
        redback_term_free(term);
    }
    if (evinput) {
        event_del(evinput);
        event_free(evinput);
    }
    if (evbase)
        event_base_free(evbase);
}
