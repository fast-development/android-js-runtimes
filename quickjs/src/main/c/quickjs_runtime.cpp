#include <quickjs.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

int QUICKJS_RUNTIME_DEBUG_ENABLED = 0;


extern "C" __attribute__((visibility("default"))) __attribute__((used))
JSRuntime *JS_NewRuntimeDartBridge(void) {
    return JS_NewRuntime();
}

#define QUICKJS_CHANNEL_CONSOLELOG 0;
#define QUICKJS_CHANNEL_SETTIMEOUT 1;
#define QUICKJS_CHANNEL_SENDNATIVE 2;

typedef JSValue *(*ChannelFunc)(const JSContext *ctx, const char *channel, const char *message);
struct channel { 
    char *name;
    JSContext *ctx;
    ChannelFunc func;
    int assigned;
}; 
    
struct channel channel_functions[10] = { /*{"cat", cat_func}, {"dog", dog_func}, {NULL, NULL}*/ 
    { NULL, NULL, 0 },
    { NULL, NULL, 0 },
    { NULL, NULL, 0 },
    { NULL, NULL, 0 },
    { NULL, NULL, 0 },
    { NULL, NULL, 0 },
    { NULL, NULL, 0 },
    { NULL, NULL, 0 },
    { NULL, NULL, 0 },
    { NULL, NULL, 0 }
};

// where cat_func is declared int cat_func(const char **args);.
// You can search the list with

int contextsLength =0;

static JSValue CChannelFunction(JSContext *ctx, JSValueConst  this_val,
                              int argc, JSValueConst *argv) {
    
    const char* channelName = JS_ToCString(ctx, argv[0]);
    const char* message = JS_ToCString(ctx, argv[1]);   

    struct channel *cur = channel_functions;

    JSValue jsResult = JS_NULL;
    char* result = nullptr;
    // while(cur->ctx) {
    //     //if(!strcmp(cur->name, channelName)) {
    //     if (cur->ctx == ctx && cur->assigned == 1) {
    //         JS_Eval(ctx, "console.log('Aqui no CChannelFunction3');", 40,"arquivo.js",0);
    //         result = cur->func(channelName, message);
    //         JS_Eval(ctx, "console.log('Aqui no CChannelFunction4');", 40,"arquivo.js",0);
    //         jsResult = JS_NewString(ctx, result);
    //         JS_Eval(ctx, "console.log('Aqui no CChannelFunction5');", 40,"arquivo.js",0);
    //     }
    // }

    int idxChannel = 0;
    if (strcmp("SendNative",channelName) == 0) {
        idxChannel = QUICKJS_CHANNEL_SENDNATIVE;
    } else if (strcmp("ConsoleLog",channelName) == 0) {
        idxChannel = QUICKJS_CHANNEL_CONSOLELOG;
    } if (strcmp("SetTimeout",channelName) == 0) {
        idxChannel = QUICKJS_CHANNEL_SETTIMEOUT;
    }

    if (channel_functions[idxChannel].assigned == 1) {
       ChannelFunc funcCaller = channel_functions[idxChannel].func;

        if (funcCaller != nullptr) {
            jsResult = * funcCaller(ctx, channelName, message);
            // JS_Eval(ctx, "console.log('Aqui no CChannelFunction5');", 40,"arquivo.js",0);
            // jsResult = JS_NewString(ctx, result);
            //jsResult = JS_NewString(ctx, "No function found");
        } else {
            jsResult = JS_NewString(ctx, "No function found");
        }
    }
    
    // if (result != nullptr) {
    //     JS_Eval(ctx, "console.log('Aqui no CChannelFunction6');", 40,"arquivo.js",0);
    //     free(result);
    //     JS_Eval(ctx, "console.log('Aqui no CChannelFunction7');", 40,"arquivo.js",0);
    // }
    return jsResult;
}

JSValue stringifyFn;

extern "C" __attribute__((visibility("default"))) __attribute__((used))
JSContext *JS_NewContextDartBridge(
    JSRuntime *rt,
    ChannelFunc consoleLogChannelFunction,
    ChannelFunc setTimeoutChannelFunction,
    ChannelFunc sendNativeChannelFunction
) {
    JSContext *ctx;
    ctx = JS_NewContext(rt);
    
     // create the QuickJS Function passing the CChannelFunction ;
    // register the function jsBridgeFunction into the global object
    JSValue globalObject = JS_GetGlobalObject(ctx);


    stringifyFn = JS_Eval(
        ctx,
        "function simpleStringify(obj) { return JSON.stringify(obj);}simpleStringify;", 
        strlen("function simpleStringify(obj) { return JSON.stringify(obj);}"),
        "f1.js",
        0
    );

    JS_SetPropertyStr(
        ctx,
        globalObject,
        "FLUTTER_JS_NATIVE_BRIDGE_sendMessage", 
        JS_NewCFunction(ctx, CChannelFunction, "FLUTTER_JS_NATIVE_BRIDGE_sendMessage", 2)
    );

    if (consoleLogChannelFunction) {
        channel_functions[0].func = consoleLogChannelFunction;
        channel_functions[0].ctx = ctx;
        channel_functions[0].name = (char*)"ConsoleLog";
        channel_functions[0].assigned = 1;

        channel_functions[1].func = setTimeoutChannelFunction;
        channel_functions[1].ctx = ctx;
        channel_functions[1].name = (char*)"SetTimeout";
        channel_functions[1].assigned = 1;
        
        // store in the function register the dartChannelFunction passed
        channel_functions[2].func = sendNativeChannelFunction;
        channel_functions[2].ctx = ctx;
        channel_functions[2].name = (char*)"SendNative";
        channel_functions[2].assigned = 1;

        contextsLength=3;
    }


    // returns the generated context
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
    static JSValue result = JS_Eval(ctx, input, input_len, filename, eval_flags);
    if (JS_IsException(result) == 1) {
        result = JS_GetException(ctx);
    }
    return &result;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
const char *JS_Eval_Wrapper2(JSContext *ctx, const char *input, size_t input_len,
                  const char *filename, int eval_flags,
                  int *errors, JSValueConst *result) {
    *result = JS_Eval(ctx, input, input_len, filename, eval_flags);

    *errors = 0;

    if (JS_IsException(*result) == 1) {
        *errors = 1;
        *result = JS_GetException(ctx);
    }

    return (char*)JS_ToCString(ctx, *result);
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

// used in method callFunction in quickjs_method_bindings
extern "C" __attribute__((visibility("default"))) __attribute__((used))
int callJsFunction1Arg(JSContext *ctx, JSValueConst *function, JSValueConst *object, JSValueConst *result) {
    JSValue globalObject = JS_GetGlobalObject(ctx);
    //JSValue function = JS_GetPropertyStr(ctx, globalObject, functionName);
    * result = JS_Call(ctx, *function, globalObject, 1, object);
    
    int successOperation = 1;

    if (JS_IsException(*result) == 1) {
        successOperation = 0;
        * result = JS_GetException(ctx);
    }
    return successOperation;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
char *JS_ToStringDartWrapper(JSContext *ctx, JSValue *val) {
    JSValue jsvalueResult = JS_ToString(ctx, *val);
    char *result = (char*)JS_ToCString(ctx, jsvalueResult);
    if (QUICKJS_RUNTIME_DEBUG_ENABLED == 1) {
        __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "toString Result: %s", result);
    }
    return result;
}
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

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int registerChannelFunction2(JSContext *ctx, char *channelName, char *functionName, JSCClosure *fn) {
    JSValue globalObject = JS_GetGlobalObject(ctx);
    char* quickJsChannelsPropertyName = (char*)"FLUTTER_JS_NATIVE_CLOSURES";

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
        JS_NewCClosure(ctx, *fn, 3, 0, reinterpret_cast<void*>(*fn), nullptr) // TODO: criar a function C do lado do dart e passar como argumento
    );
    return 1;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
void JS_NewStringDartWrapper(JSContext *ctx, const char *value, JSValue *newString) {
    char *stringValueExpression;
    asprintf(&stringValueExpression, "'%s';", value);
    * newString = JS_Eval(ctx, stringValueExpression, strlen(stringValueExpression), "f1.js", 0);
    free(stringValueExpression);
}


#define MAX_FUNCTION_ARGS            10
struct CallDartProxy {
    JSValueConst args[MAX_FUNCTION_ARGS];
    char *functionName;
};


typedef JSValueConst  *(*callDart)(JSValueConst  *);
callDart callDartFunc;


extern "C" __attribute__((visibility("default"))) __attribute__((used))
JSValue callDartCProxy(JSContext *ctx, JSValueConst  this_val,
                              int argc, JSValueConst *argv)
{
    // int i;
    // const char *str;
    // size_t len;

    // for(i = 0; i < argc; i++) {
    //     if (i != 0)
    //         putchar(' ');
    //     str = JS_ToCStringLen(ctx, &len, argv[i]);
    //     if (!str)
    //         return JS_EXCEPTION;
    //     fwrite(str, 1, len, stdout);
    //     JS_FreeCString(ctx, str);
    // }
    // putchar('\n');

     char* filename = (char*)"LOG.js";
    char* log1 = (char*)"console.log('LOG1');";
    char* log2 = (char*)"console.log('LOG2');";
    char* log3 = (char*)"console.log('LOG3');";
    char* log4 = (char*)"console.log(Object.keys(this));";

    JSValueConst *result;
    JS_Eval(ctx, log1, 19, filename, 0);
    // if (arc != NULL && argc == 0) {
    //     JS_Eval(ctx, log2, 19, filename, 0);
    //    result = callDartFunc((JSValueConst*)nullptr);
    //    JS_Eval(ctx, log3, 19, filename, 0);
    // } else {
    //    result =  callDartFunc(&argv[0]);
    // }
    //return *result;
    // if (argc == 0) {
    //     JS_Eval(ctx, log2, 19, filename, 0);
    // }
    // if (argv == NULL) {
    //     JS_Eval(ctx, log3, 19, filename, 0);
    // }
    JSValueConst *ptr;
    *ptr = JS_UNDEFINED; 
    JS_Eval(ctx, log2, 19, filename, 0);
    callDartFunc(ptr);
    JS_Eval(ctx, log3, 19, filename, 0);
    return JS_UNDEFINED;
}


extern "C" __attribute__((visibility("default"))) __attribute__((used))
int JS_NewCClosureDartWrapper(JSContext *ctx, char *name, JSCClosure *fn, int length, void *opaque, 
  void (*opaque_finalize)(void*), JSValue *closure) {

    JSValue globalObject = JS_GetGlobalObject(ctx);
    char* quickJsChannelsPropertyName = (char*)"FLUTTER_JS_NATIVE_CLOSURES";

    JSValue hashChannels = JS_GetPropertyStr(ctx, globalObject, quickJsChannelsPropertyName);
    if (JS_IsUndefined(hashChannels)) {
        hashChannels = JS_NewObject(ctx);
        JS_SetPropertyStr(ctx, globalObject, quickJsChannelsPropertyName, hashChannels);
    }

    if (QUICKJS_RUNTIME_DEBUG_ENABLED == 1) {
        __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "%p", nullptr);
    }
    JS_SetPropertyStr(
        ctx, hashChannels, "callDart", *closure
    );
  return 1;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int defineGlobalProperty(JSContext *ctx, char *name, JSValue *propertyValue) {
    JSValue globalObject = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(
        ctx,
        globalObject,
        name,
        *propertyValue
    );
    return 1;
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int isType(JSContext *ctx, char *name, JSValue *propertyValue, char *type) {
    JSValue globalObject = JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(
        ctx,
        globalObject,
        name,
        *propertyValue
    );
    return 1;
}


extern "C" __attribute__((visibility("default"))) __attribute__((used))
int getTypeTag(JSValue *jsValue) {
    if (jsValue) {
        return JS_VALUE_GET_TAG(*jsValue);
    } else {
        return JS_TAG_NULL;
    }
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int JS_IsArrayDartWrapper(JSContext *ctx, JSValueConst *val) {
    return JS_IsArray(ctx, *val);
}

extern "C" __attribute__((visibility("default"))) __attribute__((used))
int JS_JSONStringifyDartWrapper(
    JSContext *ctx,
    JSValue *obj, JSValueConst *result) {
    if (QUICKJS_RUNTIME_DEBUG_ENABLED == 1) {
    __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "JS_JSONStringifyDartWrapper %p", result);
    }
    JSValue globalObject = JS_GetGlobalObject(ctx);
    if (QUICKJS_RUNTIME_DEBUG_ENABLED == 1) {
        __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "JS_JSONStringifyDartWrapper2 %p", result);
    }
    if (JS_IsUndefined(*obj)==1) {
        if (QUICKJS_RUNTIME_DEBUG_ENABLED == 1) {
            __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "JS_JSONStringifyDartWrapper3 %p", result);
        }
        return 0;
    } else if ( JS_IsNull(*obj) == 1) {
        if (QUICKJS_RUNTIME_DEBUG_ENABLED == 1) {
            __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "JS_JSONStringifyDartWrapper4 %p", result);
        }
        return 0;
    } else {
        if (QUICKJS_RUNTIME_DEBUG_ENABLED == 1) {
            __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "JS_JSONStringifyDartWrapper5 %p", result);
        }
        * result = JS_Call(ctx, stringifyFn, globalObject, 1, obj);
        if (QUICKJS_RUNTIME_DEBUG_ENABLED == 1) {
            __android_log_print(ANDROID_LOG_DEBUG, "LOG_TAG", "JS_JSONStringifyDartWrapper6 %p", result);
        }
        return 1;
    }
}
