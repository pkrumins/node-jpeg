#ifndef JPEG_H
#define JPEG_H

#include <node.h>
#include <node_buffer.h>

#include "jpeg_encoder.h"

class Jpeg : public node::ObjectWrap {
    JpegEncoder jpeg_encoder;

public:
    static void Initialize(v8::Handle<v8::Object> target);
    Jpeg(node::Buffer *ddata, int wwidth, int hheight, int qquality, buffer_type bbuf_type);
    v8::Handle<v8::Value> JpegEncode();

    static v8::Handle<v8::Value> New(const v8::Arguments &args);
    static v8::Handle<v8::Value> JpegEncode(const v8::Arguments &args);
};

#endif

