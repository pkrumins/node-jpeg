#include <node.h>

#include "jpeg.h"
#include "fixed_jpeg_stack.h"
#include "dynamic_jpeg_stack.h"

using namespace v8;

extern "C" void
init(v8::Handle<v8::Object> target)
{
    v8::HandleScope scope;
    Jpeg::Initialize(target);
    FixedJpegStack::Initialize(target);
    DynamicJpegStack::Initialize(target);
}

