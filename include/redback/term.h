#ifndef REDBACK_TERM_H
#define REDBACK_TERM_H

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

struct event_base;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

///
/// @brief ...
///
struct redback_term;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

///
/// Type definition for a signal callback function triggered whenever a
/// specific signal is captured.
///
/// @param   term:   a pointer to the terminal instance.
/// @param   signal: the signal code that was captured.
///
typedef void (*redback_term_signal_callback)(struct redback_term *term, int signal);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

///
/// @brief   Create a new terminal instance on top of the provided |event_base|.
///
/// @param   evbase: a pointer to an underlying event base.
/// @param   output: a pointer to an output stream (e.g., stdout).
/// @param   input:  a pointer to an input stream (e.g., stdin).
///
/// @returns A pointer to the terminal instance, or NULL on failure.
///
/// @note    The caller retains ownership over provided event base. Its
///          lifetime should be at least as long as that of the terminal instance.
///
struct redback_term *redback_term_new(struct event_base *evbase, void *output, void *input);

///
/// @brief   Deallocate all memory associated with a terminal instance.
///
/// @param   term: a pointer to the terminal instance to be freed.
///
void redback_term_free(struct redback_term *term);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

///
/// @brief Setup the terminal user interface.
///
/// @param term: a pointer to the terminal instance.
///
void redback_term_setup(struct redback_term *term);

///
/// @brief Restore the terminal user interface.
///
/// @param term: a pointer to the terminal instance.
///
void redback_term_restore(struct redback_term *term);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

///
/// @brief Enable terminal input.
///
/// Begins processing user keystrokes.
///
void redback_term_input_enable(const struct redback_term *term);

///
/// @brief Disable terminal input.
///
/// Halts the processing of user keystrokes. To resume,
/// call |redback_term_input_enable|.
///
void redback_term_input_disable(const struct redback_term *term);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

///
/// @brief  Obtain the input stream of the terminal.
///
/// @param  term: the terminal instance.
///
/// @return A pointer to the input stream of the terminal.
///
void *redback_term_input(const struct redback_term *term);

///
/// @brief  Obtain the output stream of the terminal.
///
/// @param  term: the terminal instance.
///
/// @return A pointer to the output stream of the terminal.
///
void *redback_term_output(const struct redback_term *term);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

///
/// @brief Register a signal callback function for a given terminal instance.
///
void redback_term_set_signal_callback(struct redback_term *term, redback_term_signal_callback callback);

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - //

#endif
