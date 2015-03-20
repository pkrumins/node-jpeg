#ifndef JPEG_H
#define JPEG_H

#include <nan.h>
#include <node.h>
#include <node_buffer.h>

#include "jpeg_encoder.h"

using v8::FunctionTemplate;
using v8::Handle;
using v8::Object;
using v8::String;
using v8::Value;

class Jpeg : public node::ObjectWrap {
    JpegEncoder jpeg_encoder;

    static void UV_JpegEncode(uv_work_t *req);
    static void UV_JpegEncodeAfter(uv_work_t *req);
public:
    static void Initialize(Handle<Object> target);
    Jpeg(unsigned char *ddata, int wwidth, int hheight, buffer_type bbuf_type);
    Handle<Value> JpegEncodeSync();
    void SetQuality(int q);
    void SetSmoothing(int s);

    static NAN_METHOD(New);
    static NAN_METHOD(JpegEncodeSync);
    static NAN_METHOD(JpegEncodeAsync);
    static NAN_METHOD(SetQuality);
    static NAN_METHOD(SetSmoothing);
};

#endif

