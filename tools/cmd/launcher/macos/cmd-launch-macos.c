/* macOS CMD launcher template.
 * Build with clang on macOS. The Mach-O entry point locates and validates
 * the embedded CMD container before handing off to the release runtime.
 */
#ifdef __APPLE__
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include "../../format/cmd-format.h"
#include "../../format/cmd-validate.h"

static int load_self(uint8_t **data, size_t *size)
{
    char path[4096];
    uint32_t n = (uint32_t)sizeof(path);
    if (_NSGetExecutablePath(path, &n) != 0) return -1;
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long z = ftell(f);
    if (z < 0) { fclose(f); return -1; }
    rewind(f);
    *data = (uint8_t *)malloc((size_t)z);
    if (!*data) { fclose(f); return -1; }
    *size = (size_t)z;
    if (fread(*data, 1, *size, f) != *size) { free(*data); *data=NULL; fclose(f); return -1; }
    fclose(f);
    return 0;
}

int main(void)
{
    uint8_t *image = NULL;
    size_t size = 0;
    if (load_self(&image, &size) != 0) return 126;
    int valid = 0;
    for (size_t i = 0; i + sizeof(cmd_header_t) <= size; ++i) {
        cmd_header_t h;
        memcpy(&h, image + i, sizeof(h));
        if (cmd_header_valid(&h, size, i) && cmd_payload_flags_valid(h.flags)) {
            valid = 1;
            break;
        }
    }
    free(image);
    if (!valid) { fprintf(stderr, "CMD: valid header not found\n"); return 126; }
    fprintf(stderr, "CMD: macOS launcher template requires the release runtime verifier.\n");
    return 126;
}
#else
int main(void) { return 0; }
#endif
