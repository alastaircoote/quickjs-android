#ifndef QUICKJS_ANDROID_JAVA_HELPER_H
#define QUICKJS_ANDROID_JAVA_HELPER_H

#include <jni.h>

#define CLASS_NAME_ILLEGAL_STATE_EXCEPTION "java/lang/IllegalStateException"
#define CLASS_NAME_JS_DATA_EXCEPTION "com/hippo/quickjs/android/JSDataException"

#define THROW_EXCEPTION(ENV, EXCEPTION_NAME, ...)                               \
    do {                                                                        \
        throw_exception((ENV), (EXCEPTION_NAME), __VA_ARGS__);                  \
        return;                                                               \
    } while (0)

#define THROW_EXCEPTION_RET(ENV, EXCEPTION_NAME, ...)                           \
    do {                                                                        \
        throw_exception((ENV), (EXCEPTION_NAME), __VA_ARGS__);                  \
        return 0;                                                               \
    } while (0)

#define THROW_ILLEGAL_STATE_EXCEPTION(ENV, ...)                                 \
    THROW_EXCEPTION(ENV, CLASS_NAME_ILLEGAL_STATE_EXCEPTION, __VA_ARGS__)

#define THROW_ILLEGAL_STATE_EXCEPTION_RET(ENV, ...)                             \
    THROW_EXCEPTION_RET(ENV, CLASS_NAME_ILLEGAL_STATE_EXCEPTION, __VA_ARGS__)

#define THROW_JS_DATA_EXCEPTION(ENV, ...)                                       \
    THROW_EXCEPTION(ENV, CLASS_NAME_JS_DATA_EXCEPTION, __VA_ARGS__)

#define THROW_JS_DATA_EXCEPTION_RET(ENV, ...)                                   \
    THROW_EXCEPTION_RET(ENV, CLASS_NAME_JS_DATA_EXCEPTION, __VA_ARGS__)

#define CHECK_NULL(ENV, POINTER, MESSAGE)                                       \
    if ((POINTER) == NULL) {                                                    \
        THROW_ILLEGAL_STATE_EXCEPTION((ENV), (MESSAGE));                        \
    }

#define CHECK_NULL_RET(ENV, POINTER, MESSAGE)                                   \
    if ((POINTER) == NULL) {                                                    \
        THROW_ILLEGAL_STATE_EXCEPTION_RET((ENV), (MESSAGE));                    \
    }

jint throw_exception(JNIEnv *env, const char *exception_name, const char *message, ...);

#endif //QUICKJS_ANDROID_JAVA_HELPER_H
