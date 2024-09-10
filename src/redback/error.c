#include <redback/error.h>

/* # */

const char *redback_strerror(enum redback_error error) {
    switch (error) {
    case REDBACK_SUCCESS:
        return "Success";
    case REDBACK_ERROR:
        return "Error";
    default:
        return "Unknown error code";
    }
}
