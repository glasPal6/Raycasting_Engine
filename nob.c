#define NOB_IMPLEMENTATION
#include "nob.h"

#define BUILD_FOLDER "build/"
#define SRC_FOLDER "src/"

void compile_flags(Nob_Cmd *cmd) {
    nob_cmd_append(cmd, "-Wall", "-Wextra", "-Werror");
}
void profile_debug_flags(Nob_Cmd *cmd) { nob_cmd_append(cmd, "-g", "-pg"); }
void include_dirs(Nob_Cmd *cmd) {
    nob_cmd_append(cmd, "-I", "include/raylib/src");
    nob_cmd_append(cmd, "-I", "include/clay");
}

void build_raylib() { Nob_Cmd cmd = {0}; }

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_FOLDER))
        return 1;

    Nob_Cmd cmd = {0};

    nob_cmd_append(&cmd, "gcc");
    nob_cmd_append(&cmd, "-o", BUILD_FOLDER "main", SRC_FOLDER "main.c");

    compile_flags(&cmd);
    /*profile_debug_flags(&cmd);*/

    include_dirs(&cmd);

    if (!nob_cmd_run_sync(cmd))
        return 1;
    return 0;
}
