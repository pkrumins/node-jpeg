#ifndef DYNAMIC_JPEG_H
#define DYNAMIC_JPEG_H

#include <node.h>
#include <node_buffer.h>

#include <vector>
#include <utility>

#include "common.h"
#include "jpeg_encoder.h"

class DynamicJpegStack : public node::ObjectWrap {
    int bg_width, bg_height;
    int dyn_width, dyn_height;
    Point offset;

    int quality;
    buffer_type buf_type;

    unsigned char *data;

    typedef std::vector<Rect> RectVector;
    RectVector updates;

    std::pair<Point, Point> OptimalDimension();

public:
    DynamicJpegStack(int qquality, buffer_type bbuf_type);

    v8::Handle<v8::Value> JpegEncode();
    void Push(unsigned char *data_buf, int x, int y, int w, int h);
    void SetBackground(unsigned char *data_buf, int w, int h);
    v8::Handle<v8::Value> Dimensions();

    static void Initialize(v8::Handle<v8::Object> target);
    static v8::Handle<v8::Value> New(const v8::Arguments &args);
    static v8::Handle<v8::Value> JpegEncode(const v8::Arguments &args);
    static v8::Handle<v8::Value> Push(const v8::Arguments &args);
    static v8::Handle<v8::Value> SetBackground(const v8::Arguments &args);
    static v8::Handle<v8::Value> Dimensions(const v8::Arguments &args);
};

#endif

