#include "Program.hpp"

class asILockableSharedBool;
class asIScriptModule;
class asIScriptObject;

class ScriptProgram : public Program
{
public:
	ScriptProgram();
	~ScriptProgram();

	virtual const std::string& getName() const;

	virtual bool execute(const std::string& command, Robot& actor);

	static ScriptProgram* createForScript(asIScriptModule* mod, const std::string& name);

private:
	void setScriptObject(asIScriptObject* obj);

	asILockableSharedBool* mWeakRef;
	asIScriptObject* mObject;


};