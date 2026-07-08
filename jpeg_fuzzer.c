#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "jpeglib.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

static void jpeg_error_exit(j_common_ptr cinfo)
{
    jmp_buf *jmp = (jmp_buf *)cinfo->client_data;
    longjmp(*jmp, 1);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 2 || size > 1048576) return 0;

    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    jmp_buf jmp;

    cinfo.err = jpeg_std_error(&jerr);
    cinfo.client_data = &jmp;
    jerr.error_exit = jpeg_error_exit;

    if (setjmp(jmp)) {
        jpeg_destroy_decompress(&cinfo);
        return 0;
    }

    jpeg_create_decompress(&cinfo);
    jpeg_mem_src(&cinfo, data, size);
    if (jpeg_read_header(&cinfo, TRUE) == JPEG_HEADER_OK) {
        jpeg_start_decompress(&cinfo);
        JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
            ((j_common_ptr)&cinfo, JPOOL_IMAGE, cinfo.output_width * cinfo.output_components, 1);
        while (cinfo.output_scanline < cinfo.output_height) {
            (void)jpeg_read_scanlines(&cinfo, buffer, 1);
        }
        jpeg_finish_decompress(&cinfo);
    }
    jpeg_destroy_decompress(&cinfo);
    return 0;
}

#ifdef __cplusplus
}
#endif
