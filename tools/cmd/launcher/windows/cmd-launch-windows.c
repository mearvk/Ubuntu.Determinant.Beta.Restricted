/* Windows CMD launcher template.
 * Build with MSVC or MinGW on Windows. The executable locates the CMD header
 * in its own image, verifies the embedded payload, then starts Java.
 */
#ifdef _WIN32
#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../format/cmd-format.h"
#include "../../format/cmd-validate.h"

static int self_image(uint8_t **data, size_t *size)
{
    char path[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, path, sizeof(path));
    if (!n || n >= sizeof(path)) return -1;
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
    if (self_image(&image, &size) != 0) return 126;
    size_t found = 0;
    for (size_t i = 0; i + sizeof(cmd_header_t) <= size; ++i) {
        cmd_header_t h;
        memcpy(&h, image + i, sizeof(h));
        if (cmd_header_valid(&h, size, i) && cmd_payload_flags_valid(h.flags)) {
            found = i;
            break;
        }
    }
    if (!found) { free(image); fprintf(stderr, "CMD: valid header not found\n"); return 126; }
    /* Payload extraction and SHA-256 verification are shared with the release
       launcher implementation; this source is the native Windows entry point. */
    free(image);
    fprintf(stderr, "CMD: Windows launcher template requires the release runtime verifier.\n");
    return 126;
}
#else
int main(void) { return 0; }
#endif
