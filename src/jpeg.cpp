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
    NODE_SET_PROTOTYPE_METHOD(t, "encode", JpegEncodeAsync);
    NODE_SET_PROTOTYPE_METHOD(t, "encodeSync", JpegEncodeSync);
    NODE_SET_PROTOTYPE_METHOD(t, "setQuality", SetQuality);
    target->Set(String::NewSymbol("Jpeg"), t->GetFunction());
}

Jpeg::Jpeg(Buffer *ddata, int wwidth, int hheight, buffer_type bbuf_type) :
    jpeg_encoder((unsigned char *)ddata->data(), wwidth, hheight, 60, bbuf_type) {}

Handle<Value>
Jpeg::JpegEncodeSync()
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
Jpeg::JpegEncodeSync(const Arguments &args)
{
    HandleScope scope;

    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());
    return scope.Close(jpeg->JpegEncodeSync());
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

int
Jpeg::EIO_JpegEncode(eio_req *req)
{
    encode_request *enc_req = (encode_request *)req->data;
    Jpeg *jpeg = (Jpeg *)enc_req->jpeg_obj;

    try {
        jpeg->jpeg_encoder.encode();
        enc_req->jpeg_len = jpeg->jpeg_encoder.get_jpeg_len();
        enc_req->jpeg = (char *)malloc(sizeof(*enc_req->jpeg)*enc_req->jpeg_len);
        if (!enc_req->jpeg) {
            enc_req->error = strdup("malloc in Jpeg::EIO_JpegEncode failed.");
            return 0;
        }
        else {
            memcpy(enc_req->jpeg, jpeg->jpeg_encoder.get_jpeg(), enc_req->jpeg_len);
        }
    }
    catch (const char *err) {
        enc_req->error = strdup(err);
    }

    return 0;
}

int 
Jpeg::EIO_JpegEncodeAfter(eio_req *req)
{
    HandleScope scope;

    ev_unref(EV_DEFAULT_UC);
    encode_request *enc_req = (encode_request *)req->data;

    Handle<Value> argv[2];

    if (enc_req->error) {
        argv[0] = Undefined();
        argv[1] = ErrorException(enc_req->error);
    }
    else {
        argv[0] = Local<Value>::New(Encode(enc_req->jpeg, enc_req->jpeg_len, BINARY));
        argv[1] = Undefined();
    }

    TryCatch try_catch; // don't quite see the necessity of this

    enc_req->callback->Call(Context::GetCurrent()->Global(), 2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    enc_req->callback.Dispose();
    free(enc_req->jpeg);
    free(enc_req->error);

    ((Jpeg *)enc_req->jpeg_obj)->Unref();
    free(enc_req);

    return 0;
}

Handle<Value>
Jpeg::JpegEncodeAsync(const Arguments &args)
{
    HandleScope scope;

    if (args.Length() != 1)
        return VException("One argument required - callback function.");

    if (!args[0]->IsFunction())
        return VException("First argument must be a function.");

    Local<Function> callback = Local<Function>::Cast(args[0]);
    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());

    encode_request *enc_req = (encode_request *)malloc(sizeof(*enc_req));
    if (!enc_req)
        return VException("malloc in Jpeg::JpegEncodeAsync failed.");

    enc_req->callback = Persistent<Function>::New(callback);
    enc_req->jpeg_obj = jpeg;
    enc_req->jpeg = NULL;
    enc_req->jpeg_len = 0;
    enc_req->error = NULL;

    eio_custom(EIO_JpegEncode, EIO_PRI_DEFAULT, EIO_JpegEncodeAfter, enc_req);

    ev_ref(EV_DEFAULT_UC);
    jpeg->Ref();

    return Undefined();
}

