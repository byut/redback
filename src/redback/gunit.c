#include <redback/gunit.h>

#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <ncurses.h>

#include <event2/event.h>

/* # */

struct redback_gunit {
    struct event_base *evbase;

    struct event *evsig;

    WINDOW *scr;
    SCREEN *term;
    FILE *out;
    FILE *in;

    redback_gunit_signal_callback on_signal;
};

/* # */

static void on_redback_gunit_signal(int signal, short events, void *token) {
    (void)signal, (void)events, (void)token;
    struct redback_gunit *gunit;
    gunit = (struct redback_gunit *)token;
    if (gunit->on_signal) {
        gunit->on_signal(gunit, signal);
    }
}

/* # */

struct redback_gunit *redback_gunit_new(struct event_base *evbase) {
    struct redback_gunit *gunit;
    gunit = (struct redback_gunit *)malloc(sizeof(struct redback_gunit));
    if (NULL == gunit)
        return NULL;

    gunit->evbase = evbase;

    gunit->evsig = NULL;

    gunit->term = NULL;
    gunit->scr = NULL;

    gunit->out = NULL;
    gunit->in = NULL;

    return gunit;
}

void redback_gunit_free(struct redback_gunit *gunit) {
    if (gunit->evsig) {
        event_del(gunit->evsig);
        event_free(gunit->evsig);
    }
    free(gunit);
}

/* # */

int redback_gunit_setup(struct redback_gunit *gunit, const char *term) {
    assert(NULL == gunit->term);
    assert(NULL == gunit->scr);

    [[maybe_unused]] int rv;

    gunit->out = stdout;
    gunit->in = stdin;

    gunit->term = newterm(term, gunit->out, gunit->in);
    gunit->scr = stdscr;
    if (NULL == gunit->term)
        return -1;

    rv = cbreak(), assert(ERR != rv);
    rv = noecho(), assert(ERR != rv);
    rv = keypad(gunit->scr, TRUE), assert(ERR != rv);
    rv = nodelay(gunit->scr, TRUE), assert(ERR != rv);

    if (has_colors()) {
        start_color();
        use_default_colors();

        init_pair(0, COLOR_WHITE, COLOR_BLACK);
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_GREEN, COLOR_BLACK);
        init_pair(3, COLOR_BLUE, COLOR_BLACK);
        init_pair(4, COLOR_YELLOW, COLOR_BLACK);
        init_pair(5, COLOR_CYAN, COLOR_BLACK);
        init_pair(6, COLOR_MAGENTA, COLOR_BLACK);

        wbkgd(gunit->scr, COLOR_PAIR(0));
    }

    (void)wrefresh(gunit->scr);

    /* # */

    gunit->evsig = event_new(
        gunit->evbase,
        SIGINT, EV_SIGNAL | EV_PERSIST,
        on_redback_gunit_signal, gunit);
    if (NULL == gunit->evsig) {
        return -1;
    }
    event_add(gunit->evsig, 0);

    /* # */

    return 0;
}

int redback_gunit_restore(struct redback_gunit *gunit) {
    [[maybe_unused]] int rv;

    if (NULL == gunit->scr)
        return 1;
    rv = endwin(), assert(ERR != rv);
    gunit->scr = NULL;

    if (NULL == gunit->term)
        return 1;
    delscreen(gunit->term);
    gunit->term = NULL;

    return 0;
}

/* # */

void redback_gunit_set_signal_callback(struct redback_gunit *gunit, redback_gunit_signal_callback callback) {
    gunit->on_signal = callback;
}
