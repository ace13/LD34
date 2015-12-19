#include "ScriptProgram.hpp"

#include <Core/ScriptManager.hpp>


ScriptProgram* ScriptProgram::createForScript(asIScriptModule* mod, const std::string& name)
{

}

ScriptProgram::ScriptProgram() :
	mWeakRef(nullptr),
	mObject(nullptr)
{

}
ScriptProgram::~ScriptProgram()
{

}

const std::string& ScriptProgram::getName() const
{

}

bool ScriptProgram::execute(const std::string& command, Robot& actor)
{

}

void ScriptProgram::setScriptObject(asIScriptObject* obj)
{
	if (!obj)
	{
		if (!mWeakRef->Get())
			mObject->Release();
		mWeakRef->Release();

		mWeakRef = nullptr;
		mObject = nullptr;

		return;
	}

	if (mWeakRef)
	{
		auto oldObjProp = mObject->GetAddressOfProperty(0);
		auto newObjProp = obj->GetAddressOfProperty(0);

		// Move the Object pointer over to the new object
		*reinterpret_cast<ScriptProgram**>(newObjProp) = *reinterpret_cast<ScriptProgram**>(oldObjProp);
		*reinterpret_cast<ScriptProgram**>(oldObjProp) = nullptr;

		//man.removeChangeNotice(mObject);
		
		if (!mWeakRef->Get())
			mObject->Release();
		mObject = nullptr;

		mWeakRef->Release();
		mWeakRef = nullptr;
	}

	mWeakRef = obj->GetWeakRefFlag();
	mWeakRef->AddRef();

	mObject = obj;
	mObject->AddRef();

	/*
	man.addChangeNotice(mObject, [this](asIScriptObject* newObj) {
		setScriptObject(newObj);
	});
	*/
}