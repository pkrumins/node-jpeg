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

public:
    static void Initialize(v8::Handle<v8::Object> target);
    FixedJpegStack(int wwidth, int hheight, int qquality, buffer_type bbuf_type);
    v8::Handle<v8::Value> JpegEncode();
    void Push(unsigned char *data_buf, int x, int y, int w, int h);

    static v8::Handle<v8::Value> New(const v8::Arguments &args);
    static v8::Handle<v8::Value> JpegEncode(const v8::Arguments &args);
    static v8::Handle<v8::Value> Push(const v8::Arguments &args);
};

#endif

