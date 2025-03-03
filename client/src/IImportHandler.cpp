#include "IImportHandler.h"
#include "V8Module.h"

static v8::MaybeLocal<v8::Module> CompileESM(v8::Isolate* isolate, const std::string& name, const std::string& src)
{
    v8::Local<v8::String> sourceCode = V8::JSValue(src);

    v8::ScriptOrigin scriptOrigin(isolate, V8::JSValue(name), 0, 0, false, -1, v8::Local<v8::Value>(), false, false, true, v8::Local<v8::PrimitiveArray>());

    v8::ScriptCompiler::Source source{ sourceCode, scriptOrigin };
    return v8::ScriptCompiler::CompileModule(isolate, &source);
}

static v8::MaybeLocal<v8::Module> WrapModule(v8::Isolate* isolate, const std::deque<std::string>& _exportKeys, const std::string& name, bool exportAsDefault = false)
{
    bool hasDefault = false;
    std::stringstream src;

    src << "const _exports = __internal_get_exports('" << name << "');\n";

    for(auto& key : _exportKeys)
    {
        if(key == "default")
        {
            src << "export default _exports['" << key << "'];\n";
            hasDefault = true;
        }
        else
        {
            src << "export let " << key << " = _exports['" << key << "'];\n";
        }
    }

    if(!hasDefault && exportAsDefault) src << "export default _exports;";

    return CompileESM(isolate, name, src.str());
}

static bool IsSystemModule(const std::string& name)
{
    return V8Module::Exists(name);
}

bool IImportHandler::IsValidModule(const std::string& name)
{
    if(V8Module::Exists(name)) return true;

    alt::IResource* resource = alt::ICore::Instance().GetResource(name);
    if(resource) return true;

    return false;
}

std::deque<std::string> IImportHandler::GetModuleKeys(const std::string& name)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    auto context = isolate->GetEnteredOrMicrotaskContext();
    auto& v8module = V8Module::All().find(name);
    if(v8module != V8Module::All().end())
    {
        auto _exports = v8module->second->GetExports(isolate, context);
        v8::Local<v8::Array> v8Keys = _exports->GetOwnPropertyNames(context).ToLocalChecked();

        std::deque<std::string> keys;

        for(uint32_t i = 0; i < v8Keys->Length(); ++i)
        {
            v8::Local<v8::String> v8Key = v8Keys->Get(context, i).ToLocalChecked().As<v8::String>();
            keys.push_back(*v8::String::Utf8Value(isolate, v8Key));
        }

        return keys;
    }

    alt::IResource* resource = alt::ICore::Instance().GetResource(name);
    if(resource)
    {
        std::deque<std::string> keys;

        alt::MValueDict _exports = resource->GetExports();
        for(auto it = _exports->Begin(); it; it = _exports->Next()) keys.push_back(it->GetKey().ToString());

        return keys;
    }

    return {};
}

std::string IImportHandler::GetModulePath(v8::Local<v8::Module> moduleHandle)
{
    for(auto& md : modules)
    {
        if(md.second == moduleHandle) return md.first;
    }

    return std::string{};
}

v8::Local<v8::Module> IImportHandler::GetModuleFromPath(std::string modulePath)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    for(auto& md : modules)
    {
        if(md.first == modulePath) return md.second.Get(isolate);
    }

    return v8::Local<v8::Module>{};
}

v8::MaybeLocal<v8::Value> IImportHandler::Require(const std::string& name)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    auto it = requires.find(name);
    if(it != requires.end()) return it->second.Get(isolate);

    auto& v8module = V8Module::All().find(name);
    if(v8module != V8Module::All().end())
    {
        auto _exports = v8module->second->GetExports(isolate, isolate->GetEnteredOrMicrotaskContext());
        requires.insert({ name, v8::UniquePersistent<v8::Value>{ isolate, _exports } });

        return _exports;
    }

    alt::IResource* resource = alt::ICore::Instance().GetResource(name);
    if(resource)
    {
        v8::Local<v8::Value> _exports = V8Helpers::MValueToV8(resource->GetExports());

        requires.insert({ name, v8::UniquePersistent<v8::Value>{ isolate, _exports } });

        return _exports;
    }

    return v8::MaybeLocal<v8::Value>();
}

v8::MaybeLocal<v8::Module> IImportHandler::ResolveFile(const std::string& name, v8::Local<v8::Module> referrer, alt::IResource* resource)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    auto path = alt::ICore::Instance().Resolve(resource, name, GetModulePath(referrer));

    if(!path.pkg) return v8::MaybeLocal<v8::Module>();

    auto fileName = path.fileName.ToString();

    if(fileName.size() == 0)
    {
        if(path.pkg->FileExists("index.js")) fileName = "index.js";
        else if(path.pkg->FileExists("index.mjs"))
            fileName = "index.mjs";
        else
            return v8::MaybeLocal<v8::Module>();
    }
    else
    {
        if(path.pkg->FileExists(fileName + ".js")) fileName += ".js";
        else if(path.pkg->FileExists(fileName + ".mjs"))
            fileName += ".mjs";
        else if(path.pkg->FileExists(fileName + "/index.js"))
            fileName += "/index.js";
        else if(path.pkg->FileExists(fileName + "/index.mjs"))
            fileName += "/index.mjs";
        else if(!path.pkg->FileExists(fileName))
            return v8::MaybeLocal<v8::Module>();
    }

    std::string fullName = path.prefix.ToString() + fileName;

    auto it = modules.find(fullName);

    if(it != modules.end()) return it->second.Get(isolate);

    v8::MaybeLocal<v8::Module> maybeModule;

    V8Helpers::TryCatch([&] {
        alt::IPackage::File* file = path.pkg->OpenFile(fileName);

        std::string src(path.pkg->GetFileSize(file), '\0');
        path.pkg->ReadFile(file, src.data(), src.size());
        path.pkg->CloseFile(file);

        maybeModule = CompileESM(isolate, fullName, src);

        if(maybeModule.IsEmpty()) return false;

        v8::Local<v8::Module> _module = maybeModule.ToLocalChecked();

        modules.emplace(fullName, v8::UniquePersistent<v8::Module>{ isolate, _module });

        return true;
    });

    if(maybeModule.IsEmpty())
    {
        modules.erase(fullName);
    }

    return maybeModule;
}

v8::MaybeLocal<v8::Module> IImportHandler::ResolveModule(const std::string& _name, v8::Local<v8::Module> referrer, alt::IResource* resource)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::MaybeLocal<v8::Module> maybeModule;

    std::string name = _name;

    if(name == "alt-client") name = "alt";

    auto it = modules.find(name);
    if(it != modules.end())
    {
        maybeModule = it->second.Get(isolate);
    }

    if(maybeModule.IsEmpty())
    {
        if(IsValidModule(name))
        {
            V8Helpers::TryCatch([&] {
                maybeModule = WrapModule(isolate, GetModuleKeys(name), name, IsSystemModule(name));

                if(!maybeModule.IsEmpty())
                {
                    v8::Local<v8::Module> _module = maybeModule.ToLocalChecked();
                    modules.emplace(name, v8::UniquePersistent<v8::Module>{ isolate, _module });

                    /*v8::Maybe<bool> res = _module->InstantiateModule(GetContext(), CV8ScriptRuntime::ResolveModule);
                    if (res.IsNothing())
                    {
                            Log::Info(__LINE__, "res.IsNothing()");
                            return false;
                    }

                    v8::MaybeLocal<v8::Value> res2 = _module->Evaluate(GetContext());
                    if (res2.IsEmpty())
                    {
                            Log::Info(__LINE__, "res2.IsEmpty()");
                            return false;
                    }*/

                    return true;
                }

                Log::Info << __LINE__ << "maybeModule.IsEmpty()";
                return false;
            });

            if(maybeModule.IsEmpty())
            {
                modules.erase(name);
                isolate->ThrowException(v8::Exception::ReferenceError(V8::JSValue(("Failed to load module: " + name))));
                return v8::MaybeLocal<v8::Module>{};
            }
        }
    }

    if(maybeModule.IsEmpty())
    {
        maybeModule = ResolveFile(name, referrer, resource);
    }

    if(maybeModule.IsEmpty())
    {
        modules.erase(name);
        isolate->ThrowException(v8::Exception::ReferenceError(V8::JSValue(("No such module: " + name))));
        return v8::MaybeLocal<v8::Module>{};
    }

    /*v8::Local<v8::Module> _module = maybeModule.ToLocalChecked();
    if (_module->GetStatus() != v8::Module::kEvaluated)
    {
            modules.erase(name);
            isolate->ThrowException(v8::Exception::ReferenceError(v8::String::NewFromUtf8(isolate, ("Failed to import: " + name).c_str())));
            return v8::MaybeLocal<v8::Module>{ };
    }*/

    return maybeModule;
}

v8::MaybeLocal<v8::Module> IImportHandler::ResolveCode(const std::string& code, const V8::SourceLocation& location)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::MaybeLocal<v8::Module> maybeModule;
    std::stringstream name;
    name << "[module " << location.GetFileName() << ":" << location.GetLineNumber() << "]";
    maybeModule = CompileESM(isolate, name.str(), code);

    return maybeModule;
}
