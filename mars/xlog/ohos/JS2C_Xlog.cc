#include "napi/native_api.h"
#include "hilog/log.h"
#include "xlogger.h"
#include "xlogger_interface.h"

#define LOG_DOMAIN 0x0201
#define LOG_TAG "XLOG"
#define LONGTHREADID2INT(a) ((a >> 32) ^ ((a & 0xFFFF)))

const int32_t STR_DEFAULT_SIZE = 2048;
const int32_t MAX_STR_LENGTH = 2048;

static void JsValueToString(const napi_env & env, const napi_value & value, const int32_t bufLen, std::string & target)
{
    if (bufLen <= 0 || bufLen > MAX_STR_LENGTH) {
        return;
    }

    std::unique_ptr < char[] > buf = std::make_unique < char[] >(bufLen);
    if (buf.get() == nullptr) {
        return;
    }
    (void)memset(buf.get(), 0, bufLen);
    size_t result = 0;
    napi_get_value_string_utf8(env, value, buf.get(), bufLen, &result);
    target = buf.get();
}

// export const logWrite2: (logInstancePtr: number, level: number, tag: string, filename: string, funcName: string, line: number, pid: number, tid: number, mainTid: number, log: string) => void;
static napi_value logWrite2(napi_env env, napi_callback_info info) {
    size_t argc = 10;
    napi_value args[10] = {nullptr};
    
    // OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, "logWriter2");
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    
    int level;
    napi_get_value_int32(env, args[1], &level);
    
    std::string tag;
    JsValueToString(env, args[2], STR_DEFAULT_SIZE, tag);
    
    std::string filename;
    JsValueToString(env, args[3], STR_DEFAULT_SIZE, filename);
    
    std::string funcname;
    JsValueToString(env, args[4], STR_DEFAULT_SIZE, funcname);

     int line;
    napi_get_value_int32(env, args[5], &line);

    int64_t pid;
    napi_get_value_int64(env, args[6], &pid);

    int64_t tid;
    napi_get_value_int64(env, args[7], &tid);

    int maintid;
    napi_get_value_int32(env, args[8], &maintid);

    std::string log;
    JsValueToString(env, args[9], STR_DEFAULT_SIZE, log);
    
    XLoggerInfo xlog_info = XLOGGER_INFO_INITIALIZER;
    gettimeofday(&xlog_info.timeval, NULL);
    xlog_info.level = (TLogLevel)level;
    xlog_info.line = (int)line;
    xlog_info.pid = (int)pid;
    xlog_info.tid = LONGTHREADID2INT(tid);
    xlog_info.maintid = LONGTHREADID2INT(maintid);
    xlog_info.tag = NULL == tag.c_str() ? "" : tag.c_str();
    xlog_info.filename = NULL == filename.c_str() ? "" : filename.c_str();
    xlog_info.func_name = NULL == funcname.c_str() ? "" : funcname.c_str();
    // OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, "logWriter2 11111");
    
    mars::xlog::XloggerWrite(_log_instance_ptr, &xlog_info, NULL == log.c_str() ? "NULL == log" : log.c_str());
    // OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, "logWriter2 22222");
    return nullptr;
}

// export const getLogLevel: (logInstancePtr: number) => number;
static napi_value getLogLevel(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    
    napi_value result;
    napi_create_int32(env, mars::xlog::GetLevel(_log_instance_ptr), &result);
    return result;
}

// export const setLogLevel: (logInstancePtr: number, level: number) => void;
static napi_value setLogLevel(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    int _level;
    napi_get_value_int32(env, args[1], &_level);
    
    mars::xlog::SetLevel(_log_instance_ptr, (TLogLevel)_level);
    
    return nullptr;
}

// export const setAppenderMode: (logInstancePtr: number, mode: number) => void;
static napi_value setAppenderMode(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    int mode;
    napi_get_value_int32(env, args[1], &mode);

    mars::xlog::SetAppenderMode(_log_instance_ptr, (mars::xlog::TAppenderMode)mode);
    
    return nullptr;
}

// export const newXlogInstance: (xlogConfig: Object) => number;
static napi_value newXlogInstance(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args;
    napi_get_cb_info(env, info, &argc, &args, nullptr, nullptr);
    
    napi_value levelNapiValue;
    napi_get_named_property(env, args, "level", &levelNapiValue);
    int level;
    napi_get_value_int32(env, levelNapiValue, &level);
    
    napi_value modeNapiValue;
    napi_get_named_property(env, args, "mode", &modeNapiValue);
    int mode;
    napi_get_value_int32(env, modeNapiValue, &mode);

    napi_value logDirNapiValue;
    napi_get_named_property(env, args, "logDir", &logDirNapiValue);
    std::string logDir;
    JsValueToString(env, logDirNapiValue, STR_DEFAULT_SIZE, logDir);
    
    napi_value namePrefixNapiValue;
    napi_get_named_property(env, args, "namePrefix", &namePrefixNapiValue);
    std::string namePrefix;
    JsValueToString(env, namePrefixNapiValue, STR_DEFAULT_SIZE, namePrefix);
    
    napi_value pubKeyNapiValue;
    napi_get_named_property(env, args, "pubKey", &pubKeyNapiValue);
    std::string pubKey;
    JsValueToString(env, pubKeyNapiValue, STR_DEFAULT_SIZE, pubKey);
    
    napi_value compressModeNapiValue;
    napi_get_named_property(env, args, "compressMode", &compressModeNapiValue);
    int compressMode;
    napi_get_value_int32(env, compressModeNapiValue, &compressMode);
    
    napi_value compressLevelNapiValue;
    napi_get_named_property(env, args, "compressLevel", &compressLevelNapiValue);
    int compressLevel;
    napi_get_value_int32(env, compressLevelNapiValue, &compressLevel);
    
    napi_value cacheDirNapiValue;
    napi_get_named_property(env, args, "cacheDir", &cacheDirNapiValue);
    std::string cacheDir;
    JsValueToString(env, cacheDirNapiValue, STR_DEFAULT_SIZE, cacheDir);
    
    napi_value cacheDaysNapiValue;
    napi_get_named_property(env, args, "cacheDays", &levelNapiValue);
    int cacheDays;
    napi_get_value_int32(env, cacheDaysNapiValue, &cacheDays);
    
    mars::xlog::XLogConfig config = {
        (mars::xlog::TAppenderMode)mode,
        logDir.c_str(), 
        namePrefix.c_str(), 
        pubKey.c_str(),
        (mars::xlog::TCompressMode)compressMode, 
        compressLevel,  
        cacheDir.c_str(),   
        cacheDays
    };
    mars::comm::XloggerCategory *category = mars::xlog::NewXloggerInstance(config, (TLogLevel)level);
    if (nullptr == category) {
        napi_value result;
        napi_create_int32(env, 0, &result);
        return result;
    }
    
    napi_value categoryPointer;
    napi_create_int64(env, reinterpret_cast<uintptr_t>(category), &categoryPointer);
    return categoryPointer;
}

// export const getXlogInstance: (namePrefix: string) => number;
static napi_value getXlogInstance(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    std::string nameprefix;
    JsValueToString(env, args[0], STR_DEFAULT_SIZE, nameprefix);
    
    mars::comm::XloggerCategory *category = mars::xlog::GetXloggerInstance(nameprefix.c_str());
    if (nullptr == category) {
        napi_value result;
        napi_create_int32(env, 0, &result);
        return result;
    }
    
    napi_value result;
    napi_create_int64(env, reinterpret_cast<uintptr_t>(category), &result);
    return result;
}

// export const releaseXlogInstance: (namePrefix: string) => void;
static napi_value releaseXlogInstance(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    std::string nameprefix;
    JsValueToString(env, args[0], STR_DEFAULT_SIZE, nameprefix);
    
    mars::xlog::ReleaseXloggerInstance(nameprefix.c_str());
    
    return nullptr;
}

// export const setConsoleLogOpen: (logInstancePtr: number, isOpen: boolean) => void;
static napi_value setConsoleLogOpen(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    bool _is_open;
    napi_get_value_bool(env, args[1], &_is_open);

    mars::xlog::SetConsoleLogOpen(_log_instance_ptr, _is_open);

    return nullptr;
}

// export const appenderOpen: (xlogConfig: Object) => number;
static napi_value appenderOpen(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args;
    // OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, "appenderOpen 1...");
    
    napi_get_cb_info(env, info, &argc, &args, nullptr, nullptr);
    
    napi_value levelNapiValue;
    napi_get_named_property(env, args, "level", &levelNapiValue);
    int level;
    napi_get_value_int32(env, levelNapiValue, &level);
    
    napi_value modeNapiValue;
    napi_get_named_property(env, args, "mode", &modeNapiValue);
    int mode;
    napi_get_value_int32(env, modeNapiValue, &mode);

    napi_value logDirNapiValue;
    napi_get_named_property(env, args, "logDir", &logDirNapiValue);
    std::string logDir;
    JsValueToString(env, logDirNapiValue, STR_DEFAULT_SIZE, logDir);
    
    napi_value namePrefixNapiValue;
    napi_get_named_property(env, args, "namePrefix", &namePrefixNapiValue);
    std::string namePrefix;
    JsValueToString(env, namePrefixNapiValue, STR_DEFAULT_SIZE, namePrefix);
    
    napi_value pubKeyNapiValue;
    napi_get_named_property(env, args, "pubKey", &pubKeyNapiValue);
    std::string pubKey;
    JsValueToString(env, pubKeyNapiValue, STR_DEFAULT_SIZE, pubKey);
    
    napi_value compressModeNapiValue;
    napi_get_named_property(env, args, "compressMode", &compressModeNapiValue);
    int compressMode;
    napi_get_value_int32(env, compressModeNapiValue, &compressMode);
    
    napi_value compressLevelNapiValue;
    napi_get_named_property(env, args, "compressLevel", &compressLevelNapiValue);
    int compressLevel;
    napi_get_value_int32(env, compressLevelNapiValue, &compressLevel);
    
    napi_value cacheDirNapiValue;
    napi_get_named_property(env, args, "cacheDir", &cacheDirNapiValue);
    std::string cacheDir;
    JsValueToString(env, cacheDirNapiValue, STR_DEFAULT_SIZE, cacheDir);
    
    napi_value cacheDaysNapiValue;
    napi_get_named_property(env, args, "cacheDays", &levelNapiValue);
    int cacheDays;
    napi_get_value_int32(env, cacheDaysNapiValue, &cacheDays);
    
    // OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, "appenderOpen 2...");
    mars::xlog::XLogConfig config = {
        (mars::xlog::TAppenderMode)mode, 
        logDir.c_str(),
        namePrefix.c_str(),
        pubKey.c_str(),
        (mars::xlog::TCompressMode)compressMode, 
        compressLevel, 
        cacheDir.c_str(),   
        cacheDays
    };
    appender_open(config);
    // OH_LOG_Print(LOG_APP, LOG_DEBUG, LOG_DOMAIN, LOG_TAG, "appenderOpen 3...");
    xlogger_SetLevel((TLogLevel)level);
    
    mars::comm::XloggerCategory *category = mars::xlog::GetXloggerInstance(namePrefix.c_str());
    if (nullptr == category) {
        napi_value result;
        napi_create_int32(env, 0, &result);
        return result;
    }
    
    napi_value result;
    napi_create_int64(env, reinterpret_cast<uintptr_t>(category), &result);
    return result;
}

// export const appenderClose: () => void;
static napi_value appenderClose(napi_env env, napi_callback_info info) {
    mars::xlog::appender_close(); 
    
    return nullptr;
}

// export const appenderFlush: (logInstancePtr: number, isSync: boolean) => void;
static napi_value appenderFlush(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    bool _is_sync;
    napi_get_value_bool(env, args[1], &_is_sync);
    
    mars::xlog::Flush(_log_instance_ptr, _is_sync);
    
    return nullptr;
}

// export const setMaxFileSize: (logInstancePtr: number, maxSize: number) => void;
static napi_value setMaxFileSize(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    int _max_size;
    napi_get_value_int32(env, args[1], &_max_size);

    mars::xlog::SetMaxFileSize(_log_instance_ptr, _max_size);
    
    return nullptr;
}

// export const setMaxAliveTime: (logInstancePtr: number, maxTime: number) => void;
static napi_value setMaxAliveTime(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t _log_instance_ptr;
    napi_get_value_int64(env, args[0], &_log_instance_ptr);
    int _max_time;
    napi_get_value_int32(env, args[1], &_max_time);
    
    mars::xlog::SetMaxAliveTime(_log_instance_ptr, _max_time);
    
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "logWrite2", nullptr, logWrite2, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getLogLevel", nullptr, getLogLevel, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setLogLevel", nullptr, setLogLevel, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setAppenderMode", nullptr, setAppenderMode, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "newXlogInstance", nullptr, newXlogInstance, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "getXlogInstance", nullptr, getXlogInstance, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "releaseXlogInstance", nullptr, releaseXlogInstance, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setConsoleLogOpen", nullptr, setConsoleLogOpen, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "appenderOpen", nullptr, appenderOpen, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "appenderClose", nullptr, appenderClose, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "appenderFlush", nullptr, appenderFlush, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setMaxFileSize", nullptr, setMaxFileSize, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "setMaxAliveTime", nullptr, setMaxAliveTime, nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "marsxlog",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterMarsxlogModule(void)
{
    napi_module_register(&demoModule);
}

void ExportXlog() {
}