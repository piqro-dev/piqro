#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_DECORATE(name) name

#include <third_party/stb_sprintf.h>

extern "C" void js_console_log(const char*);

extern "C" void js_console_error(const char*);

extern "C" double atof(const char*);

extern "C" float fmodf(float, float);