#include "cerrno"
#include "android/log.h"

#ifndef TAG
#define TAG    "Fps_Unlocker"
#endif

#ifdef NDEBUG
#define LOGD(...)
#else
#define LOGD(...)     __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#endif

#define LOGI(...)     __android_log_print(ANDROID_LOG_INFO,  TAG, __VA_ARGS__)
#define LOGW(...) 	  __android_log_print(ANDROID_LOG_WARN,  TAG, __VA_ARGS__)
#define LOGERRNO(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, fmt ": %d (%s)", ##__VA_ARGS__, errno, strerror(errno))
