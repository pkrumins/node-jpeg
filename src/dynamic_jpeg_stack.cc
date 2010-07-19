#include <node.h>
#include <node_buffer.h>
#include <jpeglib.h>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "dynamic_jpeg_stack.h"
#include "jpeg_encoder.h"

using namespace v8;
using namespace node;

void
DynamicJpegStack::Initialize(v8::Handle<v8::Object> target)
{
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "encode", JpegEncode);
    NODE_SET_PROTOTYPE_METHOD(t, "push", Push);
    NODE_SET_PROTOTYPE_METHOD(t, "reset", Reset);
    NODE_SET_PROTOTYPE_METHOD(t, "setBackground", SetBackground);
    NODE_SET_PROTOTYPE_METHOD(t, "dimensions", Dimensions);
    target->Set(String::NewSymbol("DynamicJpegStack"), t->GetFunction());
}

DynamicJpegStack::DynamicJpegStack(int qquality, buffer_type bbuf_type) :
    quality(qquality), buf_type(bbuf_type),
    dyn_rect(-1, -1, 0, 0),
    bg_width(0), bg_height(0), data(NULL) {}

void
DynamicJpegStack::UpdateOptimalDimension(int x, int y, int w, int h)
{
    if (dyn_rect.x == -1 || x < dyn_rect.x)
        dyn_rect.x = x;
    if (dyn_rect.y == -1 || y < dyn_rect.y)
        dyn_rect.y = y;
    
    if (dyn_rect.w == 0)
        dyn_rect.w = w;
    if (dyn_rect.h == 0)
        dyn_rect.h = h;

    int ww = w - (dyn_rect.w - (x - dyn_rect.x));
    if (ww > 0)
        dyn_rect.w += ww;

    int hh = h - (dyn_rect.h - (y - dyn_rect.y));
    if (hh > 0)
        dyn_rect.h += hh;

}

Handle<Value>
DynamicJpegStack::JpegEncode()
{
    HandleScope scope;

    printf("%d %d\n", dyn_rect.w, dyn_rect.h);

    try {
        JpegEncoder jpeg_encoder(data, bg_width, bg_height, quality, BUF_RGB);
        jpeg_encoder.setRect(Rect(dyn_rect.x, dyn_rect.y, dyn_rect.w, dyn_rect.h));
        jpeg_encoder.encode();
        return scope.Close(
            Encode(jpeg_encoder.get_jpeg(), jpeg_encoder.get_jpeg_len(), BINARY)
        );
    }
    catch (const char *err) {
        return VException(err);
    }
}

void
DynamicJpegStack::Push(unsigned char *data_buf, int x, int y, int w, int h)
{
    UpdateOptimalDimension(x, y, w, h);

    int start = y*bg_width*3 + x*3;

    if (buf_type == BUF_RGB || buf_type == BUF_BGR) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < 3*w; j+=3) {
                data[start + i*bg_width*3 + j] = data_buf[i*w*3 + j];
                data[start + i*bg_width*3 + j + 1] = data_buf[i*w*3 + j + 1];
                data[start + i*bg_width*3 + j + 2] = data_buf[i*w*3 + j + 2];
            }
        }
    }
    else {
        for (int i = 0; i < h; i++) {
            for (int j = 0, k = 0; j < 3*w; j+=3, k+=4) {
                data[start + i*bg_width*3 + j] = data_buf[i*w*4 + k];
                data[start + i*bg_width*3 + j + 1] = data_buf[i*w*4 + k + 1];
                data[start + i*bg_width*3 + j + 2] = data_buf[i*w*4 + k + 2];
            }
        }
    }
}

void
DynamicJpegStack::SetBackground(unsigned char *data_buf, int w, int h)
{
    if (data) free(data);

    if (buf_type == BUF_RGB) {
        data = (unsigned char *)malloc(sizeof(*data)*w*h*3);
        if (!data) throw "malloc failed in DynamicJpegStack::SetBackground";
        memcpy(data, data_buf, w*h*3);
    }
    else {
        data = rgba_to_rgb(data_buf, w*h*4);
        if (!data) throw "malloc failed in DynamicJpegStack::SetBackground";
    }
    bg_width = w;
    bg_height = h;
}

void
DynamicJpegStack::Reset()
{
    dyn_rect = Rect(-1, -1, 0, 0);
}

Handle<Value>
DynamicJpegStack::Dimensions()
{
    HandleScope scope;

    Local<Object> dim = Object::New();
    dim->Set(String::NewSymbol("x"), Integer::New(dyn_rect.x));
    dim->Set(String::NewSymbol("y"), Integer::New(dyn_rect.y));
    dim->Set(String::NewSymbol("width"), Integer::New(dyn_rect.w));
    dim->Set(String::NewSymbol("height"), Integer::New(dyn_rect.h));

    return scope.Close(dim);
}

Handle<Value>
DynamicJpegStack::New(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() != 2)
        return VException("Two arguments required - quality and buffer type");
    if (!args[0]->IsInt32())
        return VException("First argument must be integer quality.");
    if (!args[1]->IsString())
        return VException("Second argument must be a string. Either 'rgb' or 'rgba'.");

    int quality = args[0]->Int32Value();
    String::AsciiValue bt(args[1]->ToString());

    if (quality < 0 || quality > 100)
        return VException("Quality must be between 0 and 100 (inclusive).");
    if (!(str_eq(*bt, "rgb") || str_eq(*bt, "rgba")))
        return VException("Buffer type must be either 'rgb' or 'rgba'.");

    buffer_type buf_type;
    if (str_eq(*bt, "rgb"))
        buf_type = BUF_RGB;
    else if (str_eq(*bt, "rgba"))
        buf_type = BUF_RGBA;
    else 
        return VException("Buffer type wasn't 'rgb' or 'rgba'");

    DynamicJpegStack *jpeg = new DynamicJpegStack(quality, buf_type);
    jpeg->Wrap(args.This());
    return args.This();
}

Handle<Value>
DynamicJpegStack::JpegEncode(const Arguments &args)
{
    HandleScope scope;
    DynamicJpegStack *jpeg = ObjectWrap::Unwrap<DynamicJpegStack>(args.This());
    return jpeg->JpegEncode();
}

Handle<Value>
DynamicJpegStack::Push(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() != 5)
        return VException("Five arguments required - buffer, x, y, width, height.");

    if (!Buffer::HasInstance(args[0]))
        return VException("First argument must be Buffer.");
    if (!args[1]->IsInt32())
        return VException("Second argument must be integer x.");
    if (!args[2]->IsInt32())
        return VException("Third argument must be integer y.");
    if (!args[3]->IsInt32())
        return VException("Fourth argument must be integer w.");
    if (!args[4]->IsInt32())
        return VException("Fifth argument must be integer h.");

    DynamicJpegStack *jpeg = ObjectWrap::Unwrap<DynamicJpegStack>(args.This());

    if (!jpeg->data)
        return VException("No background has been set, use setBackground or setSolidBackground to set.");

    Buffer *data_buf = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
    int x = args[1]->Int32Value();
    int y = args[2]->Int32Value();
    int w = args[3]->Int32Value();
    int h = args[4]->Int32Value();

    if (x < 0)
        return VException("Coordinate x smaller than 0.");
    if (y < 0)
        return VException("Coordinate y smaller than 0.");
    if (w < 0)
        return VException("Width smaller than 0.");
    if (h < 0)
        return VException("Height smaller than 0.");
    if (x >= jpeg->bg_width) 
        return VException("Coordinate x exceeds DynamicJpegStack's background dimensions.");
    if (y >= jpeg->bg_height) 
        return VException("Coordinate y exceeds DynamicJpegStack's background dimensions.");
    if (x+w > jpeg->bg_width) 
        return VException("Pushed fragment exceeds DynamicJpegStack's width.");
    if (y+h > jpeg->bg_height) 
        return VException("Pushed fragment exceeds DynamicJpegStack's height.");

    jpeg->Push((unsigned char *)data_buf->data(), x, y, w, h);

    return Undefined();
}

Handle<Value>
DynamicJpegStack::SetBackground(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() != 3)
        return VException("Four arguments required - buffer, width, height");
    if (!Buffer::HasInstance(args[0]))
        return VException("First argument must be Buffer.");
    if (!args[1]->IsInt32())
        return VException("Second argument must be integer width.");
    if (!args[2]->IsInt32())
        return VException("Third argument must be integer height.");

    DynamicJpegStack *jpeg = ObjectWrap::Unwrap<DynamicJpegStack>(args.This());
    Buffer *data_buf = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
    int w = args[1]->Int32Value();
    int h = args[2]->Int32Value();

    if (w < 0)
        return VException("Coordinate x smaller than 0.");
    if (h < 0)
        return VException("Coordinate y smaller than 0.");

    try {
        jpeg->SetBackground((unsigned char *)data_buf->data(), w, h);
    }
    catch (const char *err) {
        return VException(err);
    }

    return Undefined();
}

Handle<Value>
DynamicJpegStack::Reset(const Arguments &args)
{
    HandleScope scope;

    DynamicJpegStack *jpeg = ObjectWrap::Unwrap<DynamicJpegStack>(args.This());
    jpeg->Reset();
    return Undefined();
}

Handle<Value>
DynamicJpegStack::Dimensions(const Arguments &args)
{
    HandleScope scope;

    DynamicJpegStack *jpeg = ObjectWrap::Unwrap<DynamicJpegStack>(args.This());
    return scope.Close(jpeg->Dimensions());
}

