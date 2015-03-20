#ifndef STACKED_JPEG_H
#define STACKED_JPEG_H

#include <node.h>
#include <node_buffer.h>

#include "common.h"
#include "jpeg_encoder.h"

class FixedJpegStack : public node::ObjectWrap {
    int width, height, quality;
    buffer_type buf_type;

    unsigned char *data;

    static void UV_JpegEncode(uv_work_t *req);
    static void UV_JpegEncodeAfter(uv_work_t *req);

public:
    static void Initialize(v8::Handle<v8::Object> target);
    FixedJpegStack(int wwidth, int hheight, buffer_type bbuf_type);
    v8::Handle<v8::Value> JpegEncodeSync();
    void Push(unsigned char *data_buf, int x, int y, int w, int h);
    void SetQuality(int q);

    static NAN_METHOD(New);
    static NAN_METHOD(JpegEncodeSync);
    static NAN_METHOD(JpegEncodeAsync);
    static NAN_METHOD(Push);
    static NAN_METHOD(SetQuality);
};

#endif

