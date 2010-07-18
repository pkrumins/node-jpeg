#include <node.h>
#include "jpeg.h"

using namespace v8;

extern "C" void
init(Handle<Object> target)
{
    HandleScope scope;
    Jpeg::Initialize(target);
}

