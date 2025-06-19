#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define DEBUG

#define BUILD_DIR "./build/"
#define SRC_DIR "./src/"
#define INCLUDE_DIR "./include/"
#define LIBRARY_DIR "./lib/"

void compile_flags(Nob_Cmd *cmd) { cmd_append(cmd, "-Wall", "-Wextra"); }
void profile_debug_flags(Nob_Cmd *cmd) {
#ifdef DEBUG
    cmd_append(cmd, "-ggdb");
#endif /* ifdef DEBUG */
}

void include_dirs(Nob_Cmd *cmd) {
    cmd_append(cmd, "-I", INCLUDE_DIR "raylib");
    cmd_append(cmd, "-I", INCLUDE_DIR "clay");
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!mkdir_if_not_exists(BUILD_DIR)) return 1;
    if (!mkdir_if_not_exists(LIBRARY_DIR)) return 1;

    Nob_Cmd cmd = {0};

    cmd_append(&cmd, "gcc");
    compile_flags(&cmd);
    profile_debug_flags(&cmd);

    include_dirs(&cmd);
    cmd_append(&cmd, "-o", BUILD_DIR "main", SRC_DIR "main.c", "-O3");
    cmd_append(&cmd, "-L" LIBRARY_DIR "raylib", "-lraylib", "-lm");

    bool cmd_result = cmd_run_sync(cmd);
    cmd_free(cmd);

    if (!cmd_result) return 1;

    return 0;
}
