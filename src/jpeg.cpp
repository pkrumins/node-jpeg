#include <nan.h>
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

using v8::Object;
using v8::Handle;

void
Jpeg::Initialize(Handle<Object> target)
{
    NanScope();

    Local<FunctionTemplate> t = NanNew<FunctionTemplate>(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "encode", JpegEncodeAsync);
    NODE_SET_PROTOTYPE_METHOD(t, "encodeSync", JpegEncodeSync);
    NODE_SET_PROTOTYPE_METHOD(t, "setQuality", SetQuality);
    NODE_SET_PROTOTYPE_METHOD(t, "setSmoothing", SetSmoothing);
    target->Set(NanNew<String>("Jpeg"), t->GetFunction());
}

Jpeg::Jpeg(unsigned char *ddata, int wwidth, int hheight, buffer_type bbuf_type) :
    jpeg_encoder(ddata, wwidth, hheight, 60, bbuf_type) {}

Handle<Value>
Jpeg::JpegEncodeSync()
{
    try {
        jpeg_encoder.encode();
    }
    catch (const char *err) {
        NanThrowError(err);
    }

    Handle<Object> retbuf = NanNewBufferHandle((const char *) jpeg_encoder.get_jpeg(), jpeg_encoder.get_jpeg_len());
    // memcpy(Buffer::Data(retbuf), , jpeg_len);
    return retbuf;
}

void
Jpeg::SetQuality(int q)
{
    jpeg_encoder.set_quality(q);
}

void
Jpeg::SetSmoothing(int s)
{
    jpeg_encoder.set_smoothing(s);
}

NAN_METHOD(Jpeg::New)
{
    NanScope();

    if (args.Length() < 3) {
        NanThrowError("At least three arguments required - buffer, width, height, [and buffer type]");
    }
    if (!Buffer::HasInstance(args[0])) {
        NanThrowError("First argument must be Buffer.");
    }
    if (!args[1]->IsInt32()) {
        NanThrowError("Second argument must be integer width.");
    }
    if (!args[2]->IsInt32()) {
        NanThrowError("Third argument must be integer height.");
    }

    int w = args[1]->Int32Value();
    int h = args[2]->Int32Value();

    if (w < 0) {
        NanThrowError("Width can't be negative.");
    }
    if (h < 0) {
        NanThrowError("Height can't be negative.");
    }

    buffer_type buf_type = BUF_RGB;
    if (args.Length() == 4) {
        if (!args[3]->IsString()) {
            NanThrowError("Fifth argument must be a string. Either 'rgb', 'bgr', 'rgba' or 'bgra'.");
        }

        NanUtf8String bt(args[3]->ToString());
        if (!(str_eq(*bt, "rgb") || str_eq(*bt, "bgr") ||
            str_eq(*bt, "rgba") || str_eq(*bt, "bgra")))
        {
            NanThrowError("Buffer type must be 'rgb', 'bgr', 'rgba' or 'bgra'.");
        }

        if (str_eq(*bt, "rgb")) {
            buf_type = BUF_RGB;
        } else if (str_eq(*bt, "bgr")) {
            buf_type = BUF_BGR;
        } else if (str_eq(*bt, "rgba")) {
            buf_type = BUF_RGBA;
        } else if (str_eq(*bt, "bgra")) {
            buf_type = BUF_BGRA;
        } else {
            NanThrowError("Buffer type wasn't 'rgb', 'bgr', 'rgba' or 'bgra'.");
        }
    }

    Local<Object> buffer = args[0]->ToObject();
    Jpeg *jpeg = new Jpeg((unsigned char*) Buffer::Data(buffer), w, h, buf_type);
    jpeg->Wrap(args.This());
    NanReturnThis();
}

NAN_METHOD(Jpeg::JpegEncodeSync)
{
    NanScope();

    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());
    NanReturnValue(jpeg->JpegEncodeSync());
}

NAN_METHOD(Jpeg::SetQuality)
{
    NanScope();

    if (args.Length() != 1) {
        NanThrowError("One argument required - quality");
    }

    if (!args[0]->IsInt32()) {
        NanThrowError("First argument must be integer quality");
    }

    int q = args[0]->Int32Value();

    if (q < 0) {
        NanThrowError("Quality must be greater or equal to 0.");
    }
    if (q > 100) {
        NanThrowError("Quality must be less than or equal to 100.");
    }

    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());
    jpeg->SetQuality(q);

    NanReturnUndefined();
}

NAN_METHOD(Jpeg::SetSmoothing)
{
    NanScope();

    if (args.Length() != 1) {
        NanThrowError("One argument required - quality");
    }

    if (!args[0]->IsInt32()) {
        NanThrowError("First argument must be integer quality");
    }

    int s = args[0]->Int32Value();

    if (s < 0) {
        NanThrowError("Smoothing must be greater or equal to 0.");
    }
    if (s > 100) {
        NanThrowError("Smoothing must be less than or equal to 100.");
    }

    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());
    jpeg->SetSmoothing(s);

    NanReturnUndefined();
}

void
Jpeg::UV_JpegEncode(uv_work_t *req)
{
    encode_request *enc_req = (encode_request *)req->data;
    Jpeg *jpeg = (Jpeg *)enc_req->jpeg_obj;

    try {
        jpeg->jpeg_encoder.encode();
        enc_req->jpeg_len = jpeg->jpeg_encoder.get_jpeg_len();
        enc_req->jpeg = (char *)malloc(sizeof(*enc_req->jpeg)*enc_req->jpeg_len);
        if (!enc_req->jpeg) {
            enc_req->error = strdup("malloc in Jpeg::UV_JpegEncode failed.");
            return;
        }
        else {
            memcpy(enc_req->jpeg, jpeg->jpeg_encoder.get_jpeg(), enc_req->jpeg_len);
        }
    }
    catch (const char *err) {
        enc_req->error = strdup(err);
    }
}

void 
Jpeg::UV_JpegEncodeAfter(uv_work_t *req)
{
    NanScope();

    encode_request *enc_req = (encode_request *)req->data;
    delete req;

    Handle<Value> argv[2];

    if (enc_req->error) {
        argv[0] = NanUndefined();
        argv[1] = NanError(enc_req->error);
    }
    else {
        Handle<Object> buf = NanNewBufferHandle(enc_req->jpeg, enc_req->jpeg_len);
        argv[0] = buf;
        argv[1] = NanUndefined();
    }

    TryCatch try_catch; // don't quite see the necessity of this

    enc_req->callback->Call(2, argv);

    if (try_catch.HasCaught())
        FatalException(try_catch);

    delete enc_req->callback;
    free(enc_req->jpeg);
    free(enc_req->error);

    ((Jpeg *)enc_req->jpeg_obj)->Unref();
    free(enc_req);
}

NAN_METHOD(Jpeg::JpegEncodeAsync)
{
    NanScope();

    if (args.Length() != 1) {
        NanThrowError("One argument required - callback function.");
    }

    if (!args[0]->IsFunction()) {
        NanThrowError("First argument must be a function.");
    }

    Local<Function> callback = args[0].As<Function>();
    Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());

    encode_request *enc_req = (encode_request *)malloc(sizeof(*enc_req));
    if (!enc_req) {
        NanThrowError("malloc in Jpeg::JpegEncodeAsync failed.");
    }

    enc_req->callback = new NanCallback(callback);
    enc_req->jpeg_obj = jpeg;
    enc_req->jpeg = NULL;
    enc_req->jpeg_len = 0;
    enc_req->error = NULL;

    uv_work_t* req = new uv_work_t;
    req->data = enc_req;
    uv_queue_work(uv_default_loop(), req, UV_JpegEncode, (uv_after_work_cb)UV_JpegEncodeAfter);

    jpeg->Ref();

    NanReturnUndefined();
}

