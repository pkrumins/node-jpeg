#include <nan.h>
#include <node.h>
#include <node_buffer.h>
#include <jpeglib.h>
#include <cstdlib>
#include <cstring>

#include "common.h"
#include "fixed_jpeg_stack.h"
#include "jpeg_encoder.h"

using v8::Object;
using v8::Handle;
using v8::Local;
using v8::Function;
using v8::FunctionTemplate;
using v8::String;

void
FixedJpegStack::Initialize(Handle<Object> target)
{
    NanScope();

    Local<FunctionTemplate> t = NanNew<FunctionTemplate>(New);
    t->InstanceTemplate()->SetInternalFieldCount(1);
    NODE_SET_PROTOTYPE_METHOD(t, "encode", JpegEncodeAsync);
    NODE_SET_PROTOTYPE_METHOD(t, "encodeSync", JpegEncodeSync);
    NODE_SET_PROTOTYPE_METHOD(t, "push", Push);
    NODE_SET_PROTOTYPE_METHOD(t, "setQuality", SetQuality);
    target->Set(NanNew<String>("FixedJpegStack"), t->GetFunction());
}

FixedJpegStack::FixedJpegStack(int wwidth, int hheight, buffer_type bbuf_type) :
    width(wwidth), height(hheight), quality(60), buf_type(bbuf_type)
{
    data = (unsigned char *)calloc(width*height*3, sizeof(*data));
    if (!data) {
        throw "calloc in FixedJpegStack::FixedJpegStack failed!";
    }
}

Handle<Value>
FixedJpegStack::JpegEncodeSync()
{
    JpegEncoder jpeg_encoder(data, width, height, quality, BUF_RGB);
    jpeg_encoder.encode();
    Handle<Object> retbuf = NanNewBufferHandle((const char*) jpeg_encoder.get_jpeg(), jpeg_encoder.get_jpeg_len());
    return retbuf;
}

void
FixedJpegStack::Push(unsigned char *data_buf, int x, int y, int w, int h)
{
    int start = y*width*3 + x*3;

    switch (buf_type) {
    case BUF_RGB:
        for (int i = 0; i < h; i++) {
            unsigned char *datap = &data[start + i*width*3];
            for (int j = 0; j < w; j++) {
                *datap++ = *data_buf++;
                *datap++ = *data_buf++;
                *datap++ = *data_buf++;
            }
        }
        break;

    case BUF_BGR:
        for (int i = 0; i < h; i++) {
            unsigned char *datap = &data[start + i*width*3];
            for (int j = 0; j < w; j++) {
                *datap++ = *(data_buf+2);
                *datap++ = *(data_buf+1);
                *datap++ = *data_buf;
                data_buf+=3;
            }
        }
        break;

    case BUF_RGBA:
        for (int i = 0; i < h; i++) {
            unsigned char *datap = &data[start + i*width*3];
            for (int j = 0; j < w; j++) {
                *datap++ = *data_buf++;
                *datap++ = *data_buf++;
                *datap++ = *data_buf++;
                data_buf++;
            }
        }
        break;

    case BUF_BGRA:
        for (int i = 0; i < h; i++) {
            unsigned char *datap = &data[start + i*width*3];
            for (int j = 0; j < w; j++) {
                *datap++ = *(data_buf+2);
                *datap++ = *(data_buf+1);
                *datap++ = *data_buf;
                data_buf += 4;
            }
        }
        break;

    default:
        throw "Unexpected buf_type in FixedJpegStack::Push";
    }
}


void
FixedJpegStack::SetQuality(int q)
{
    quality = q;
}

NAN_METHOD(FixedJpegStack::New)
{
    NanScope();

    if (args.Length() < 2) {
        NanThrowError("At least two arguments required - width, height, [and buffer type]");
    }
    if (!args[0]->IsInt32()) {
        NanThrowError("First argument must be integer width.");
    }
    if (!args[1]->IsInt32()) {
        NanThrowError("Second argument must be integer height.");
    }

    int w = args[0]->Int32Value();
    int h = args[1]->Int32Value();

    if (w < 0) {
        NanThrowError("Width can't be negative.");
    }
    if (h < 0) {
        NanThrowError("Height can't be negative.");
    }

    buffer_type buf_type = BUF_RGB;
    if (args.Length() == 3) {
        if (!args[2]->IsString()) {
            NanThrowError("Third argument must be a string. Either 'rgb', 'bgr', 'rgba' or 'bgra'.");
        }

        NanUtf8String bt(args[2]->ToString());
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

    try {
        FixedJpegStack *jpeg = new FixedJpegStack(w, h, buf_type);
        jpeg->Wrap(args.This());
        NanReturnThis();
    }
    catch (const char *err) {
        NanThrowError(err);
    }
}

NAN_METHOD(FixedJpegStack::JpegEncodeSync)
{
    NanScope();
    FixedJpegStack *jpeg = ObjectWrap::Unwrap<FixedJpegStack>(args.This());
    try {
        NanReturnValue(jpeg->JpegEncodeSync());
    } catch (const char *err) {
        NanThrowError(err);
    }
}

NAN_METHOD(FixedJpegStack::Push)
{
    NanScope();

    if (!node::Buffer::HasInstance(args[0])) {
        NanThrowError("First argument must be Buffer.");
    }
    if (!args[1]->IsInt32()) {
        NanThrowError("Second argument must be integer x.");
    }
    if (!args[2]->IsInt32()) {
        NanThrowError("Third argument must be integer y.");
    }
    if (!args[3]->IsInt32()) {
        NanThrowError("Fourth argument must be integer w.");
    }
    if (!args[4]->IsInt32()) {
        NanThrowError("Fifth argument must be integer h.");
    }

    FixedJpegStack *jpeg = ObjectWrap::Unwrap<FixedJpegStack>(args.This());
    Local<Object> data_buf = args[0]->ToObject();
    int x = args[1]->Int32Value();
    int y = args[2]->Int32Value();
    int w = args[3]->Int32Value();
    int h = args[4]->Int32Value();

    if (x < 0) {
        NanThrowError("Coordinate x smaller than 0.");
    }
    if (y < 0) {
        NanThrowError("Coordinate y smaller than 0.");
    }
    if (w < 0) {
        NanThrowError("Width smaller than 0.");
    }
    if (h < 0) {
        NanThrowError("Height smaller than 0.");
    }
    if (x >= jpeg->width) {
        NanThrowError("Coordinate x exceeds FixedJpegStack's dimensions.");
    }
    if (y >= jpeg->height) {
        NanThrowError("Coordinate y exceeds FixedJpegStack's dimensions.");
    }
    if (x+w > jpeg->width) {
        NanThrowError("Pushed fragment exceeds FixedJpegStack's width.");
    }
    if (y+h > jpeg->height) {
        NanThrowError("Pushed fragment exceeds FixedJpegStack's height.");
    }

    jpeg->Push((unsigned char *)node::Buffer::Data(data_buf), x, y, w, h);

    NanReturnUndefined();
}

NAN_METHOD(FixedJpegStack::SetQuality)
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

    FixedJpegStack *jpeg = ObjectWrap::Unwrap<FixedJpegStack>(args.This());
    jpeg->SetQuality(q);

    NanReturnUndefined();
}

void
FixedJpegStack::UV_JpegEncode(uv_work_t *req)
{
    encode_request *enc_req = (encode_request *)req->data;
    FixedJpegStack *jpeg = (FixedJpegStack *)enc_req->jpeg_obj;

    try {
        JpegEncoder encoder(jpeg->data, jpeg->width, jpeg->height, jpeg->quality, BUF_RGB);
        encoder.encode();
        enc_req->jpeg_len = encoder.get_jpeg_len();
        enc_req->jpeg = (char *)malloc(sizeof(*enc_req->jpeg)*enc_req->jpeg_len);
        if (!enc_req->jpeg) {
            enc_req->error = strdup("malloc in FixedJpegStack::UV_JpegEncode failed.");
            return;
        }
        else {
            memcpy(enc_req->jpeg, encoder.get_jpeg(), enc_req->jpeg_len);
        }
    }
    catch (const char *err) {
        enc_req->error = strdup(err);
    }
}

void 
FixedJpegStack::UV_JpegEncodeAfter(uv_work_t *req)
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

    enc_req->callback->Call(2, argv);

    delete enc_req->callback;
    free(enc_req->jpeg);
    free(enc_req->error);

    ((FixedJpegStack *)enc_req->jpeg_obj)->Unref();
    free(enc_req);
}

NAN_METHOD(FixedJpegStack::JpegEncodeAsync)
{
    NanScope();

    if (args.Length() != 1) {
        NanThrowError("One argument required - callback function.");
    }

    if (!args[0]->IsFunction()) {
        NanThrowError("First argument must be a function.");
    }

    Local<Function> callback = args[0].As<Function>();
    FixedJpegStack *jpeg = ObjectWrap::Unwrap<FixedJpegStack>(args.This());

    encode_request *enc_req = (encode_request *)malloc(sizeof(*enc_req));
    if (!enc_req) {
        NanThrowError("malloc in FixedJpegStack::JpegEncodeAsync failed.");
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

