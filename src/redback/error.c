#include <asm-generic/errno.h>
#include <redback/error.h>
#include <string.h>

/* # */

const char *redback_strerror(enum redback_error error) {
    if (error <= EHWPOISON) {
        return strerror(error);
    }

    switch ((unsigned short)error) {
    case REDBACK_SUCCESS:
        return "Success";
    case REDBACK_ERROR:
        return "Error";
    default:
        return "Unknown error code";
    }
}
