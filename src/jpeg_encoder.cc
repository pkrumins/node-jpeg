#include "jpeg_encoder.h"

JpegEncoder::JpegEncoder(unsigned char *ddata, int wwidth, int hheight,
    int qquality, buffer_type bbuf_type)
    :
    data(ddata), width(wwidth), height(hheight), quality(qquality),
    buf_type(bbuf_type),
    jpeg(NULL), jpeg_len(0),
    offset(0, 0, 0, 0) {}

JpegEncoder::~JpegEncoder() {
    free(jpeg);
}

void
JpegEncoder::encode()
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, &jpeg, &jpeg_len);

    if (offset.isNull()) {
        cinfo.image_width = width;
        cinfo.image_height = height;
    }
    else {
        cinfo.image_width = offset.w;
        cinfo.image_height = offset.h;
    }
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    unsigned char *rgb_data;
    if (buf_type == BUF_RGBA) {
        rgb_data = rgba_to_rgb(data, width*height*4);
        if (!rgb_data) throw "malloc failed in JpegEncoder::encode/rgba_to_rgb.";
    }
    else if (buf_type == BUF_BGRA) {
        rgb_data = bgra_to_rgb(data, width*height*4);
        if (!rgb_data) throw "malloc failed in JpegEncoder::encode/bgra_to_rgb.";

    }
    else if (buf_type == BUF_BGR) {
        rgb_data = bgr_to_rgb(data, width*height*3);
        if (!rgb_data) throw "malloc failed in JpegEncoder::encode/bgr_to_rgb.";
    }
    else if (buf_type == BUF_RGB) {
        rgb_data = data;
    }
    else {
        throw "Unknown buf_type";
    }

    JSAMPROW row_pointer;

    if (offset.isNull()) {
        while (cinfo.next_scanline < cinfo.image_height) {
            row_pointer = &rgb_data[cinfo.next_scanline*3*width];
            jpeg_write_scanlines(&cinfo, &row_pointer, 1);
        }
    }
    else {
        int start = offset.y*width*3 + offset.x*3;
        while (cinfo.next_scanline < cinfo.image_height) {
            row_pointer = &rgb_data[start + cinfo.next_scanline*3*width];
            jpeg_write_scanlines(&cinfo, &row_pointer, 1);
        }
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);

    if (buf_type == BUF_RGBA || buf_type == BUF_BGRA)
        free(rgb_data);
}

const unsigned char *
JpegEncoder::get_jpeg() const
{
    return jpeg;
}

unsigned int
JpegEncoder::get_jpeg_len() const
{
    return jpeg_len;
}

void
JpegEncoder::setRect(const Rect &r)
{
    offset = r;
}

