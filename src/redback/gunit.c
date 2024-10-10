#include <redback/gunit.h>

#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ncurses.h>

#include <event2/event.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static int redback_gunit_refocus(struct redback_gunit *gunit);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

struct redback_text_view {
    struct redback_gunit *gunit;
    struct event *evoutput;
    FILE *rpipe; /* read pipe */
    FILE *wpipe;
    WINDOW *win;
    bool new_line;
};

struct redback_text_box {
    struct redback_gunit *gunit;
    struct event *evinput;
    FILE *in;    /* input stream */
    FILE *rpipe; /* read pipe */
    FILE *wpipe; /* write pipe */
    WINDOW *win;
    char buf[1024];
    unsigned long buflen;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static int redback_text_view_addch(struct redback_text_view *text_view, char ch) {
    int rv;
    if (text_view->new_line) {
        text_view->new_line = false;
        rv = waddch(text_view->win, '\n');
        if (ERR == rv)
            return rv;
    }
    if (ch == '\n') {
        text_view->new_line = true;
        return 0;
    }
    return waddch(text_view->win, ch);
}

static void redback_text_view_rpipe_read_callback(int readfd, short events, void *token) {
    (void)readfd, (void)events, (void)token;
    struct redback_text_view *text_view;
    text_view = (struct redback_text_view *)token;
    assert(readfd == fileno(text_view->rpipe));

    [[maybe_unused]] int rv;
    long lrv;
    char readbuf[getmaxx(text_view->win) * getmaxy(text_view->win)];

    while (true) {
        lrv = read(fileno(text_view->rpipe), readbuf, sizeof(readbuf));
        if (lrv <= 0)
            break;
        for (size_t i = 0, size = lrv; i < size; ++i) {
            rv = redback_text_view_addch(text_view, readbuf[i]), assert(ERR != rv);
        }
    }

    rv = wnoutrefresh(text_view->win), assert(ERR != rv);
    rv = redback_gunit_refocus(text_view->gunit), assert(ERR != rv);
    rv = curs_set(0), assert(ERR != rv);
    rv = doupdate(), assert(ERR != rv);
    rv = curs_set(1), assert(ERR != rv);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void redback_text_box_in_read_callback(int readfd, short events, void *token) {
    (void)readfd, (void)events, (void)token;
    struct redback_text_box *text_box;
    text_box = (struct redback_text_box *)token;
    assert(readfd == fileno(text_box->in));

    [[maybe_unused]] int rv;
    [[maybe_unused]] long lrv;
    int key;

    while (true) {
        key = ERR;
        key = wgetch(text_box->win);
        if (ERR == key)
            break;

        if (key >= 32 && key <= 126 && text_box->buflen < sizeof(text_box->buf) - 1) {
            text_box->buf[text_box->buflen++] = (char)key;
            rv = waddch(text_box->win, key), assert(ERR != rv);
            rv = wnoutrefresh(text_box->win), assert(ERR != rv);
            rv = doupdate(), assert(ERR != rv);
            continue;
        }

        switch (key) {
        case '\n':
            rv = werase(text_box->win), assert(ERR != rv);
            rv = wnoutrefresh(text_box->win), assert(ERR != rv);
            rv = doupdate(), assert(ERR != rv);
            text_box->buf[text_box->buflen++] = '\n';
            lrv = write(fileno(text_box->wpipe), text_box->buf, text_box->buflen);
            assert(lrv > 0);
            text_box->buflen = 0;
            break;
        default:
            break;
        }
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

struct redback_gunit {
    struct event_base *evbase;
    struct event *evsigint;

    SCREEN *term;
    WINDOW *win;
    FILE *out;
    FILE *in;

    struct redback_text_view text_view;
    struct redback_text_box text_box;

    redback_gunit_signal_callback on_signal;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static int redback_gunit_refocus(struct redback_gunit *gunit) {
    return wnoutrefresh(gunit->text_box.win);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void redback_gunit_sigint_callback(int signal, short events, void *token) {
    (void)signal, (void)events, (void)token;
    struct redback_gunit *gunit;
    gunit = (struct redback_gunit *)token;
    if (gunit->on_signal) {
        gunit->on_signal(gunit, signal);
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

struct redback_gunit *redback_gunit_new(struct event_base *evbase) {
    struct redback_gunit *gunit;
    gunit = (struct redback_gunit *)malloc(sizeof(struct redback_gunit));
    if (NULL == gunit)
        return NULL;
    memset(gunit, 0x00, sizeof(struct redback_gunit));
    gunit->evbase = evbase;
    return gunit;
}

int redback_gunit_setup(struct redback_gunit *gunit, void *output, void *input, const char *term) {
    [[maybe_unused]] int rv;
    int pipefds[2] = {-1, -1};

    // Terminal Setup ///

    gunit->out = output;
    gunit->in = input;

    gunit->term = newterm(term, gunit->out, gunit->in);
    gunit->win = stdscr;
    if (NULL == gunit->term) {
        return -1;
    }

    rv = curs_set(0), assert(ERR != rv);
    rv = cbreak(), assert(ERR != rv);
    rv = noecho(), assert(ERR != rv);

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

    // Signal Handlers //

    gunit->evsigint = event_new(
        gunit->evbase,
        SIGINT, EV_SIGNAL | EV_PERSIST,
        redback_gunit_sigint_callback, gunit);
    if (NULL == gunit->evsigint) {
        return -1;
    }
    event_add(gunit->evsigint, 0);

    // Text Box //

    gunit->text_box.gunit = gunit;
    gunit->text_box.in = gunit->in;
    gunit->text_box.buflen = 0;
    memset(gunit->text_box.buf, 0x00, sizeof(gunit->text_box.buf));

    gunit->text_box.win = newwin(
        1, getmaxx(gunit->win),
        getmaxy(gunit->win) - 1, 0);
    if (NULL == gunit->text_box.win) {
        return -1;
    }
    rv = notimeout(gunit->text_box.win, TRUE), assert(ERR != rv);
    rv = nodelay(gunit->text_box.win, TRUE), assert(ERR != rv);
    rv = keypad(gunit->text_box.win, TRUE), assert(ERR != rv);
    rv = wbkgd(gunit->text_box.win, COLOR_PAIR(0)), assert(ERR != rv);

    rv = pipe(pipefds);
    if (-1 == rv) {
        return -1;
    }

    gunit->text_box.rpipe = fdopen(pipefds[0], "r");
    if (NULL == gunit->text_box.rpipe) {
        return -1;
    }
    gunit->text_box.wpipe = fdopen(pipefds[1], "w");
    if (NULL == gunit->text_box.wpipe) {
        return -1;
    }

    gunit->text_box.evinput = event_new(
        gunit->evbase,
        fileno(gunit->text_box.in),
        EV_READ | EV_PERSIST,
        redback_text_box_in_read_callback,
        &gunit->text_box);
    if (NULL == gunit->text_box.evinput) {
        return -1;
    }
    event_add(gunit->text_box.evinput, 0);

    // Text View //

    gunit->text_view.gunit = gunit;
    gunit->text_view.new_line = false;

    gunit->text_view.win = newwin(getmaxy(gunit->win) - 2, getmaxx(gunit->win), 0, 0);
    if (NULL == gunit->text_view.win) {
        return -1;
    }
    rv = scrollok(gunit->text_view.win, TRUE), assert(ERR != rv);
    rv = wbkgd(gunit->text_box.win, COLOR_PAIR(0)), assert(ERR != rv);

    rv = pipe(pipefds);
    if (-1 == rv) {
        return -1;
    }

    rv = fcntl(pipefds[0], F_SETFL, fcntl(pipefds[0], F_GETFL) | O_NONBLOCK);
    if (-1 == rv) {
        return -1;
    }

    gunit->text_view.rpipe = fdopen(pipefds[0], "r");
    if (NULL == gunit->text_view.rpipe) {
        return -1;
    }
    gunit->text_view.wpipe = fdopen(pipefds[1], "w");
    if (NULL == gunit->text_view.wpipe) {
        return -1;
    }

    gunit->text_view.evoutput = event_new(
        gunit->evbase,
        fileno(gunit->text_view.rpipe), EV_READ | EV_PERSIST,
        redback_text_view_rpipe_read_callback, &gunit->text_view);
    if (NULL == gunit->text_view.evoutput) {
        return -1;
    }
    event_add(gunit->text_view.evoutput, 0);

    // ... //

    rv = wnoutrefresh(gunit->win), assert(ERR != rv);
    rv = wnoutrefresh(gunit->text_view.win), assert(ERR != rv);
    rv = wnoutrefresh(gunit->text_box.win), assert(ERR != rv);

    rv = curs_set(1), assert(ERR != rv);
    rv = doupdate(), assert(ERR != rv);

    return 0;
}

int redback_gunit_restore(struct redback_gunit *gunit) {
    [[maybe_unused]] int rv;

    // Text View //

    if (gunit->text_view.evoutput) {
        event_del(gunit->text_view.evoutput);
        event_free(gunit->text_view.evoutput);
    }
    if (gunit->text_view.rpipe) {
        fclose(gunit->text_view.rpipe);
    }
    if (gunit->text_view.wpipe) {
        fclose(gunit->text_view.wpipe);
    }
    if (gunit->text_view.win) {
        delwin(gunit->text_view.win);
    }

    // Text Box //

    if (gunit->text_box.evinput) {
        event_del(gunit->text_box.evinput);
        event_free(gunit->text_box.evinput);
    }
    if (gunit->text_box.rpipe) {
        fclose(gunit->text_box.rpipe);
    }
    if (gunit->text_box.wpipe) {
        fclose(gunit->text_box.wpipe);
    }
    if (gunit->text_box.win) {
        delwin(gunit->text_box.win);
    }

    // Signal Handlers //

    if (gunit->evsigint) {
        event_del(gunit->evsigint);
        event_free(gunit->evsigint);
    }

    // Terminal //

    if (NULL == gunit->win)
        return -1;
    rv = endwin(), assert(ERR != rv);
    gunit->win = NULL;

    if (NULL == gunit->term)
        return 1;
    delscreen(gunit->term);
    gunit->term = NULL;

    return 0;
}

void redback_gunit_free(struct redback_gunit *gunit) {
    free(gunit);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void *redback_gunit_get_input_stream(const struct redback_gunit *gunit) {
    return gunit->text_box.rpipe;
}

void *redback_gunit_get_output_stream(const struct redback_gunit *gunit) {
    return gunit->text_view.wpipe;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void redback_gunit_set_signal_callback(struct redback_gunit *gunit, redback_gunit_signal_callback callback) {
    gunit->on_signal = callback;
}
