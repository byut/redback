#include <log.h>

int main(int argc, char *argv[]) {
    (void)argc, (void)argv;
    log_info("%s v%s [%s %s]",
             REDBACK_PROJECT_NAME,
             REDBACK_PROJECT_VERSION,
             REDBACK_BUILD_TYPE,
             REDBACK_BUILD_TIMESTAMP);
    return 0;
}
