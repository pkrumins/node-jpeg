#include <node.h>
#include <node_buffer.h>
#include <jpeglib.h>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "jpeg.h"
#include "jpeg_encoder.h"

using namespace v8;
using namespace node;

void
Jpeg::Initialize(v8::Handle<v8::Object> target)
{
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "encode", JpegEncode);
    target->Set(String::NewSymbol("Jpeg"), t->GetFunction());
}

Jpeg::Jpeg(Buffer *ddata, int wwidth, int hheight, int qquality, buffer_type bbuf_type) :
    jpeg_encoder((unsigned char *)ddata->data(), wwidth, hheight, qquality, bbuf_type) {}

Handle<Value>
Jpeg::JpegEncode()
{
    HandleScope scope;

    try {
        jpeg_encoder.encode();
    }
    catch (const char *err) {
        return VException(err);
    }

    return scope.Close(
        Encode(jpeg_encoder.get_jpeg(), jpeg_encoder.get_jpeg_len(), BINARY)
    );
}

Handle<Value>
Jpeg::New(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() != 5)
        return VException("Five arguments required - rgba/rgb buffer, width, height, quality, buffer type");
    if (!Buffer::HasInstance(args[0]))
        return VException("First argument must be Buffer.");
    if (!args[1]->IsInt32())
        return VException("Second argument must be integer width.");
    if (!args[2]->IsInt32())
        return VException("Third argument must be integer height.");
    if (!args[3]->IsInt32())
        return VException("Fourth argument must be integer quality.");
    if (!args[4]->IsString())
        return VException("Fifth argument must be a string. Either 'rgb' or 'rgba'.");

    int w = args[1]->Int32Value();
    int h = args[2]->Int32Value();
    int quality = args[3]->Int32Value();
    String::AsciiValue bt(args[4]->ToString());

    if (w < 0)
        return VException("Width can't be negative.");
    if (h < 0)
        return VException("Height can't be negative.");
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

    Buffer *data_buf = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
    Jpeg *jpeg = new Jpeg(data_buf, w, h, quality, buf_type);
    jpeg->Wrap(args.This());
    return args.This();
}

Handle<Value>
Jpeg::JpegEncode(const Arguments &args)
{
    HandleScope scope;
    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());
    return jpeg->JpegEncode();
}

