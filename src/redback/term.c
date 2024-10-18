#include <redback/term.h>

#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <event2/event.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

struct redback_term {
    FILE *out;
    FILE *in;

    FILE *out_r;
    FILE *out_w;
    FILE *in_r;
    FILE *in_w;

    struct event_base *evbase;
    struct event *evsigint;
    struct event *evoutput;
    struct event *evinput;

    char uin[1024]; /* user input */
    size_t uinlen;  /* user input length (<=1024) */
    size_t uinpos;  /* current position in the user input */

    redback_term_signal_callback on_signal;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

static void redback_term_sigint_callback(int signal, short events, void *token) {
    (void)signal, (void)events, (void)token;
    struct redback_term *term;
    term = (struct redback_term *)token;
    if (term->on_signal) {
        term->on_signal(term, signal);
    }
}

static void redback_term_output_callback(int readfd, short events, void *token) {
    (void)readfd, (void)events, (void)token;
    struct redback_term *term;
    term = (struct redback_term *)token;

    // i'm assuming the following line won't do anything, if input
    // is already disabled.
    redback_term_input_disable(term);

    long lrv;
    bool nl = false;
    char readbuf[1024];
    lrv = read(readfd, readbuf, sizeof(readbuf));
    if (-1 == lrv)
        return;
    if (0 == lrv)
        return;

    for (size_t i = 0, size = lrv; i < size; ++i) {
        nl = false;
        fprintf(term->out, "%c", readbuf[i]);
        if (readbuf[i] == '\n') {
            nl = true;
        }
    }
    fflush(term->out);

    if (nl) {
        redback_term_input_enable(term);
    }
}

static void redback_term_input_callback(int readfd, short events, void *token) {
    (void)readfd, (void)events, (void)token;
    struct redback_term *term;
    term = (struct redback_term *)token;

    long lrv;
    char ch;
    lrv = read(readfd, &ch, sizeof(ch));
    if (1 != lrv)
        return;

    switch (ch) {
    case '\n':
        term->uin[term->uinlen++] = '\n';
        (void)write(fileno(term->in_w), term->uin, term->uinlen);
        term->uinlen = 0;
    default:
        break;
    }

    if (ch >= 32 && ch <= 126 && term->uinlen < sizeof(term->uin) - 1) {
        term->uin[term->uinlen++] = ch;
        fprintf(term->out, "%c", ch);
        fflush(term->out);
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

struct redback_term *redback_term_new(struct event_base *evbase, void *output, void *input) {
    int rv;
    struct redback_term *term;
    term = (struct redback_term *)malloc(sizeof(struct redback_term));
    if (NULL == term)
        return NULL;
    term->evbase = evbase;
    term->out = output;
    term->in = input;

    memset(term->uin, 0x00, sizeof(term->uin));
    term->uinlen = 0;
    term->uinpos = 0;

    // I/O pipes //

    int pipefds[2] = {-1, -1};

    rv = pipe(pipefds);
    if (-1 == rv) {
        redback_term_free(term);
        return NULL;
    }

    term->in_r = fdopen(pipefds[0], "r");
    if (NULL == term->in_r) {
        redback_term_free(term);
        return NULL;
    }

    term->in_w = fdopen(pipefds[1], "w");
    if (NULL == term->in_w) {
        redback_term_free(term);
        return NULL;
    }

    rv = pipe(pipefds);
    if (-1 == rv) {
        redback_term_free(term);
        return NULL;
    }

    rv = fcntl(pipefds[0], F_SETFL, fcntl(pipefds[0], F_GETFL) | O_NONBLOCK);
    if (-1 == rv) {
        redback_term_free(term);
        return NULL;
    }

    term->out_r = fdopen(pipefds[0], "r");
    if (NULL == term->out_r) {
        redback_term_free(term);
        return NULL;
    }

    term->out_w = fdopen(pipefds[1], "w");
    if (NULL == term->out_w) {
        redback_term_free(term);
        return NULL;
    }

    // Events //

    term->evsigint = event_new(
        term->evbase,
        SIGINT, EV_SIGNAL | EV_PERSIST,
        redback_term_sigint_callback, term);
    if (NULL == term->evsigint) {
        redback_term_free(term);
        return NULL;
    }
    event_add(term->evsigint, 0);

    term->evoutput = event_new(
        term->evbase,
        fileno(term->out_r), EV_READ | EV_PERSIST,
        redback_term_output_callback, term);
    if (NULL == term->evoutput) {
        redback_term_free(term);
        return NULL;
    }
    event_add(term->evoutput, 0);

    term->evinput = event_new(
        term->evbase,
        fileno(term->in), EV_READ | EV_PERSIST,
        redback_term_input_callback, term);
    if (NULL == term->evinput) {
        redback_term_free(term);
        return NULL;
    }
    // do not add right away.
    // event_add(term->evsigint, 0);

    return term;
}

void redback_term_free(struct redback_term *term) {
    if (term->out_r) {
        fclose(term->out_r);
    }
    if (term->out_w) {
        fclose(term->out_w);
    }
    if (term->in_r) {
        fclose(term->in_r);
    }
    if (term->in_w) {
        fclose(term->in_w);
    }

    if (term->evsigint) {
        event_del(term->evsigint);
        event_free(term->evsigint);
    }
    if (term->evoutput) {
        event_del(term->evoutput);
        event_free(term->evoutput);
    }
    if (term->evinput) {
        event_del(term->evinput);
        event_free(term->evinput);
    }

    free(term);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void redback_term_setup(struct redback_term *term) {
    (void)term;

    struct termios tattr;

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &tattr);

    // Set terminal to raw mode
    tattr.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

void redback_term_restore(struct redback_term *term) {
    (void)term;

    struct termios tattr;

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &tattr);

    // Restore canonical mode and echo
    tattr.c_lflag |= (ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &tattr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void redback_term_input_enable(const struct redback_term *term) {
    fprintf(term->out, "> %.*s", (int)term->uinlen, term->uin);
    fflush(term->out);
    event_add(term->evinput, 0);
}

void redback_term_input_disable(const struct redback_term *term) {
    if (!event_pending(term->evinput, EV_READ, NULL))
        return;
    event_del(term->evinput);
    fprintf(term->out, "\033[2K\r");
    fflush(term->out);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void *redback_term_input(const struct redback_term *term) {
    return term->in_r;
}

void *redback_term_output(const struct redback_term *term) {
    return term->out_w;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

void redback_term_set_signal_callback(struct redback_term *term, redback_term_signal_callback callback) {
    term->on_signal = callback;
}
