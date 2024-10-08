#include <redback/gunit.h>

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ncurses.h>

#include <event2/event.h>

/* # */

struct redback_gunit {
    struct event_base *evbase;

    struct event *evin;
    struct event *evsig;
    struct event *evoutpipe;

    SCREEN *term;
    WINDOW *win;
    WINDOW *focus;

    /* # */

    FILE *out_raw;
    FILE *out_r;
    FILE *out_w;
    WINDOW *out_win;
    bool out_nl;

    /* # */

    FILE *in_raw;
    FILE *in_r;
    FILE *in_w;
    WINDOW *in_win;
    char in_buf[1024];
    int in_buflen;

    /* # */

    redback_gunit_signal_callback on_signal;
};

/* # */

static void redback_gunit_out_addch_h(struct redback_gunit *gunit, char ch) {
    waddch(gunit->out_win, ch);
}

static void redback_gunit_out_addch(struct redback_gunit *gunit, char ch) {
    if (gunit->out_nl) {
        gunit->out_nl = false;
        redback_gunit_out_addch_h(gunit, '\n');
    }
    if (ch == '\n') {
        gunit->out_nl = true;
        return;
    }
    redback_gunit_out_addch_h(gunit, ch);
}

/* # */

static void on_output(int readfd, short events, void *token) {
    (void)readfd, (void)events, (void)token;
    struct redback_gunit *gunit;
    gunit = (struct redback_gunit *)token;

    assert(readfd == fileno(gunit->out_r));

    long lrv;
    char readbuf[1024];

    while (true) {
        lrv = read(readfd, readbuf, sizeof(readbuf));
        if (lrv <= 0)
            break;
        assert(lrv <= INT_MAX);
        for (size_t i = 0, size = lrv; i < size; ++i) {
            redback_gunit_out_addch(gunit, readbuf[i]);
        }
    }

    /* # */

    curs_set(0);

    wrefresh(gunit->out_win);
    wrefresh(gunit->in_win);

    curs_set(1);
}

/* # */

static void on_keyup(int readfd, short events, void *token) {
    (void)readfd, (void)events, (void)token;
    struct redback_gunit *gunit;
    gunit = (struct redback_gunit *)token;

    assert(readfd == fileno(gunit->in_raw));
    assert(gunit->in_buflen >= 0);

    int key = wgetch(gunit->in_win);

    // check if |key| is an ASCII character.
    if (key >= 32 && key <= 126 && (unsigned long)gunit->in_buflen < sizeof(gunit->in_buf)) {
        gunit->in_buf[gunit->in_buflen++] = (char)key;
        waddch(gunit->in_win, key);
    }

    switch (key) {
    case '\n': {
        gunit->in_buf[gunit->in_buflen++] = '\n';
        write(fileno(gunit->in_w), gunit->in_buf, gunit->in_buflen);
        wmove(gunit->in_win, 0, 0);
        wclrtoeol(gunit->in_win);
        gunit->in_buflen = 0;
        break;
    }
    default:
        break;
    }

    wrefresh(gunit->in_win);
}

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

    gunit->evin = NULL;
    gunit->evsig = NULL;
    gunit->evoutpipe = NULL;

    gunit->term = NULL;
    gunit->win = NULL;
    gunit->focus = NULL;

    gunit->out_raw = NULL;
    gunit->out_r = NULL;
    gunit->out_w = NULL;
    gunit->out_win = NULL;
    gunit->out_nl = false;

    gunit->in_raw = NULL;
    gunit->in_r = NULL;
    gunit->in_w = NULL;
    gunit->in_win = NULL;
    memset(gunit->in_buf, 0, sizeof(gunit->in_buf));
    gunit->in_buflen = 0;

    gunit->on_signal = NULL;

    return gunit;
}

void redback_gunit_free(struct redback_gunit *gunit) {
    if (gunit->evin) {
        event_del(gunit->evin);
        event_free(gunit->evin);
    }
    if (gunit->evsig) {
        event_del(gunit->evsig);
        event_free(gunit->evsig);
    }
    if (gunit->evoutpipe) {
        event_del(gunit->evoutpipe);
        event_free(gunit->evoutpipe);
    }
    if (gunit->out_r) {
        fclose(gunit->out_r);
    }
    if (gunit->out_w) {
        fclose(gunit->out_w);
    }
    if (gunit->in_r) {
        fclose(gunit->in_r);
    }
    if (gunit->in_w) {
        fclose(gunit->in_w);
    }
    free(gunit);
}

/* # */

int redback_gunit_setup(struct redback_gunit *gunit, void *output, void *input, const char *term) {
    assert(NULL == gunit->term);
    assert(NULL == gunit->win);

    [[maybe_unused]] int rv;

    gunit->out_raw = output;
    gunit->in_raw = input;

    gunit->term = newterm(term, gunit->out_raw, gunit->in_raw);
    gunit->win = stdscr;
    if (NULL == gunit->term)
        return -1;

    curs_set(0);

    rv = cbreak(), assert(ERR != rv);
    rv = noecho(), assert(ERR != rv);
    rv = keypad(gunit->win, TRUE), assert(ERR != rv);
    rv = nodelay(gunit->win, TRUE), assert(ERR != rv);

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

        wbkgd(gunit->win, COLOR_PAIR(0));
    }

    /* # */

    int width = getmaxx(gunit->win);
    int height = getmaxy(gunit->win);

    gunit->out_win = newwin(height - 2, width, 0, 0);
    if (NULL == gunit->out_win) {
        return -1;
    }
    scrollok(gunit->out_win, TRUE);

    gunit->in_win = newwin(1, width, height - 1, 0);
    if (NULL == gunit->in_win) {
        return -1;
    }

    // focus on |gunit->inbox|.
    (void)wrefresh(gunit->win);
    (void)wrefresh(gunit->out_win);
    (void)wrefresh(gunit->in_win);
    curs_set(1);
    gunit->focus = gunit->in_win;

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

    gunit->evin = event_new(
        gunit->evbase,
        fileno(gunit->in_raw), EV_READ | EV_PERSIST,
        on_keyup, gunit);
    if (NULL == gunit->evin) {
        return -1;
    }
    event_add(gunit->evin, 0);

    /* # */

    int pipefds[2];

    rv = pipe(pipefds);
    if (-1 == rv) {
        return -1;
    }

    rv = fcntl(pipefds[0], F_SETFL, fcntl(pipefds[0], F_GETFL) | O_NONBLOCK);
    if (-1 == rv) {
        return -1;
    }

    gunit->out_r = fdopen(pipefds[0], "r");
    if (NULL == gunit->out_r) {
        return -1;
    }
    gunit->out_w = fdopen(pipefds[1], "w");
    if (NULL == gunit->out_w) {
        return -1;
    }

    gunit->evoutpipe = event_new(
        gunit->evbase,
        fileno(gunit->out_r), EV_READ | EV_PERSIST,
        on_output, gunit);
    if (NULL == gunit->evoutpipe) {
        return -1;
    }
    event_add(gunit->evoutpipe, 0);

    /* # */

    rv = pipe(pipefds);
    if (-1 == rv) {
        return -1;
    }

    gunit->in_r = fdopen(pipefds[0], "r");
    if (NULL == gunit->in_r) {
        return -1;
    }
    gunit->in_w = fdopen(pipefds[1], "w");
    if (NULL == gunit->in_w) {
        return -1;
    }

    /* # */

    return 0;
}

int redback_gunit_restore(struct redback_gunit *gunit) {
    [[maybe_unused]] int rv;

    if (gunit->out_win) {
        delwin(gunit->out_win);
    }

    if (gunit->in_win) {
        delwin(gunit->in_win);
    }

    if (NULL == gunit->win)
        return 1;
    rv = endwin(), assert(ERR != rv);
    gunit->win = NULL;

    if (NULL == gunit->term)
        return 1;
    delscreen(gunit->term);
    gunit->term = NULL;

    return 0;
}

/* # */

void *redback_gunit_get_input_stream(const struct redback_gunit *gunit) {
    return gunit->in_r;
}

void *redback_gunit_get_output_stream(const struct redback_gunit *gunit) {
    return gunit->out_w;
}

/* # */

void redback_gunit_set_signal_callback(struct redback_gunit *gunit, redback_gunit_signal_callback callback) {
    gunit->on_signal = callback;
}
