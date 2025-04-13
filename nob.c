#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"

#define DEBUG
/*#define Werror*/

#define BUILD_DIR "./build/"
#define SRC_DIR "./src/"
#define INCLUDE_DIR "./include/"
#define LIBRARY_DIR "./library/"
#define RAYLIB_SRC_DIR INCLUDE_DIR "raylib/src/"

void compile_flags(Nob_Cmd *cmd) {
    cmd_append(cmd, "-Wall", "-Wextra");
#ifdef Werror
    cmd_append(cmd, "-Werror");
#endif /* ifdef Werror */
}
void profile_debug_flags(Nob_Cmd *cmd) {
#ifdef DEBUG
    cmd_append(cmd, "-ggdb");
#endif /* ifdef DEBUG */
}

void include_dirs(Nob_Cmd *cmd) {
    cmd_append(cmd, "-I", INCLUDE_DIR "raylib/src");
    cmd_append(cmd, "-I", INCLUDE_DIR "include/clay");
}

bool build_raylib(void) {
    const char *raylib_modules[] = {
        "rcore",   "raudio", "rglfw",     "rmodels",
        "rshapes", "rtext",  "rtextures", "utils",
    };

    bool result = true;
    Nob_Cmd cmd = {0};
    Nob_File_Paths object_files = {0};

    const char *library_path = LIBRARY_DIR "raylib";
    if (!mkdir_if_not_exists(library_path)) {
        return_defer(false);
    }

    Nob_Procs procs = {0};

    for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); i++) {
        const char *input_path =
            temp_sprintf(RAYLIB_SRC_DIR "%s.c", raylib_modules[i]);
        const char *output_path =
            temp_sprintf("%s/%s.o", library_path, raylib_modules[i]);
        output_path = temp_sprintf("%s/%s.o", library_path, raylib_modules[i]);

        da_append(&object_files, output_path);

        if (needs_rebuild(output_path, &input_path, 1)) {
            // What `make` does in RAYLIB_SRC_DIR
            cmd_append(&cmd, "cc", "-ggdb", "-DPLATFORM_DESKTOP", "-D_GLFW_X11",
                       "-fPIC", "-DSUPPORT_FILEFORMAT_FLAC=1", "-I",
                       RAYLIB_SRC_DIR "external/glfw/include", "-c", input_path,
                       "-o", output_path);
            da_append(&procs, nob_cmd_run_async_and_reset(&cmd));
        }
    }

    if (!procs_wait_and_reset(&procs))
        return_defer(false);

    const char *libraylib_path = temp_sprintf("%s/libraylib.a", library_path);

    if (needs_rebuild(libraylib_path, object_files.items, object_files.count)) {
        // Create static library
        cmd_append(&cmd, "ar", "-crs", libraylib_path);
        for (size_t i = 0; i < NOB_ARRAY_LEN(raylib_modules); ++i) {
            const char *input_path =
                temp_sprintf("%s/%s.o", library_path, raylib_modules[i]);
            cmd_append(&cmd, input_path);
        }
        if (!cmd_run_sync_and_reset(&cmd))
            return_defer(false);
    }

// If it fails clean up the code
defer:
    cmd_free(cmd);
    da_free(object_files);
    da_free(procs);
    return result;
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!mkdir_if_not_exists(BUILD_DIR))
        return 1;
    if (!mkdir_if_not_exists(LIBRARY_DIR))
        return 1;

    if (!build_raylib())
        return 1;

    Nob_Cmd cmd = {0};

    cmd_append(&cmd, "gcc");
    compile_flags(&cmd);
    profile_debug_flags(&cmd);

    include_dirs(&cmd);
    cmd_append(&cmd, "-o", BUILD_DIR "main", SRC_DIR "main.c", "-O3");
    cmd_append(&cmd, "-L" LIBRARY_DIR "raylib", "-lraylib");
    cmd_append(&cmd, "-march=native", "-ffast-math");
    cmd_append(&cmd, "-lm", "-lpthread", "-ldl", "-lrt");
    cmd_append(&cmd, "-lGL", "-lX11", "-lXrandr", "-lXinerama", "-lXi",
               "-lXcursor");

    bool cmd_result = cmd_run_sync(cmd);
    cmd_free(cmd);

    if (!cmd_result)
        return 1;

    return 0;
}
