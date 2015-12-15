#include "ScriptManager.hpp"

#include <SFML/System/InputStream.hpp>

#include <angelscript.h>

#include <algorithm>
#include <iterator>
#include <fstream>
#include <iostream>
#include <locale>
#include <sstream>

#ifndef NDEBUG
ASException::ASException(const std::string& message, int errcode, const std::string& file, int line) : std::runtime_error("")
{
	std::ostringstream oss;

	if (file.empty())
		oss << message << " (" << errcode << " - " << std::string(GetMessage(errcode)) << ")";
	else
		oss << file << ":" << line << ": Call to AngelScript failed with error " << (-errcode) << " - " << std::string(GetMessage(errcode)) << std::endl << ">  The call was: '" << message << "'";

	mMessage = oss.str();
}

const char* ASException::what() const noexcept
{
	return mMessage.c_str();
}

inline const char* ASException::GetMessage(int code) noexcept
{
	switch (code)
	{
	case asSUCCESS:
		return "Success";
	case asERROR:
		return "Error";
	case asCONTEXT_ACTIVE:
		return "Context is active";
	case asCONTEXT_NOT_FINISHED:
		return "Context is not finished";
	case asCONTEXT_NOT_PREPARED:
		return "Context is not prepared";
	case asINVALID_ARG:
		return "Invalid argument";
	case asNO_FUNCTION:
		return "No function";
	case asNOT_SUPPORTED:
		return "Not supported";
	case asINVALID_NAME:
		return "Invalid name";
	case asNAME_TAKEN:
		return "Name is taken";
	case asINVALID_DECLARATION:
		return "Invalid declaration";
	case asINVALID_OBJECT:
		return "Invalid object";
	case asINVALID_TYPE:
		return "Invalid type";
	case asALREADY_REGISTERED:
		return "Already registered";
	case asMULTIPLE_FUNCTIONS:
		return "Multiple functions";
	case asNO_MODULE:
		return "No module";
	case asNO_GLOBAL_VAR:
		return "No global variable";
	case asINVALID_CONFIGURATION:
		return "Invalid configuration";
	case asINVALID_INTERFACE:
		return "Invalid interface";
	case asCANT_BIND_ALL_FUNCTIONS:
		return "Can't bind all functions";
	case asLOWER_ARRAY_DIMENSION_NOT_REGISTERED:
		return "Lower array dimension not registered";
	case asWRONG_CONFIG_GROUP:
		return "Wrong config group";
	case asCONFIG_GROUP_IS_IN_USE:
		return "Config group is in use";
	case asILLEGAL_BEHAVIOUR_FOR_TYPE:
		return "Illegal behaviour for type";
	case asWRONG_CALLING_CONV:
		return "Wrong calling convention";
	case asBUILD_IN_PROGRESS:
		return "Build in progress";
	case asINIT_GLOBAL_VARS_FAILED:
		return "Initializing global variables failed";
	case asOUT_OF_MEMORY:
		return "Out of memory";
	case asMODULE_IS_IN_USE:
		return "Module is in use";

	default:
		return "";
	}
}
#endif

namespace
{

	void error(const asSMessageInfo* msg)
	{
		std::cerr << "Angelscript ";
		switch (msg->type)
		{
		case asMSGTYPE_ERROR: std::cerr << "error"; break;
		case asMSGTYPE_WARNING: std::cerr << "warning"; break;
		case asMSGTYPE_INFORMATION: std::cerr << "info"; break;
		}

		if (msg->section && msg->section[0] != 0)
			std::cerr << " (" << msg->section << ":" << msg->row << ":" << msg->col << ")";
		std::cerr << ";" << std::endl
			<< ">  " << msg->message << std::endl;
	}

	class BytecodeStore : public asIBinaryStream
	{
	public:
		BytecodeStore() : mTellg(0) { }
		BytecodeStore(const char* data, size_t len) :
			mTellg(0)
		{
			mStore.assign(data, data + len);
		}

		void Read(void *ptr, asUINT size)
		{
			char* data = (char*)ptr;

			for (uint32_t i = 0; i < size; ++i)
			{
				data[i] = mStore[mTellg + i];
			}

			mTellg += size;
		}
		void Write(const void *ptr, asUINT size)
		{
			const char* data = (const char*)ptr;

			for (uint32_t i = 0; i < size; ++i)
				mStore.push_back(data[i]);
		}

	private:
		std::vector<char> mStore;
		size_t mTellg;
	};
}

void ScriptManager::addExtension(const std::string& name, const ScriptExtensionFun& function)
{
	mExtensions.emplace_back(name, function);
}
void ScriptManager::registerSerializedType(const std::string& name, const std::function<CUserType*()>& ser)
{
	mSerializers[name] = ser;
}

void ScriptManager::init()
{
	addExtension("ScriptHooks", [this](asIScriptEngine* eng) {
		eng->SetDefaultNamespace("Hooks");

		eng->RegisterGlobalFunction("void Add(const string&in, const string&in)", asMETHOD(ScriptManager, addHookFromScript), asCALL_THISCALL_ASGLOBAL, this);
		eng->RegisterGlobalFunction("void Remove(const string&in, const string&in = \"\")", asMETHOD(ScriptManager, removeHookFromScript), asCALL_THISCALL_ASGLOBAL, this);

		eng->SetDefaultNamespace("");
	});

	asIScriptEngine* eng = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	eng->SetUserData(this, 0x4547);
	eng->SetMessageCallback(asFUNCTION(error), nullptr, asCALL_CDECL);

#ifndef NDEBUG
	std::unordered_map<std::string, uint32_t> counts;
#define CHECK_COUNT(Name, Check) do { \
	auto cnt = eng->Get ## Check ## Count(); \
	if (counts.count(Name) > 0 && cnt != counts.at(Name)) \
	{ \
		std::cout << ">  " << (cnt - counts.at(Name)) << " new " Name "(s)" << std::endl; \
	} \
	counts[Name] = cnt; \
} while (false);
#else
#define CHECK_COUNT(A, B)
#endif
	for (auto& ext : mExtensions)
	{
		std::cout << "Registering " << std::get<0>(ext) << "..." << std::endl;
		try
		{
			std::get<1>(ext)(eng);

			CHECK_COUNT("enum", Enum);
			CHECK_COUNT("funcdef", Funcdef);
			CHECK_COUNT("global function", GlobalFunction);
			CHECK_COUNT("global property", GlobalProperty);
			CHECK_COUNT("object type", ObjectType);
			CHECK_COUNT("typedef", Typedef);
		}
		catch (const std::runtime_error& ex)
		{
			std::cerr << std::string(ex.what()) << std::endl;
			std::cout << "Failed to register " << std::get<0>(ext) << std::endl;
		}
	}
#undef CHECK_COUNT

	//eng->ClearMessageCallback();
	mEngine = eng;
}

bool ScriptManager::loadFromFile(const std::string& file, ScriptType type)
{
	std::ifstream ifs(file.c_str());
	if (!ifs)
		return false;

	ifs.seekg(0, std::ios::end);
	size_t len = size_t(ifs.tellg());
	ifs.seekg(0, std::ios::beg);

	std::vector<char> data(len);
	ifs.read(&data[0], len);

	// Remove extraneous nullbytes
	auto it = std::remove(data.begin(), data.end(), '\0');
	if (it != data.end())
		data.erase(it, data.end());

	return loadFromMemory(file, &data[0], data.size(), type);
}
bool ScriptManager::loadFromMemory(const std::string& name, const void* data, size_t len, ScriptType type)
{
	std::string lower;
	std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);
	bool reload = mScripts.count(lower) > 0;

	std::list<Persist> toPersist;
	asIScriptModule* module = mEngine->GetModule(lower.c_str(), asGM_ONLY_IF_EXISTS);
	CSerializer serial;

	if (reload && module)
	{
		for (auto& reg : mSerializers)
			serial.AddUserType(reg.second(), reg.first);

		for (auto it = mPersistant.begin(); it != mPersistant.end();)
		{
			if (it->WeakRef->Get())
			{
				it->WeakRef->Release();
				it = mPersistant.erase(it);
				continue;
			}

			auto* obj = it->Object;

			if (obj->GetObjectType()->GetModule() == module)
			{
				serial.AddExtraObjectToStore(obj);

				toPersist.push_back(*it);
				obj->AddRef();

				it = mPersistant.erase(it);
			}
			else
				++it;
		}

		serial.Store(module);
	}

	BytecodeStore bcode;
	if (type == Type_Text)
	{
		static const char* scratchName = "!!ScratchSpace!!";

		mBuilder.StartNewModule(mEngine, scratchName);

		if (mPreLoadCallback && !mPreLoadCallback(mBuilder.GetModule()))
		{
			for (auto& it : toPersist)
			{
				if (!it.WeakRef->Get())
					it.Object->Release();
				mPersistant.push_back(it);
			}

			return false;
		}

		mBuilder.AddSectionFromMemory(lower.c_str(), (const char*)data, len);

		int r = mBuilder.BuildModule();
		if (r < 0)
		{
			for (auto& it : toPersist)
			{
				if (!it.WeakRef->Get())
					it.Object->Release();
				mPersistant.push_back(it);
			}

#ifndef NDEBUG
			puts(ASException::GetMessage(r));
#endif
			return false;
		}

#ifdef NDEBUG
		mBuilder.GetModule()->SaveByteCode(&bcode, true);
#else
		mBuilder.GetModule()->SaveByteCode(&bcode, false);
#endif

		mEngine->DiscardModule(scratchName);
	}
	else
	{
		bcode = BytecodeStore((const char*)data, len);
	}

	if (module)
		module->Discard();

	module = mEngine->GetModule(lower.c_str(), asGM_ALWAYS_CREATE);

	if (type == Type_Bytecode && mPreLoadCallback && !mPreLoadCallback(module))
	{
		for (auto& it : toPersist)
		{
			if (!it.WeakRef->Get())
				it.Object->Release();
			mPersistant.push_back(it);
		}

		module->Discard();

		return false;
	}

	int r = module->LoadByteCode(&bcode);
	if (r < 0)
	{
		for (auto& it : toPersist)
		{
			if (!it.WeakRef->Get())
				it.Object->Release();
			mPersistant.push_back(it);
		}

		module->Discard();

		return false;
	}

	module->BindAllImportedFunctions();

	if (mScripts.count(lower) == 0)
		mScripts[lower].Name = name;

	auto* fun = module->GetFunctionByName("OnLoad");
	if (reload)
	{
		serial.Restore(module);

		for (auto& it : toPersist)
		{
			auto* newObj = (asIScriptObject*)serial.GetPointerToRestoredObject(it.Object);

			if (it.Callback)
				it.Callback(newObj);

			mPersistant.push_back({ newObj->GetWeakRefFlag(), newObj, it.Callback });
			newObj->GetWeakRefFlag()->AddRef();

			if (!it.WeakRef->Get())
				it.Object->Release();

			it.WeakRef->Release();
		}

		mEngine->GarbageCollect(asGC_FULL_CYCLE);

		fun = module->GetFunctionByName("OnReload");
		if (fun)
		{
			auto* ctx = mEngine->RequestContext();
			ctx->Prepare(fun);

			ctx->Execute();

			ctx->Unprepare();
			mEngine->ReturnContext(ctx);
		}
	}
	else if (fun)
	{
		auto* ctx = mEngine->RequestContext();
		ctx->Prepare(fun);

		ctx->Execute();

		ctx->Unprepare();
		mEngine->ReturnContext(ctx);
	}

	if ((fun = module->GetFunctionByName("OnLoad")))
		module->RemoveFunction(fun);
	if ((fun = module->GetFunctionByName("OnReload")))
		module->RemoveFunction(fun);

	return true;
}
bool ScriptManager::loadFromStream(const std::string& name, sf::InputStream& stream, ScriptType type)
{
	auto len = size_t(stream.getSize());
	std::vector<char> data(len);
	stream.read(&data[0], len);

	return loadFromMemory(name, &data[0], len, type);
}

void ScriptManager::clearPreLoadCallback()
{
	mPreLoadCallback = ScriptPreLoadCallbackFun();
}
void ScriptManager::setPreLoadCallback(const ScriptPreLoadCallbackFun& func)
{
	mPreLoadCallback = func;
}

void ScriptManager::unload(const std::string& name)
{
	asIScriptModule* module = mEngine->GetModule(name.c_str(), asGM_ONLY_IF_EXISTS);
	if (!module)
		return;

	std::list<asIScriptObject*> released;

	for (auto it = mPersistant.begin(); it != mPersistant.end();)
	{
		auto* obj = it->Object;

		if (obj->GetObjectType()->GetModule() == module)
		{
			if (it->WeakRef->Get() || obj->Release() <= 0)
				released.push_back(obj);

			it->WeakRef->Release();
			it = mPersistant.erase(it);
		}
		else
			++it;
	}

	for (auto& hooks : mScriptHooks)
	{
		for (auto it = hooks.second.begin(); it != hooks.second.end();)
		{
			auto* obj = it->Object;

			auto find = std::find(released.begin(), released.end(), obj);
			if (find != released.end())
			{
				it = hooks.second.erase(it);
			}
			else if (obj->GetObjectType()->GetModule() == module)
			{
				if (obj->Release() <= 0)
					released.push_back(obj);

				it = hooks.second.erase(it);
			}
			else
				++it;
		}
	}

	mScripts.erase(name);

	mEngine->GarbageCollect(asGC_FULL_CYCLE, 2);
}

void ScriptManager::unloadAll()
{
	for (auto& script : mScripts)
	{
		unload(script.first);
	}

	mEngine->GarbageCollect(asGC_FULL_CYCLE, 5);
}

bool ScriptManager::hasLoaded(const std::string& name)
{
	std::string lower;
	std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);

	return mScripts.count(lower) > 0;
}

void ScriptManager::registerHook(const std::string& name, const std::string& decl)
{
	mRegisteredHooks.emplace(name, decl);
}
bool ScriptManager::addHook(const std::string& hook, asIScriptFunction* func, asIScriptObject* obj)
{
	if (mRegisteredHooks.count(hook) == 0)
		return false;

	auto& data = mRegisteredHooks.at(hook);
	std::string decl = func->GetDeclaration(false);
	std::string name = func->GetName();
	auto off = decl.find(name);
	decl.replace(off, name.length(), "f");

	if (decl != data)
		return false;

	auto& hooks = mScriptHooks[hook];
	auto it = std::find_if(hooks.begin(), hooks.end(), [obj, func](ScriptHook& hook) { return hook.Function == func && hook.Object == obj; });
	if (it == hooks.end())
		hooks.push_back({ func, obj });

	return true;
}
bool ScriptManager::removeHook(const std::string& hook, asIScriptFunction* func, asIScriptObject* obj)
{
	if (mRegisteredHooks.count(hook) == 0 || mScriptHooks.count(hook) == 0)
		return false;

	auto& hooks = mScriptHooks.at(hook);
	auto it = std::remove_if(hooks.begin(), hooks.end(), [obj, func](ScriptHook& hook) { return hook.Function == func && hook.Object == obj; });

	if (it != hooks.end())
	{
		hooks.erase(it, hooks.end());

		if (hooks.empty())
			mScriptHooks.erase(hook);

		return true;
	}

	return false;
}

void ScriptManager::addHookFromScript(const std::string& hook, const std::string& func)
{
	auto* ctx = asGetActiveContext();
	if (mRegisteredHooks.count(hook) == 0)
	{
		ctx->SetException("No such hook");
		return;
	}

	asIScriptObject* obj = (asIScriptObject*)ctx->GetThisPointer();
	auto* funcptr = obj->GetObjectType()->GetMethodByName(func.c_str());

	if (!addHook(hook, funcptr, obj))
	{
		ctx->SetException(("Invalid declaration for hook '" + hook + "'\n" +
			"Expects " + mRegisteredHooks.at(hook)).c_str());

		return;
	}

	static std::function<void(asIScriptObject* oldObj, asIScriptFunction* func, asIScriptObject* newObj)> persistFunc;
	if (!persistFunc)
	persistFunc = [this](asIScriptObject* oldObj, asIScriptFunction* func, asIScriptObject* newObj)
	{
		for (auto& hooks : mScriptHooks)
		{
			auto it = std::find_if(hooks.second.begin(), hooks.second.end(), [oldObj, func](const ScriptHook& h) { return h.Function == func && h.Object == oldObj; });

			if (it != hooks.second.end())
			{
				auto* decl = it->Function->GetDeclaration(false);
				it->Function = newObj->GetObjectType()->GetMethodByDecl(decl);

				it->Object->Release();
				removePersist(it->Object);

				it->Object = newObj;
				it->Object->AddRef();

				addPersist(newObj, [=](asIScriptObject* obj) {
					persistFunc(newObj, func, obj);
				});
			}
		}
	};

	addPersist(obj, [=](asIScriptObject* newObj) {
		persistFunc(obj, funcptr, newObj);
	});
	obj->AddRef();
}
void ScriptManager::removeHookFromScript(const std::string& hook, const std::string& func)
{
	auto* ctx = asGetActiveContext();
	if (mRegisteredHooks.count(hook) == 0)
	{
		ctx->SetException("No such hook");
		return;
	}

	asIScriptObject* obj = (asIScriptObject*)ctx->GetThisPointer();
	asIScriptFunction* funcptr = nullptr;
	if (func.empty())
	{
		if (mScriptHooks.count(hook) > 0)
		{
			auto& hooks = mScriptHooks.at(hook);
			std::find_if(hooks.begin(), hooks.end(), [&funcptr, obj](const ScriptHook& sh) {
				if (sh.Object == obj)
				{
					funcptr = sh.Function;
					return true;
				}
				return false;
			});
		}

		if (!funcptr)
		{
			ctx->SetException("No such hook registered");
			return;
		}
	}
	else
		funcptr = obj->GetObjectType()->GetMethodByName(func.c_str());

	if (removeHook(hook, funcptr, obj))
	{
		int i = obj->Release();
		if (i <= 0)
			removePersist(obj);
	}
}

void ScriptManager::addPersist(asIScriptObject* obj, const std::function<void(asIScriptObject*)>& callback)
{
	auto it = std::find_if(mPersistant.begin(), mPersistant.end(), [obj](const Persist& p) { return p.Object == obj; });
	if (it == mPersistant.end())
	{
		mPersistant.push_back({ obj->GetWeakRefFlag(), obj, callback });
		obj->GetWeakRefFlag()->AddRef();
	}
}
void ScriptManager::removePersist(asIScriptObject* obj)
{
	auto it = std::find_if(mPersistant.begin(), mPersistant.end(), [obj](const Persist& p) { return p.Object == obj; });
	if (it != mPersistant.end())
	{
		it->WeakRef->Release();
		mPersistant.erase(it);
	}
}

asIScriptEngine* ScriptManager::getEngine()
{
	return mEngine;
}

#define PRIMITIVE_ARG(Type, SetType) template<> \
	void ScriptManager::setCTXArg<Type>(asIScriptContext* ctx, uint32_t id, Type arg) \
	{ \
		ctx->SetArg ## SetType (id, arg); \
	}//

	PRIMITIVE_ARG(uint8_t, Byte)
	PRIMITIVE_ARG(uint16_t, Word)
	PRIMITIVE_ARG(uint32_t, DWord)
	PRIMITIVE_ARG(uint64_t, QWord)
	PRIMITIVE_ARG(bool, Byte)
	PRIMITIVE_ARG(int8_t, Byte)
	PRIMITIVE_ARG(int16_t, Word)
	PRIMITIVE_ARG(int32_t, DWord)
	PRIMITIVE_ARG(int64_t, QWord)
	PRIMITIVE_ARG(float, Float)
	PRIMITIVE_ARG(double, Double)
#undef PRIMITIVE_ARG
