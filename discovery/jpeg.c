#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>

#define WIDTH 720
#define HEIGHT 400

#define SIZE 720*400*4

#define OUTFILE "jpeg.jpeg"

unsigned char *rgba_to_rgb(unsigned char *rgba, int rgba_size);
void jpeg_write_file(unsigned char *gba, int quality);
void jpeg_write_mem(unsigned char *gba, int quality);

int main()
{
    FILE *f = fopen("./rgba-terminal.dat", "rb");
    if (!f) {
        printf("Couldn't open rgba-terminal.dat\n");
        exit(1);
    }

    unsigned char buf[SIZE];
    int read = fread(buf, sizeof(unsigned char), SIZE, f);
    printf("Read %d bytes.\n", read);
    fclose(f);

    unsigned char *rgb = rgba_to_rgb(buf, SIZE);

    //jpeg_write_file(rgb, 70);
    jpeg_write_mem(rgb, 70);

    free(rgb);

    return 0;
}

unsigned char *
rgba_to_rgb(unsigned char *rgba, int rgba_size)
{
    int rgb_size = rgba_size/4*3;
    unsigned char *rgb = malloc(sizeof(unsigned char)*rgb_size);
    if (!rgb) return NULL;
    int i, j;
    for (i=0,j=0;i<rgba_size;i+=4,j+=3) {
        rgb[j] = *(rgba+i);
        rgb[j+1] = *(rgba+i+1);
        rgb[j+2] = *(rgba+i+2);
    }
    return rgb;
}

void jpeg_write_file(unsigned char *rgb, int quality) {
    printf("Writing to %s.\n", OUTFILE);

    FILE *f = fopen(OUTFILE, "wb");
    if (!f) {
        printf("Couldn't open %s\n", OUTFILE);
        exit(1);
    }

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, f);

    cinfo.image_width = WIDTH;
    cinfo.image_height = HEIGHT;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];

    int row_stride = WIDTH * 3;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &rgb[cinfo.next_scanline*row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    fclose(f);

    jpeg_destroy_compress(&cinfo);
}

void jpeg_write_mem(unsigned char *rgb, int quality) {
    unsigned char *mem_dest;
    unsigned long outsize = 0;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &mem_dest, &outsize);

    cinfo.image_width = WIDTH;
    cinfo.image_height = HEIGHT;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);

    jpeg_set_quality(&cinfo, quality, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer[1];

    int row_stride = WIDTH * 3;

    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &rgb[cinfo.next_scanline*row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    FILE *out = fopen("jpeg-mem.jpeg", "w+");
    if (!out) {
        printf("damn\n");
    }
    fwrite(mem_dest, outsize, 1, out);
    free(mem_dest);
    fclose(out);
}

