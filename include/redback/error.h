#ifndef REDBACK_ERROR_H
#define REDBACK_ERROR_H

/* # */

/// @brief Application error codes.
enum redback_error : unsigned short {
    REDBACK_SUCCESS = 256,
    REDBACK_ERROR,
};

/* # */

/// @brief Get the string equivalent of an application error
///        code (|enum redback_error|).
const char *redback_strerror(enum redback_error error);

#endif
