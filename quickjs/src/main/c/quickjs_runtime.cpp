#include <quickjs.h>

#include <stdio.h>
#include <stdlib.h>

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int32_t new_native_add(int32_t x, int32_t y) {
    return x * y;
}


extern "C" __attribute__((visibility("default"))) __attribute__((used))
char *doSomeStringOperation() {
    //char const *p = "String vinda do C";
    char* p = (char*)"abc";
    return p;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
JSRuntime *JS_NewRuntimeDartBridge(void) {
    return JS_NewRuntime();
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
JSContext *JS_NewContextDartBridge(JSRuntime *rt) {
    JSContext *ctx;
    ctx = (JSContext*)JS_NewContext(rt);
    return ctx;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
char *JS_ToCStringDartBridge(JSContext *ctx, JSValueConst *val1) {
    char *result = (char*)JS_ToCString(ctx, * val1);
    return result;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
const JSValue *JS_Eval_Wrapper(JSContext *ctx, const char *input, size_t input_len,
                const char *filename, int eval_flags) {
    JSValue result = JS_Eval(ctx, input, input_len, filename, eval_flags);
    if (JS_IsException(result) == 1) {
        result = JS_GetException(ctx);
    }
    return &result;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
const char *JS_Eval_Wrapper2(JSContext *ctx, const char *input, size_t input_len,
                  const char *filename, int eval_flags, int *errors) {
    JSValue result = JS_Eval(ctx, input, input_len, filename, eval_flags);

    *errors = 0;

    if (JS_IsException(result) == 1) {
        *errors = 1;
        result = JS_GetException(ctx);
    }

    return (char*)JS_ToCString(ctx, result);
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
const JSValue *JS_GetGlobalObjectWrapper(JSContext *ctx) {
    JSValue result = JS_GetGlobalObject(ctx);
    return &result;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void registerNativeFunction(JSContext *ctx) {
    JSValue result = JS_GetGlobalObject(ctx);
   // return &result;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int JS_IsErrorDartWrapper(JSContext *ctx, JSValueConst *value) {
    JS_BOOL result = JS_IsError(ctx, * value);
    return result;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int JS_IsExceptionDartWrapper(const JSValueConst *value) {
    JS_BOOL result = JS_IsException(* value);
    return result;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
JSValue *JS_GetExceptionDartWrapper(JSContext *ctx) {
    JSValue result = JS_GetException(ctx);
    return &result;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void JS_FreeValueDartWrapper(JSContext *ctx, JSValueConst *value) {
    JS_FreeValue(ctx, * value);
}



// typedef void(*callback_t)(int);        // callback receives int returns void
//typedef void (*channel)(*char, *char);  // callback receives two string arguments and returns void

// function which will receives a channel function which javascript would send data to Dart
//

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int registerChannelFunction(JSContext *ctx, char *channelName, char *functionName, JSCFunction *fn) {
    JSValue globalObject = JS_GetGlobalObject(ctx);
    char* quickJsChannelsPropertyName = (char*)"QUICKJS_TO_DART_CHANNEL_NATIVE";

    JSValue hashChannels = JS_GetPropertyStr(ctx, globalObject, quickJsChannelsPropertyName);
    if (JS_IsUndefined(hashChannels)) {
        hashChannels = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, globalObject, quickJsChannelsPropertyName, hashChannels);
    }

    //char* functionName = (char*)"QUICKJS_CHANNELS_channelName";
    JS_SetPropertyStr(
        ctx,
        hashChannels,
        channelName,
        JS_NewCFunction(ctx, *fn, functionName, 2) // TODO: criar a function C do lado do dart e passar como argumento
    );
    return 1;
}
/*
extern "C" __attribute__((visibility("default"))) __attribute__((used))
JSValue *JS_NewCFunctionDartWrapper(JSContext *ctx, JSCFunction *func, const char *name, int *length) {
    return JS_NewCFunction(ctx, func, name, length);
}*/