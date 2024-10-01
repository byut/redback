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

#endif
