#ifndef REDBACK_GUNIT_H
#define REDBACK_GUNIT_H

/* # */

struct event_base;

/* # */

///
/// @brief Structure keeping track of the graphical unit state.
///
struct redback_gunit;

/* # */

///
/// @details Type definition for a signal callback function triggered
///          whenever a specific signal is captured on a given |redback_gunit| instance.
///
/// @param   gunit:  A pointer to the |redback_gunit| that captures the signal.
/// @param   signal: The signal code that was captured.
///
typedef void (*redback_gunit_signal_callback)(struct redback_gunit *gunit, int signal);

/* # */

///
/// @brief   Create a new |redback_gunit| instance on top of the provided |event_base|.
///
/// @param   evbase: Pointer to the underlying event base.
///
/// @returns A pointer to a newly created |redback_gunit| instance, or NULL
///          if the allocation failed.
///
/// @note    The caller retains ownership over the provided |event_base|. Its
///          lifetime should be at least as long as that of the |redback_gunit| instance.
///
struct redback_gunit *redback_gunit_new(struct event_base *evbase);

///
/// @brief   Deallocate all memory associated with a |redback_gunit| instance.
///
/// @param   gunit: the |redback_gunit| instance to be freed.
///
void redback_gunit_free(struct redback_gunit *gunit);

/* # */

///
/// @brief   Setup the terminal user interface.
///
/// @param   gunit: a |redback_gunit| instance.
/// @param   term:  terminal type (e.g., "xterm", "vt100") or |NULL| to use the default.
///
int redback_gunit_setup(struct redback_gunit *gunit, const char *term);

///
/// @brief   Restore the terminal user interface.
///
int redback_gunit_restore(struct redback_gunit *gunit);

/* # */

///
/// @brief Register a signal callback function for a given |redback_gunit| instance.
///
void redback_gunit_set_signal_callback(struct redback_gunit *gunit, redback_gunit_signal_callback callback);

/* # */

#endif
