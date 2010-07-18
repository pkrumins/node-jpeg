#ifndef COMMON_H
#define COMMON_H

#include <node.h>
#include <cstring>

v8::Handle<v8::Value> VException(const char *msg);

bool str_eq(const char *s1, const char *s2);
unsigned char *rgba_to_rgb(const unsigned char *rgba, int rgba_size);
unsigned char *bgra_to_rgb(const unsigned char *rgba, int bgra_size);
unsigned char *bgr_to_rgb(const unsigned char *rgb, int rgb_size);

typedef enum { BUF_RGB, BUF_BGR, BUF_RGBA, BUF_BGRA } buffer_type;

#endif

