#ifndef DYNAMIC_JPEG_H
#define DYNAMIC_JPEG_H

#include <node.h>
#include <node_buffer.h>

#include <vector>
#include <utility>

#include "common.h"
#include "jpeg_encoder.h"

class DynamicJpegStack : public node::ObjectWrap {
    int quality;
    buffer_type buf_type;

    unsigned char *data;

    int bg_width, bg_height; // background width and height after setBackground
    Rect dyn_rect; // rect of dynamic push area (updated after each push)

    void update_optimal_dimension(int x, int y, int w, int h);

    static void UV_JpegEncode(uv_work_t *req);
    static void UV_JpegEncodeAfter(uv_work_t *req);
public:
    DynamicJpegStack(buffer_type bbuf_type);
    ~DynamicJpegStack();

    v8::Handle<v8::Value> JpegEncodeSync();
    void Push(unsigned char *data_buf, int x, int y, int w, int h);
    void SetBackground(unsigned char *data_buf, int w, int h);
    void SetQuality(int q);
    v8::Handle<v8::Value> Dimensions();
    void Reset();

    static void Initialize(v8::Handle<v8::Object> target);
    static NAN_METHOD(New);
    static NAN_METHOD(JpegEncodeSync);
    static NAN_METHOD(JpegEncodeAsync);
    static NAN_METHOD(Push);
    static NAN_METHOD(SetBackground);
    static NAN_METHOD(SetQuality);
    static NAN_METHOD(Dimensions);
    static NAN_METHOD(Reset);
};

#endif

