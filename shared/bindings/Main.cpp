
#include "../V8Helpers.h"
#include "../V8ResourceImpl.h"
#include "../V8Module.h"

static void HashCb(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();

    V8_CHECK_ARGS_LEN(1);
    V8_ARG_TO_STRING(1, str);

    V8_RETURN_UINT(alt::ICore::Instance().Hash(str));
}

static void On(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN2(1, 2);
    if(info.Length() == 1)
    {
        V8_ARG_TO_FUNCTION(1, callback);

        resource->SubscribeGenericLocal(callback, V8::SourceLocation::GetCurrent(isolate));
    }
    else if(info.Length() == 2)
    {
        V8_ARG_TO_STRING(1, evName);
        V8_ARG_TO_FUNCTION(2, callback);

        resource->SubscribeLocal(evName.ToString(), callback, V8::SourceLocation::GetCurrent(isolate));
    }
}

static void Once(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN2(1, 2);
    if(info.Length() == 1)
    {
        V8_ARG_TO_FUNCTION(1, callback);

        resource->SubscribeGenericLocal(callback, V8::SourceLocation::GetCurrent(isolate), true);
    }
    else if(info.Length() == 2)
    {
        V8_ARG_TO_STRING(1, evName);
        V8_ARG_TO_FUNCTION(2, callback);

        resource->SubscribeLocal(evName.ToString(), callback, V8::SourceLocation::GetCurrent(isolate), true);
    }
}

static void Off(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN2(1, 2);
    if(info.Length() == 1)
    {
        V8_ARG_TO_FUNCTION(1, callback);

        resource->UnsubscribeGenericLocal(callback);
    }
    else if(info.Length() == 2)
    {
        V8_ARG_TO_STRING(1, evName);
        V8_ARG_TO_FUNCTION(2, callback);

        resource->UnsubscribeLocal(evName.ToString(), callback);
    }
}

static void Emit(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN_MIN(1);
    V8_ARG_TO_STRING(1, name);

    alt::MValueArgs args;

    for(int i = 1; i < info.Length(); ++i) args.Push(V8Helpers::V8ToMValue(info[i]));

    alt::ICore::Instance().TriggerLocalEvent(name, args);
}

static void HasMeta(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();

    V8_CHECK_ARGS_LEN_MIN(1);
    V8_ARG_TO_STRING(1, key);

    V8_RETURN(alt::ICore::Instance().HasMetaData(key));
}

static void GetMeta(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();

    V8_CHECK_ARGS_LEN_MIN(1);
    V8_ARG_TO_STRING(1, key);

    V8_RETURN_MVALUE(alt::ICore::Instance().GetMetaData(key));
}

static void SetMeta(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();

    V8_CHECK_ARGS_LEN_MIN(2);
    V8_ARG_TO_STRING(1, key);
    V8_ARG_TO_MVALUE(2, value);

    alt::ICore::Instance().SetMetaData(key, value);
}

static void DeleteMeta(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();

    V8_CHECK_ARGS_LEN_MIN(1);
    V8_ARG_TO_STRING(1, key);

    alt::ICore::Instance().DeleteMetaData(key);
}

static void HasSyncedMeta(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();

    V8_CHECK_ARGS_LEN_MIN(1);
    V8_ARG_TO_STRING(1, key);

    V8_RETURN(alt::ICore::Instance().HasSyncedMetaData(key));
}

static void GetSyncedMeta(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();

    V8_CHECK_ARGS_LEN_MIN(1);
    V8_ARG_TO_STRING(1, key);

    V8_RETURN_MVALUE(alt::ICore::Instance().GetSyncedMetaData(key));
}

static void Log(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();
    V8_CHECK_ARGS_LEN_MIN(1);

    std::stringstream ss;
    for(int i = 0; i < info.Length(); ++i)
    {
        v8::Local<v8::Value> val = info[i];

        if(i > 0) ss << " ";

        auto str = V8::Stringify(ctx, val);
        if(str.IsEmpty()) continue;

        ss << str.CStr();
    }

    alt::ICore::Instance().LogColored(ss.str());
}

static void LogWarning(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();
    V8_CHECK_ARGS_LEN_MIN(1);

    std::stringstream ss;
    for(int i = 0; i < info.Length(); ++i)
    {
        v8::Local<v8::Value> val = info[i];

        if(i > 0) ss << " ";

        auto str = V8::Stringify(ctx, val);
        if(str.IsEmpty()) continue;

        ss << str.CStr();
    }

    alt::ICore::Instance().LogWarning(ss.str());
}

static void LogError(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();
    V8_CHECK_ARGS_LEN_MIN(1);

    std::stringstream ss;
    for(int i = 0; i < info.Length(); ++i)
    {
        v8::Local<v8::Value> val = info[i];

        if(i > 0) ss << " ";

        auto str = V8::Stringify(ctx, val);
        if(str.IsEmpty()) continue;

        ss << str.CStr();
    }

    alt::ICore::Instance().LogError(ss.str());
}

static void SetTimeout(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN(2);

    V8_ARG_TO_FUNCTION(1, callback);
    V8_ARG_TO_INT(2, time);

    V8_RETURN_INT(resource->CreateTimer(ctx, callback, time, true, V8::SourceLocation::GetCurrent(isolate)));
}

static void SetInterval(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN(2);

    V8_ARG_TO_FUNCTION(1, callback);
    V8_ARG_TO_INT(2, time);

    V8_RETURN_INT(resource->CreateTimer(ctx, callback, time, false, V8::SourceLocation::GetCurrent(isolate)));
}

static void NextTick(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN(1);

    V8_ARG_TO_FUNCTION(1, callback);

    V8_RETURN_INT(resource->CreateTimer(ctx, callback, 0, true, V8::SourceLocation::GetCurrent(isolate)));
}

static void EveryTick(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN(1);

    V8_ARG_TO_FUNCTION(1, callback);

    V8_RETURN_INT(resource->CreateTimer(ctx, callback, 0, false, V8::SourceLocation::GetCurrent(isolate)));
}

static void ClearTimer(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();

    V8_CHECK_ARGS_LEN(1);

    V8_ARG_TO_INT(1, timer);

    resource->RemoveTimer(timer);
}

static void HasResource(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT();
    V8_CHECK_ARGS_LEN(1);

    V8_ARG_TO_STRING(1, name);

    alt::IResource* resource = alt::ICore::Instance().GetResource(name);

    V8_RETURN_BOOLEAN(resource && resource->IsStarted());
}

static void GetEventListeners(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();
    V8_CHECK_ARGS_LEN(1);

    std::vector<V8::EventCallback*> handlers;

    if(info[0]->IsNull())
    {
        handlers = std::move(resource->GetGenericHandlers(true));
    }
    else
    {
        V8_ARG_TO_STRING(1, eventName);
        handlers = std::move(resource->GetLocalHandlers(eventName.ToString()));
    }

    auto array = v8::Array::New(isolate, handlers.size());
    for(int i = 0; i < handlers.size(); i++)
    {
        array->Set(ctx, i, handlers[i]->fn.Get(isolate));
    }

    V8_RETURN(array);
}

static void GetRemoteEventListeners(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    V8_GET_ISOLATE_CONTEXT_RESOURCE();
    V8_CHECK_ARGS_LEN(1);

    std::vector<V8::EventCallback*> handlers;

    if(info[0]->IsNull())
    {
        handlers = std::move(resource->GetGenericHandlers(false));
    }
    else
    {
        V8_ARG_TO_STRING(1, eventName);
        handlers = std::move(resource->GetRemoteHandlers(eventName.ToString()));
    }

    auto array = v8::Array::New(isolate, handlers.size());
    for(int i = 0; i < handlers.size(); i++)
    {
        array->Set(ctx, i, handlers[i]->fn.Get(isolate));
    }

    V8_RETURN(array);
}

extern V8Class v8BaseObject, v8WorldObject, v8Entity, v8File, v8RGBA, v8Vector2, v8Vector3, v8Blip, v8AreaBlip, v8RadiusBlip, v8PointBlip;

extern V8Module sharedModule("alt-shared",
                             nullptr,
                             { v8BaseObject, v8WorldObject, v8Entity, v8File, v8RGBA, v8Vector2, v8Vector3, v8Blip, v8AreaBlip, v8RadiusBlip, v8PointBlip },
                             [](v8::Local<v8::Context> ctx, v8::Local<v8::Object> exports) {
                                 v8::Isolate* isolate = ctx->GetIsolate();

                                 V8Helpers::RegisterFunc(exports, "hash", &HashCb);

                                 V8Helpers::RegisterFunc(exports, "log", &Log);
                                 V8Helpers::RegisterFunc(exports, "logWarning", &LogWarning);
                                 V8Helpers::RegisterFunc(exports, "logError", &LogError);

                                 V8Helpers::RegisterFunc(exports, "on", &On);
                                 V8Helpers::RegisterFunc(exports, "once", &Once);
                                 V8Helpers::RegisterFunc(exports, "off", &Off);
                                 V8Helpers::RegisterFunc(exports, "emit", &Emit);

                                 V8Helpers::RegisterFunc(exports, "getEventListeners", &GetEventListeners);
                                 V8Helpers::RegisterFunc(exports, "getRemoteEventListeners", &GetRemoteEventListeners);

                                 V8Helpers::RegisterFunc(exports, "hasMeta", &HasMeta);
                                 V8Helpers::RegisterFunc(exports, "getMeta", &GetMeta);
                                 V8Helpers::RegisterFunc(exports, "setMeta", &SetMeta);
                                 V8Helpers::RegisterFunc(exports, "deleteMeta", &DeleteMeta);

                                 V8Helpers::RegisterFunc(exports, "hasSyncedMeta", &HasSyncedMeta);
                                 V8Helpers::RegisterFunc(exports, "getSyncedMeta", &GetSyncedMeta);

                                 V8Helpers::RegisterFunc(exports, "nextTick", &NextTick);
                                 V8Helpers::RegisterFunc(exports, "everyTick", &EveryTick);
                                 V8Helpers::RegisterFunc(exports, "setTimeout", &SetTimeout);
                                 V8Helpers::RegisterFunc(exports, "setInterval", &SetInterval);
                                 V8Helpers::RegisterFunc(exports, "clearTimer", &ClearTimer);
                                 V8Helpers::RegisterFunc(exports, "clearNextTick", &ClearTimer);
                                 V8Helpers::RegisterFunc(exports, "clearEveryTick", &ClearTimer);
                                 V8Helpers::RegisterFunc(exports, "clearTimeout", &ClearTimer);
                                 V8Helpers::RegisterFunc(exports, "clearInterval", &ClearTimer);

                                 V8Helpers::RegisterFunc(exports, "hasResource", &HasResource);

                                 V8_OBJECT_SET_STRING(exports, "version", alt::ICore::Instance().GetVersion());
                                 V8_OBJECT_SET_STRING(exports, "branch", alt::ICore::Instance().GetBranch());
                                 V8_OBJECT_SET_INT(exports, "sdkVersion", alt::ICore::Instance().SDK_VERSION);
                                 V8_OBJECT_SET_BOOLEAN(exports, "debug", alt::ICore::Instance().IsDebug());

                                 V8_OBJECT_SET_STRING(exports, "resourceName", V8ResourceImpl::GetResource(ctx)->GetName());

#ifdef ALT_CLIENT_API
                                 V8_OBJECT_SET_BOOLEAN(exports, "isClient", true);
                                 V8_OBJECT_SET_BOOLEAN(exports, "isServer", false);
#endif
#ifdef ALT_SERVER_API
                                 V8_OBJECT_SET_BOOLEAN(exports, "isClient", false);
                                 V8_OBJECT_SET_BOOLEAN(exports, "isServer", true);
#endif
                             });
