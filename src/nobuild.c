/*
* Not a build script, the actual implementation of nobuild
*/
#include "nobuild.h"
#include "nobuild_log.h"
#include "nobuild_cstr.h"
#include "nobuild_io.h"
#include "nobuild_cmd.h"
#include "nobuild_path.h"

#include <errno.h>

char *shift_args(int *argc, char ***argv)
{
    assert(*argc > 0);
    char *result = **argv;
    *argc -= 1;
    *argv += 1;
    return result;
}

void file_to_c_array(Cstr path, Cstr out_path, Cstr array_type, Cstr array_name, int null_term) {
    Fd file = fd_open_for_read(path);
    Fd output_file = fd_open_for_write(out_path);
    fd_printf(output_file, "%s %s[] = {\n", array_type, array_name);

    unsigned char buffer[4096];
    unsigned long total_bytes_read = 0;
    do {
#ifndef _WIN32
        ssize_t bytes = read(file, buffer, sizeof(buffer));
        if (bytes == -1) {
            ERRO("Could not read file %s: %s", path, strerror(errno));
            fd_close(file);
            fd_close(output_file);
            break;
        }

        if (bytes == 0) {
            break;
        }
#else
        DWORD bytes;
        if (!ReadFile(file, buffer, sizeof buffer, &bytes, NULL)) {
            ERRO("Could not read file %s: %s", path, nobuild__GetLastErrorAsString());
            break;
        }
#endif
        int bytes_read = (int) bytes;

        if (bytes_read == 0) {
            break;
        }

        for (int i = 0; i < bytes_read; i+=16) {
            fd_printf(output_file, "\t");
            for (int j = i; j < i+16; j++) {
                if (j >= bytes_read) {
                    break;
                }
                fd_printf(output_file, "0x%02x, ", buffer[j]);
            }
            fd_printf(output_file, "\n");
        }
        total_bytes_read += (unsigned long) bytes_read;
    } while (1);

    if (null_term) {
        fd_printf(output_file, "\t0x00 /* Terminate with null */\n");
        total_bytes_read++;
    }
    fd_printf(output_file, "};\n");
    fd_printf(output_file, "unsigned long %s_len = %lu;\n", array_name, total_bytes_read);

    fd_close(file);
    fd_close(output_file);
}
