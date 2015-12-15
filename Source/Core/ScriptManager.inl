#pragma once



template<typename T>
void ScriptManager::CSimpleType<T>::Store(CSerializedValue *val, void *ptr)
{
	val->SetUserData(new T(*reinterpret_cast<T*>(ptr)));
}
template<typename T>
void ScriptManager::CSimpleType<T>::Restore(CSerializedValue *val, void *ptr)
{
	auto buffer = reinterpret_cast<T*>(val->GetUserData());
	*reinterpret_cast<T*>(ptr) = *buffer;
}
template<typename T>
void ScriptManager::CSimpleType<T>::CleanupUserData(CSerializedValue *val)
{
	auto buffer = reinterpret_cast<T*>(val->GetUserData());

	if (buffer)
		delete buffer;

	val->SetUserData(nullptr);
}

template<typename T>
void ScriptManager::registerSerializedType(const std::string& name)
{
	registerSerializedType(name, []() -> CUserType* {
		return new CSimpleType<T>();
	});
}

namespace
{
	template<typename... Args>
	void setCTXArgs(asIScriptContext*, uint32_t) { }

	template<typename Arg, typename... Args>
	void setCTXArgs(asIScriptContext* ctx, uint32_t id, Arg a1, Args... args)
	{
		ScriptManager::setCTXArg<Arg>(ctx, id, std::forward<Arg>(a1));
		setCTXArgs(ctx, id, std::forward<Args>(args)...);
	}
}

template<typename... Args>
void ScriptManager::runHook(const std::string& name, Args... args)
{
	if (mRegisteredHooks.count(name) == 0 || mScriptHooks.count(name) == 0)
		return;

	auto* ctx = mEngine->RequestContext();

	auto copy = mScriptHooks.at(name);

	for (auto& hook : copy)
	{
		int r = 0;
		r = ctx->Prepare(hook.Function);
		if (r < 0)
			continue;
		r = ctx->SetObject(hook.Object);
		if (r < 0)
			continue;

		setCTXArgs<Args...>(ctx, 0, std::forward<Args>(args)...);

		ctx->Execute();

		ctx->Unprepare();
	}

	mEngine->ReturnContext(ctx);
}
