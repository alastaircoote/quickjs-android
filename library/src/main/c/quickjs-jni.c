#include <jni.h>
#include <quickjs.h>
#include <string.h>

#include "java-method.h"
#include "java-object.h"
#include "java-helper.h"

#define MSG_OOM "Out of memory"
#define MSG_NULL_JS_RUNTIME "Null JSRuntime"
#define MSG_NULL_JS_CONTEXT "Null JSContext"
#define MSG_NULL_JS_VALUE "Null JSValue"

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createRuntime(JNIEnv *env, jclass clazz) {
    JSRuntime *rt = JS_NewRuntime();
    CHECK_NULL_RET(env, rt, MSG_OOM);
    return (jlong) rt;
}

JNIEXPORT void JNICALL
Java_com_hippo_quickjs_android_QuickJS_setRuntimeMallocLimit(JNIEnv *env, jclass clazz, jlong runtime, jint mallocLimit) {
    JSRuntime *rt = (JSRuntime *) runtime;
    CHECK_NULL(env, rt, MSG_NULL_JS_RUNTIME);
    JS_SetMemoryLimit(rt, (size_t) mallocLimit);
}

#ifdef LEAK_TRIGGER

static int leak_state = 0;

// Redirect printf() to this function
// to get memory leak detection result in JS_FreeRuntime()
// without modifying the source code of QuickJS.
int leak_trigger(const char* _, ...) {
    leak_state = 1;
    return 0;
}

#endif

JNIEXPORT void JNICALL
Java_com_hippo_quickjs_android_QuickJS_destroyRuntime(JNIEnv *env, jclass clazz, jlong runtime) {
    JSRuntime *rt = (JSRuntime *) runtime;
    CHECK_NULL(env, rt, MSG_NULL_JS_RUNTIME)
#ifdef LEAK_TRIGGER
    leak_state = 0;
#endif
    JS_FreeRuntime(rt);
#ifdef LEAK_TRIGGER
    if (leak_state != 0) {
        THROW_ILLEGAL_STATE_EXCEPTION(env, "Memory Leak");
    }
#endif
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createContext(JNIEnv *env, jclass clazz, jlong runtime) {
    JSRuntime *rt = (JSRuntime *) runtime;
    CHECK_NULL_RET(env, rt, MSG_NULL_JS_RUNTIME);
    JSContext *ctx = JS_NewContext(rt);
    CHECK_NULL_RET(env, ctx, MSG_OOM);

    if (java_method_init_context(ctx)) THROW_ILLEGAL_STATE_EXCEPTION_RET(env, MSG_OOM);
    if (java_object_init_context(ctx)) THROW_ILLEGAL_STATE_EXCEPTION_RET(env, MSG_OOM);

    return (jlong) ctx;
}

JNIEXPORT void JNICALL
Java_com_hippo_quickjs_android_QuickJS_destroyContext(JNIEnv *env, jclass clazz, jlong context) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL(env, ctx, MSG_NULL_JS_CONTEXT);
    JS_FreeContext(ctx);
}

#define COPY_JS_VALUE(JS_CONTEXT, JS_VALUE, RESULT)                                    \
    do {                                                                               \
        void *__copy__ = js_malloc_rt(JS_GetRuntime(JS_CONTEXT), sizeof(JSValue));     \
        if (__copy__ != NULL) {                                                        \
            memcpy(__copy__, &(JS_VALUE), sizeof(JSValue));                            \
            (RESULT) = __copy__;                                                       \
        } else {                                                                       \
            JS_FreeValue((JS_CONTEXT), (JS_VALUE));                                    \
        }                                                                              \
    } while (0)

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueUndefined(JNIEnv *env, jclass clazz, jlong context) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = 0;
    JSValue val = JS_UNDEFINED;
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueNull(JNIEnv *env, jclass clazz, jlong context) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = NULL;
    JSValue val = JS_NULL;
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueBoolean(JNIEnv *env, jclass clazz, jlong context, jboolean value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = NULL;
    JSValue val = JS_NewBool(ctx, value);
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueInt(JNIEnv *env, jclass clazz, jlong context, jint value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = NULL;
    JSValue val = JS_NewInt32(ctx, value);
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueFloat64(JNIEnv *env, jclass clazz, jlong context, jdouble value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = NULL;
    JSValue val = JS_NewFloat64(ctx, value);
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueString(JNIEnv *env, jclass clazz, jlong context, jstring value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    CHECK_NULL_RET(env, value, "Null value");

    const char *value_utf = (*env)->GetStringUTFChars(env, value, NULL);
    CHECK_NULL_RET(env, value_utf, MSG_OOM);

    JSValue *result = NULL;
    JSValue val = JS_NewString(ctx, value_utf);
    COPY_JS_VALUE(ctx, val, result);

    (*env)->ReleaseStringUTFChars(env, value, value_utf);

    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueObject(JNIEnv *env, jclass clazz, jlong context) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = NULL;
    JSValue val = JS_NewObject(ctx);
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueArray(JNIEnv *env, jclass clazz, jlong context) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = NULL;
    JSValue val = JS_NewArray(ctx);
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

static jlong createValueFunction(
        JNIEnv *env,
        jlong context,
        jobject js_context,
        jboolean is_static,
        jobject callee,
        jstring method_name,
        jstring method_sign,
        jobject return_type,
        jobjectArray arg_types
) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    const char *method_name_utf8 = (*env)->GetStringUTFChars(env, method_name, NULL);
    const char *method_sign_utf8 = (*env)->GetStringUTFChars(env, method_sign, NULL);
    if (method_name_utf8 == NULL || method_sign_utf8 == NULL) {
        if (method_name_utf8 != NULL) (*env)->ReleaseStringUTFChars(env, method_name, method_name_utf8);
        if (method_sign_utf8 != NULL) (*env)->ReleaseStringUTFChars(env, method_sign, method_sign_utf8);
        THROW_ILLEGAL_STATE_EXCEPTION_RET(env, MSG_OOM);
    }

    jmethodID method = NULL;
    if (is_static) {
        method = (*env)->GetStaticMethodID(env, callee, method_name_utf8, method_sign_utf8);
    } else {
        jclass callee_class = (*env)->GetObjectClass(env, callee);
        method = (*env)->GetMethodID(env, callee_class, method_name_utf8, method_sign_utf8);
    }
    (*env)->ReleaseStringUTFChars(env, method_name, method_name_utf8);
    (*env)->ReleaseStringUTFChars(env, method_sign, method_sign_utf8);
    if (method == NULL) {
        if ((*env)->ExceptionCheck(env)) return 0;
        THROW_ILLEGAL_STATE_EXCEPTION_RET(env, "Can't find method");
    }

    int arg_count = (*env)->GetArrayLength(env, arg_types);
    jobject arg_types_copy[arg_count];
    for (int i = 0; i < arg_count; i++) {
        arg_types_copy[i] = (*env)->GetObjectArrayElement(env, arg_types, i);
    }

    JSValue *result = NULL;
    JSValue val = QJ_NewJavaMethod(ctx, env, js_context, is_static, callee, method, return_type, arg_count, arg_types_copy);
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueFunction(
        JNIEnv *env,
        jclass clazz,
        jlong context,
        jobject js_context,
        jobject instance,
        jstring method_name,
        jstring method_sign,
        jobject return_type,
        jobjectArray arg_types
) {
    return createValueFunction(env, context, js_context, JNI_FALSE, instance, method_name, method_sign, return_type, arg_types);
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueFunctionS(
        JNIEnv *env,
        jclass clazz,
        jlong context,
        jobject js_context,
        jstring class_name,
        jstring method_name,
        jstring method_sign,
        jobject return_type,
        jobjectArray arg_types
) {
    const char *class_name_utf8 = (*env)->GetStringUTFChars(env, class_name, NULL);
    CHECK_NULL_RET(env, class_name_utf8, MSG_OOM);
    jclass callee = (*env)->FindClass(env, class_name_utf8);
    (*env)->ReleaseStringUTFChars(env, class_name, class_name_utf8);
    if (callee == NULL) {
        if ((*env)->ExceptionCheck(env)) return 0;
        THROW_ILLEGAL_STATE_EXCEPTION_RET(env, "Can't find class");
    }
    return createValueFunction(env, context, js_context, JNI_TRUE, callee, method_name, method_sign, return_type, arg_types);
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_createValueJavaObject(JNIEnv *env, jclass clazz, jlong context, jobject object) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = NULL;
    JSValue val = QJ_NewJavaObject(ctx, env, object);
    COPY_JS_VALUE(ctx, val, result);
    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jboolean JNICALL
Java_com_hippo_quickjs_android_QuickJS_defineValueProperty__JJIJI(
        JNIEnv *env,
        jclass clazz,
        jlong context,
        jlong value,
        jint index,
        jlong property,
        jint flags
) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    JSValue *prop = (JSValue *) property;
    CHECK_NULL_RET(env, prop, "Null property");

    JS_DupValue(ctx, *prop);

    return (jboolean) (JS_DefinePropertyValueUint32(ctx, *val, (uint32_t) index, *prop, flags) >= 0);
}

JNIEXPORT jboolean JNICALL
Java_com_hippo_quickjs_android_QuickJS_defineValueProperty__JJLjava_lang_String_2JI(
        JNIEnv *env,
        jclass clazz,
        jlong context,
        jlong value,
        jstring name,
        jlong property,
        jint flags
) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    JSValue *prop = (JSValue *) property;
    CHECK_NULL_RET(env, prop, "Null property");

    const char *name_utf = (*env)->GetStringUTFChars(env, name, NULL);
    CHECK_NULL_RET(env, name_utf, MSG_OOM);

    JS_DupValue(ctx, *prop);

    jboolean result = (jboolean) (JS_DefinePropertyValueStr(ctx, *val, name_utf, *prop, flags) >= 0);

    (*env)->ReleaseStringUTFChars(env, name, name_utf);

    return result;
}

JNIEXPORT jint JNICALL
Java_com_hippo_quickjs_android_QuickJS_getValueTag(JNIEnv *env, jclass clazz, jlong value) {
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    return JS_VALUE_GET_NORM_TAG(*val);
}

JNIEXPORT jboolean JNICALL
Java_com_hippo_quickjs_android_QuickJS_isValueArray(JNIEnv *env, jclass clazz, jlong context, jlong value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    return (jboolean) JS_IsArray(ctx, *val);
}

JNIEXPORT jboolean JNICALL
Java_com_hippo_quickjs_android_QuickJS_isValueFunction(JNIEnv *env, jclass clazz, jlong context, jlong value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    return (jboolean) JS_IsFunction(ctx, *val);
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_invokeValueFunction(JNIEnv *env, jclass clazz,
        jlong context, jlong function, jlong thisObj, jlongArray args) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *func_obj = (JSValue *) function;
    CHECK_NULL_RET(env, func_obj, "Null function");
    JSValue *this_obj = (JSValue *) thisObj;
    CHECK_NULL_RET(env, args, "Null arguments")
    jlong *elements = (*env)->GetLongArrayElements(env, args, NULL);
    CHECK_NULL_RET(env, elements, MSG_OOM);

    int argc = (*env)->GetArrayLength(env, args);
    JSValueConst argv[argc];
    for (int i = 0; i < argc; i++) {
        argv[i] = *((JSValue *) elements[i]);
    }

    JSValue *result = NULL;

    JSValue ret = JS_Call(ctx, *func_obj, this_obj != NULL ? *this_obj : JS_UNDEFINED, argc, argv);

    COPY_JS_VALUE(ctx, ret, result);

    (*env)->ReleaseLongArrayElements(env, args, elements, JNI_ABORT);

    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_getValueProperty__JJI(JNIEnv *env, jclass clazz, jlong context, jlong value, jint index) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);

    JSValue *result = NULL;

    JSValue prop = JS_GetPropertyUint32(ctx, *val, (uint32_t) index);

    COPY_JS_VALUE(ctx, prop, result);

    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_getValueProperty__JJLjava_lang_String_2(JNIEnv *env, jclass clazz, jlong context, jlong value, jstring name) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    CHECK_NULL_RET(env, name, "Null name");

    const char *name_utf = (*env)->GetStringUTFChars(env, name, NULL);
    CHECK_NULL_RET(env, name_utf, MSG_OOM);

    JSValue *result = NULL;

    JSValue prop = JS_GetPropertyStr(ctx, *val, name_utf);

    COPY_JS_VALUE(ctx, prop, result);

    (*env)->ReleaseStringUTFChars(env, name, name_utf);

    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jboolean JNICALL
Java_com_hippo_quickjs_android_QuickJS_setValueProperty__JJIJ(JNIEnv *env, jclass clazz, jlong context, jlong value, jint index, jlong property) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    JSValue *prop = (JSValue *) property;
    CHECK_NULL_RET(env, prop, "Null property");

    // JS_SetPropertyUint32 requires a reference count of the property JSValue
    // Meanwhile, it calls JS_FreeValue on the property JSValue if it fails
    JS_DupValue(ctx, *prop);

    return (jboolean) (JS_SetPropertyUint32(ctx, *val, (uint32_t) index, *prop) >= 0);
}

JNIEXPORT jboolean JNICALL
Java_com_hippo_quickjs_android_QuickJS_setValueProperty__JJLjava_lang_String_2J(JNIEnv *env, jclass clazz, jlong context, jlong value, jstring name, jlong property) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    CHECK_NULL_RET(env, name, "Null name");
    JSValue *prop = (JSValue *) property;
    CHECK_NULL_RET(env, prop, "Null property");

    const char *name_utf = (*env)->GetStringUTFChars(env, name, NULL);
    CHECK_NULL_RET(env, name_utf, MSG_OOM);

    // JS_SetPropertyStr requires a reference count of the property JSValue
    // Meanwhile, it calls JS_FreeValue on the property JSValue if it fails
    JS_DupValue(ctx, *prop);

    jboolean result = (jboolean) (JS_SetPropertyStr(ctx, *val, name_utf, *prop) >= 0);

    (*env)->ReleaseStringUTFChars(env, name, name_utf);

    return result;
}

#define CHECK_JS_TAG_RET(VAL, TARGET, TYPE)                                                    \
    int32_t __tag__ = JS_VALUE_GET_NORM_TAG(VAL);                                              \
    if (__tag__ != (TARGET)) {                                                                 \
        THROW_JS_DATA_EXCEPTION_RET(env, "Invalid JSValue tag for %s: %d", (TYPE), __tag__);   \
    }

JNIEXPORT jboolean JNICALL
Java_com_hippo_quickjs_android_QuickJS_getValueBoolean(JNIEnv *env, jclass clazz, jlong value) {
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    CHECK_JS_TAG_RET(*val, JS_TAG_BOOL, "boolean");
    return (jboolean) (JS_VALUE_GET_BOOL(*val));
}

JNIEXPORT jint JNICALL
Java_com_hippo_quickjs_android_QuickJS_getValueInt(JNIEnv *env, jclass clazz, jlong value) {
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    CHECK_JS_TAG_RET(*val, JS_TAG_INT, "int");
    return (jint) (JS_VALUE_GET_INT(*val));
}

JNIEXPORT jdouble JNICALL
Java_com_hippo_quickjs_android_QuickJS_getValueFloat64(JNIEnv *env, jclass clazz, jlong value) {
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    CHECK_JS_TAG_RET(*val, JS_TAG_FLOAT64, "float64");
    return (jdouble) JS_VALUE_GET_FLOAT64(*val);
}

JNIEXPORT jstring JNICALL
Java_com_hippo_quickjs_android_QuickJS_getValueString(JNIEnv *env, jclass clazz, jlong context, jlong value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    CHECK_JS_TAG_RET(*val, JS_TAG_STRING, "string");

    const char *str = JS_ToCString(ctx, *val);
    CHECK_NULL_RET(env, str, MSG_OOM);

    jstring j_str = (*env)->NewStringUTF(env, str);

    JS_FreeCString(ctx, str);

    CHECK_NULL_RET(env, j_str, MSG_OOM);

    return j_str;
}

JNIEXPORT jobject JNICALL
Java_com_hippo_quickjs_android_QuickJS_getValueJavaObject(JNIEnv *env, jclass clazz, jlong context, jlong value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL_RET(env, val, MSG_NULL_JS_VALUE);
    return QJ_GetJavaObject(ctx, *val);
}

JNIEXPORT void JNICALL
Java_com_hippo_quickjs_android_QuickJS_destroyValue(JNIEnv *env, jclass clazz, jlong context, jlong value) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL(env, ctx, MSG_NULL_JS_CONTEXT);
    JSValue *val = (JSValue *) value;
    CHECK_NULL(env, val, MSG_NULL_JS_VALUE);
    JS_FreeValue(ctx, *val);
    js_free_rt(JS_GetRuntime(ctx), val);
}

JNIEXPORT jobject JNICALL
Java_com_hippo_quickjs_android_QuickJS_getException(JNIEnv *env, jclass clazz, jlong context) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    jclass js_exception_class = (*env)->FindClass(env, "com/hippo/quickjs/android/JSException");
    CHECK_NULL_RET(env, js_exception_class, "Can't find JSException");

    jmethodID constructor_id = (*env)->GetMethodID(env, js_exception_class, "<init>", "(ZLjava/lang/String;Ljava/lang/String;)V");
    CHECK_NULL_RET(env, constructor_id, "Can't find JSException constructor");

    const char *exception_str = NULL;
    const char *stack_str = NULL;

    JSValue exception = JS_GetException(ctx);
    exception_str = JS_ToCString(ctx, exception);
    jboolean is_error = (jboolean) JS_IsError(ctx, exception);
    if (is_error) {
        JSValue stack = JS_GetPropertyStr(ctx, exception, "stack");
        if (!JS_IsUndefined(stack)) {
            stack_str = JS_ToCString(ctx, stack);
        }
        JS_FreeValue(ctx, stack);
    }
    JS_FreeValue(ctx, exception);

    jstring exception_j_str = (exception_str != NULL) ? (*env)->NewStringUTF(env, exception_str) : NULL;
    jstring stack_j_str = (stack_str != NULL) ? (*env)->NewStringUTF(env, stack_str) : NULL;

    if (exception_str != NULL) {
        JS_FreeCString(ctx, exception_str);
    }
    if (stack_str != NULL) {
        JS_FreeCString(ctx, stack_str);
    }

    jobject result = (*env)->NewObject(env, js_exception_class, constructor_id, is_error, exception_j_str, stack_j_str);
    CHECK_NULL_RET(env, result, "Can't create instance of JSException");

    return result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_getGlobalObject(JNIEnv *env, jclass clazz, jlong context) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);

    JSValue *result = NULL;

    JSValue val = JS_GetGlobalObject(ctx);
    COPY_JS_VALUE(ctx, val, result);

    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jlong JNICALL
Java_com_hippo_quickjs_android_QuickJS_evaluate(JNIEnv *env, jclass clazz,
        jlong context, jstring source_code, jstring file_name, jint flags) {
    JSContext *ctx = (JSContext *) context;
    CHECK_NULL_RET(env, ctx, MSG_NULL_JS_CONTEXT);
    CHECK_NULL_RET(env, source_code, "Null source code");
    CHECK_NULL_RET(env, file_name, "Null file name");

    const char *source_code_utf = NULL;
    jsize source_code_length = 0;
    const char *file_name_utf = NULL;
    JSValue *result = NULL;

    source_code_utf = (*env)->GetStringUTFChars(env, source_code, NULL);
    source_code_length = (*env)->GetStringUTFLength(env, source_code);
    file_name_utf = (*env)->GetStringUTFChars(env, file_name, NULL);

    if (source_code_utf != NULL && file_name_utf != NULL) {
        JSValue val = JS_Eval(ctx, source_code_utf, (size_t) source_code_length, file_name_utf, flags);

        COPY_JS_VALUE(ctx, val, result);
    }

    if (source_code_utf != NULL) {
        (*env)->ReleaseStringUTFChars(env, source_code, source_code_utf);
    }
    if (file_name_utf != NULL) {
        (*env)->ReleaseStringUTFChars(env, file_name, file_name_utf);
    }

    CHECK_NULL_RET(env, result, MSG_OOM);

    return (jlong) result;
}

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void* reserved) {
    JNIEnv *env = NULL;

    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    if (java_method_init(env)) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}
