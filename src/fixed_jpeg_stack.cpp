#include <node.h>
#include <node_buffer.h>
#include <jpeglib.h>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "fixed_jpeg_stack.h"
#include "jpeg_encoder.h"

using namespace v8;
using namespace node;

void
FixedJpegStack::Initialize(v8::Handle<v8::Object> target)
{
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "encode", JpegEncode);
    NODE_SET_PROTOTYPE_METHOD(t, "push", Push);
    target->Set(String::NewSymbol("FixedJpegStack"), t->GetFunction());
}

FixedJpegStack::FixedJpegStack(int wwidth, int hheight, int qquality, buffer_type bbuf_type) :
    width(wwidth), height(hheight), quality(qquality), buf_type(bbuf_type)
{
    data = (unsigned char *)calloc(width*height*3, sizeof(*data));
    if (!data) throw "calloc in FixedJpegStack::FixedJpegStack failed!";
}

Handle<Value>
FixedJpegStack::JpegEncode()
{
    HandleScope scope;

    try {
        JpegEncoder jpeg_encoder(data, width, height, quality, BUF_RGB);
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
FixedJpegStack::Push(unsigned char *data_buf, int x, int y, int w, int h)
{
    int start = y*width*3 + x*3;

    if (buf_type == BUF_RGB) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < 3*w; j+=3) {
                data[start + i*width*3 + j] = data_buf[i*w*3 + j];
                data[start + i*width*3 + j + 1] = data_buf[i*w*3 + j + 1];
                data[start + i*width*3 + j + 2] = data_buf[i*w*3 + j + 2];
            }
        }
    }
    else if (buf_type == BUF_BGR) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < 3*w; j+=3) {
                data[start + i*width*3 + j] = data_buf[i*w*3 + j + 2];
                data[start + i*width*3 + j + 1] = data_buf[i*w*3 + j + 1];
                data[start + i*width*3 + j + 2] = data_buf[i*w*3 + j];
            }
        }
    }
    else if (buf_type == BUF_RGBA) {
        for (int i = 0; i < h; i++) {
            for (int j = 0, k = 0; j < 3*w; j+=3, k+=4) {
                data[start + i*width*3 + j] = data_buf[i*w*4 + k];
                data[start + i*width*3 + j + 1] = data_buf[i*w*4 + k + 1];
                data[start + i*width*3 + j + 2] = data_buf[i*w*4 + k + 2];
            }
        }
    }
    else if (buf_type == BUF_BGRA) {
        for (int i = 0; i < h; i++) {
            for (int j = 0, k = 0; j < 3*w; j+=3, k+=4) {
                data[start + i*width*3 + j] = data_buf[i*w*4 + k + 2];
                data[start + i*width*3 + j + 1] = data_buf[i*w*4 + k + 1];
                data[start + i*width*3 + j + 2] = data_buf[i*w*4 + k];
            }
        }
    }
}

Handle<Value>
FixedJpegStack::New(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() != 4)
        return VException("Four arguments required - width, height, quality and buffer type");
    if (!args[0]->IsInt32())
        return VException("First argument must be integer width.");
    if (!args[1]->IsInt32())
        return VException("Second argument must be integer height.");
    if (!args[2]->IsInt32())
        return VException("Third argument must be integer quality.");
    if (!args[3]->IsString())
        return VException("Fourth argument must be a string. Either 'rgb', 'bgr', 'rgba' or 'bgra'.");

    int w = args[0]->Int32Value();
    int h = args[1]->Int32Value();
    int quality = args[2]->Int32Value();
    String::AsciiValue bt(args[3]->ToString());

    if (w < 0)
        return VException("Width can't be negative.");
    if (h < 0)
        return VException("Height can't be negative.");
    if (quality < 0 || quality > 100)
        return VException("Quality must be between 0 and 100 (inclusive).");
    if (!(str_eq(*bt, "rgb") || str_eq(*bt, "bgr") || str_eq(*bt, "rgba") || str_eq(*bt, "bgra")))
        return VException("Buffer type must be 'rgb', 'bgr', 'rgba' or 'bgra'.");

    buffer_type buf_type;
    if (str_eq(*bt, "rgb"))
        buf_type = BUF_RGB;
    else if (str_eq(*bt, "bgr"))
        buf_type = BUF_BGR;
    else if (str_eq(*bt, "rgba"))
        buf_type = BUF_RGBA;
    else if (str_eq(*bt, "bgra"))
        buf_type = BUF_BGRA;
    else 
        return VException("Buffer type wasn't 'rgb', 'bgr', 'rgba' or 'bgra'.");

    try {
        FixedJpegStack *jpeg = new FixedJpegStack(w, h, quality, buf_type);
        jpeg->Wrap(args.This());
        return args.This();
    }
    catch (const char *err) {
        return VException(err);
    }
}

Handle<Value>
FixedJpegStack::JpegEncode(const Arguments &args)
{
    HandleScope scope;
    FixedJpegStack *jpeg = ObjectWrap::Unwrap<FixedJpegStack>(args.This());
    return jpeg->JpegEncode();
}

Handle<Value>
FixedJpegStack::Push(const Arguments &args)
{
    HandleScope scope;

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

    FixedJpegStack *jpeg = ObjectWrap::Unwrap<FixedJpegStack>(args.This());
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
    if (x >= jpeg->width) 
        return VException("Coordinate x exceeds FixedJpegStack's dimensions.");
    if (y >= jpeg->height) 
        return VException("Coordinate y exceeds FixedJpegStack's dimensions.");
    if (x+w > jpeg->width) 
        return VException("Pushed fragment exceeds FixedJpegStack's width.");
    if (y+h > jpeg->height) 
        return VException("Pushed fragment exceeds FixedJpegStack's height.");

    jpeg->Push((unsigned char *)data_buf->data(), x, y, w, h);

    return Undefined();
}

