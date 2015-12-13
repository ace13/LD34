#pragma once

#include <angelscript.h>
#include <Core/AS_Addons/serializer/serializer.h>
#include <Core/AS_Addons/scriptbuilder/scriptbuilder.h>

#include <functional>
#include <list>
#include <string>
#include <unordered_map>

#ifndef NDEBUG
#include <stdexcept>

#define AS_ASSERT(f) do { int __r = (f); if (__r < 0) throw ASException(#f, __r, __FILE__, __LINE__); } while (false)

namespace sf { class InputStream; }

class ASException : public std::runtime_error
{
public:
	ASException(const std::string& message, int errcode, const std::string& file, int line);
	~ASException() = default;

	const char* what() const noexcept;

	static const char* GetMessage(int code) noexcept;

private:
	std::string mMessage;
};
#else
#define AS_ASSERT(f) f
#endif

namespace sf { class InputStream; }

class ScriptManager
{
public:
	template<typename T>
	class CSimpleType : public CUserType
	{
	public:
		void Store(CSerializedValue *val, void *ptr);
		void Restore(CSerializedValue *val, void *ptr);
		void CleanupUserData(CSerializedValue *val);
	};

	enum ScriptType
	{
		Type_Text,
		Type_Bytecode
	};

	typedef std::function<void(asIScriptEngine*)> ScriptExtensionFun;
	typedef std::function<bool(asIScriptModule*)> ScriptPreLoadCallbackFun;

	void addExtension(const std::string& name, const ScriptExtensionFun& function);
	template<typename T>
	void registerSerializedType(const std::string& name);
	void registerSerializedType(const std::string& name, const std::function<CUserType*()>& ser);

	bool loadFromFile(const std::string& file, ScriptType type = Type_Text);
	bool loadFromMemory(const std::string& name, const void* data, size_t len, ScriptType type = Type_Text);
	bool loadFromStream(const std::string& name, sf::InputStream& stream, ScriptType type = Type_Text);

	void clearPreLoadCallback();
	void setPreLoadCallback(const ScriptPreLoadCallbackFun& func);

	void unloadAll();
	void unload(const std::string& name);
	bool hasLoaded(const std::string& name);

	void addDefine(const std::string& define);

	void init();

	void registerHook(const std::string& name, const std::string& decl);
	template<typename... Args>
	void runHook(const std::string& name, Args... args);

	bool addHook(const std::string& hook, asIScriptFunction* func, asIScriptObject* obj);
	bool removeHook(const std::string& hook, asIScriptFunction* func, asIScriptObject* obj);

	void addPersist(asIScriptObject* obj, const std::function<void(asIScriptObject*)>& callback = std::function<void(asIScriptObject*)>());
	void removePersist(asIScriptObject* obj);

	asIScriptEngine* getEngine();

private:
	struct Script
	{
		std::string Name;
		bool DirectLoad;
	};
	struct Persist
	{
		asILockableSharedBool* WeakRef;
		asIScriptObject* Object;
		std::function<void(asIScriptObject*)> Callback;
	};
	struct ScriptHook
	{
		asIScriptFunction* Function;
		asIScriptObject* Object;
	};

	void addHookFromScript(const std::string& hook, const std::string& func);
	void removeHookFromScript(const std::string& hook, const std::string& func);
	
	std::list<Persist> mPersistant;
	std::list<std::pair<std::string, ScriptExtensionFun>> mExtensions;
	std::unordered_map<std::string, Script> mScripts;
	std::unordered_map<std::string, std::function<CUserType*()>> mSerializers;
	std::unordered_map<std::string, std::string> mRegisteredHooks;
	std::unordered_map<std::string, std::list<ScriptHook>> mScriptHooks;
	ScriptPreLoadCallbackFun mPreLoadCallback;
	asIScriptEngine* mEngine;
	CScriptBuilder mBuilder;
};

#include "ScriptManager.inl"
