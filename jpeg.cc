#include <node.h>
#include <node_events.h>
#include <node_buffer.h>
#include <jpeglib.h>
#include <cstdlib>
#include <cstring>

using namespace v8;
using namespace node;

static Persistent<String> end_symbol;
static Persistent<String> data_symbol;

typedef enum { BUF_RGBA, BUF_RGB } buf_type_t;

class Jpeg : public EventEmitter {
private:
    int width_;
    int height_;
    int quality_;
    buf_type_t buf_type_;
    Buffer *rgba_;

public:
    static void
    Initialize(v8::Handle<v8::Object> target)
    {
        HandleScope scope;
        Local<FunctionTemplate> t = FunctionTemplate::New(New);
        t->Inherit(EventEmitter::constructor_template);
        t->InstanceTemplate()->SetInternalFieldCount(1);
        end_symbol = NODE_PSYMBOL("end");
        data_symbol = NODE_PSYMBOL("data");
        NODE_SET_PROTOTYPE_METHOD(t, "encode", JpegEncode);
        target->Set(String::NewSymbol("Jpeg"), t->GetFunction());
    }

    Jpeg(Buffer *rgba, int width, int height, int quality, buf_type_t buf_type) :
        EventEmitter(), rgba_(rgba), width_(width), height_(height),
        quality_(quality), buf_type_(buf_type) { }

    static unsigned char *
    rgba_to_rgb(unsigned char *rgba, int rgba_size)
    {
        int rgb_size = rgba_size/4*3;
        unsigned char *rgb = (unsigned char *)malloc(sizeof(unsigned char)*rgb_size);
        if (!rgb) return NULL;
        int i, j;
        for (i=0,j=0;i<rgba_size;i+=4,j+=3) {
            rgb[j] = *(rgba+i);
            rgb[j+1] = *(rgba+i+1);
            rgb[j+2] = *(rgba+i+2);
        }
        return rgb;
    }

    void JpegEncode() {
        unsigned char *mem_dest;
        unsigned long outsize = 0;

        struct jpeg_compress_struct cinfo;
        struct jpeg_error_mgr jerr;

        cinfo.err = jpeg_std_error(&jerr);

        jpeg_create_compress(&cinfo);
        jpeg_mem_dest(&cinfo, &mem_dest, &outsize);

        cinfo.image_width = width_;
        cinfo.image_height = height_;
        cinfo.input_components = 3;
        cinfo.in_color_space = JCS_RGB;

        jpeg_set_defaults(&cinfo);
        jpeg_set_quality(&cinfo, quality_, TRUE);
        jpeg_start_compress(&cinfo, TRUE);

        unsigned char *rgb_data;
        if (buf_type_ == BUF_RGBA) {
            rgb_data = rgba_to_rgb((unsigned char *)rgba_->data(), width_*height_*4);
        } else {
            rgb_data = (unsigned char *)rgba_->data();
        }

        JSAMPROW row_pointer[1];
        while (cinfo.next_scanline < cinfo.image_height) {
            row_pointer[0] = &rgb_data[cinfo.next_scanline*3*width_];
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        jpeg_destroy_compress(&cinfo);

        free(mem_dest);
        if (buf_type_ == BUF_RGBA) free(rgb_data);

        Local<Value> args[2] = {
            Encode((char *)mem_dest, outsize, BINARY),
            Integer::New(outsize)
        };
        Emit(data_symbol, 2, args);
        Emit(end_symbol, 0, NULL);
    }

protected:
    static Handle<Value>
    New(const Arguments& args)
    {
        HandleScope scope;

        if (args.Length() != 5)
            ThrowException(Exception::Error(String::New("Five arguments required - rgba/rgb buffer, width, height, quality, buffer type")));
        if (!Buffer::HasInstance(args[0]))
            ThrowException(Exception::Error(String::New("First argument must be Buffer.")));
        if (!args[1]->IsInt32())
            ThrowException(Exception::Error(String::New("Second argument must be integer width.")));
        if (!args[2]->IsInt32())
            ThrowException(Exception::Error(String::New("Third argument must be integer height.")));
        if (!args[3]->IsInt32())
            ThrowException(Exception::Error(String::New("Fourth argument must be integer quality.")));
        int quality = args[3]->Int32Value();
        if (quality < 0 || quality > 100)
            ThrowException(Exception::Error(String::New("Quality must be between 0 and 100 (inclusive).")));
        if (!args[4]->IsString())
            ThrowException(Exception::Error(String::New("Fifth argument must be a string. Either 'rgba' or 'rgb'.")));
        String::AsciiValue bt(args[4]->ToString());
        if (!(strcmp(*bt, "rgb") == 0 || strcmp(*bt, "rgba") == 0))
            ThrowException(Exception::Error(String::New("Fifth argument must be either 'rgba' or 'rgb'.")));
        buf_type_t buf_type;
        if (strcmp(*bt, "rgb") == 0) {
            buf_type = BUF_RGB;
        }
        else {
            buf_type = BUF_RGBA;
        }

        Buffer *rgba = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
        Jpeg *jpeg = new Jpeg(rgba, args[1]->Int32Value(), args[2]->Int32Value(),
            quality, buf_type);
        jpeg->Wrap(args.This());
        return args.This();
    }

    static Handle<Value>
    JpegEncode(const Arguments& args)
    {
        HandleScope scope;
        Jpeg *jpeg = ObjectWrap::Unwrap<Jpeg>(args.This());
        jpeg->JpegEncode();
        return Undefined();
    }
};

extern "C" void
init(Handle<Object> target)
{
    HandleScope scope;
    Jpeg::Initialize(target);
}

