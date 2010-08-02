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
    NODE_SET_PROTOTYPE_METHOD(t, "setQuality", SetQuality);
    target->Set(String::NewSymbol("Jpeg"), t->GetFunction());
}

Jpeg::Jpeg(Buffer *ddata, int wwidth, int hheight, buffer_type bbuf_type) :
    jpeg_encoder((unsigned char *)ddata->data(), wwidth, hheight, 60, bbuf_type) {}

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

void
Jpeg::SetQuality(int q)
{
    jpeg_encoder.set_quality(q);
}

Handle<Value>
Jpeg::New(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() < 3)
        return VException("At least three arguments required - buffer, width, height, [and buffer type]");
    if (!Buffer::HasInstance(args[0]))
        return VException("First argument must be Buffer.");
    if (!args[1]->IsInt32())
        return VException("Second argument must be integer width.");
    if (!args[2]->IsInt32())
        return VException("Third argument must be integer height.");

    int w = args[1]->Int32Value();
    int h = args[2]->Int32Value();

    if (w < 0)
        return VException("Width can't be negative.");
    if (h < 0)
        return VException("Height can't be negative.");

    buffer_type buf_type = BUF_RGB;
    if (args.Length() == 4) {
        if (!args[3]->IsString())
            return VException("Fifth argument must be a string. Either 'rgb', 'bgr', 'rgba' or 'bgra'.");

        String::AsciiValue bt(args[3]->ToString());
        if (!(str_eq(*bt, "rgb") || str_eq(*bt, "bgr") ||
            str_eq(*bt, "rgba") || str_eq(*bt, "bgra")))
        {
            return VException("Buffer type must be 'rgb', 'bgr', 'rgba' or 'bgra'.");
        }

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
    }

    Buffer *data_buf = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
    Jpeg *jpeg = new Jpeg(data_buf, w, h, buf_type);
    jpeg->Wrap(args.This());
    return args.This();
}

Handle<Value>
Jpeg::JpegEncode(const Arguments &args)
{
    HandleScope scope;

    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());
    return scope.Close(jpeg->JpegEncode());
}

Handle<Value>
Jpeg::SetQuality(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() != 1)
        return VException("One argument required - quality");

    if (!args[0]->IsInt32())
        return VException("First argument must be integer quality");

    int q = args[0]->Int32Value();

    if (q < 0) 
        return VException("Quality must be greater or equal to 0.");
    if (q > 100)
        return VException("Quality must be less than or equal to 100.");

    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());
    jpeg->SetQuality(q);

    return Undefined();
}

