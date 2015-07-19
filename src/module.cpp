#include <nan.h>
#include <node.h>

#include "jpeg.h"
#include "fixed_jpeg_stack.h"
#include "dynamic_jpeg_stack.h"

void InitAll(Handle<Object> target)
{
    Jpeg::Initialize(target);
    FixedJpegStack::Initialize(target);
    DynamicJpegStack::Initialize(target);
}

NODE_MODULE(jpeg, InitAll)
