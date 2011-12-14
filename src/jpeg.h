#ifndef JPEG_H
#define JPEG_H

#include <node.h>
#include <node_buffer.h>

#include "jpeg_encoder.h"

class Jpeg : public node::ObjectWrap {
    JpegEncoder jpeg_encoder;

    static void EIO_JpegEncode(eio_req *req);
    static int EIO_JpegEncodeAfter(eio_req *req);
public:
    static void Initialize(v8::Handle<v8::Object> target);
    Jpeg(unsigned char *ddata, int wwidth, int hheight, buffer_type bbuf_type);
    v8::Handle<v8::Value> JpegEncodeSync();
    void SetQuality(int q);

    static v8::Handle<v8::Value> New(const v8::Arguments &args);
    static v8::Handle<v8::Value> JpegEncodeSync(const v8::Arguments &args);
    static v8::Handle<v8::Value> JpegEncodeAsync(const v8::Arguments &args);
    static v8::Handle<v8::Value> SetQuality(const v8::Arguments &args);
};

#endif

