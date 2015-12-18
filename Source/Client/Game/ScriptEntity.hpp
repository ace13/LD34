#pragma once

#include "Entity.hpp"

class asILockableSharedBool;
class asIScriptModule;
class asIScriptObject;
class asIScriptFunction;

class ScriptEntity : public Entity
{
public:
	ScriptEntity();
	~ScriptEntity();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual std::string serialize() const;
	virtual bool deserialize(const std::string&);

	virtual const std::type_info& getType() const;
	virtual const std::string& getName() const;

	virtual void initialize();

	const asIScriptObject* getScriptObject() const;


	static void registerType(ScriptManager&);
	static bool preLoadInject(asIScriptModule* mod);
	static ScriptEntity* createForScript(asIScriptModule* module, const std::string& type);

protected:
	static ScriptEntity* createFromScript();

	void setScriptObject(asIScriptObject* obj);

	std::string mName;

	asILockableSharedBool* mScript;
	asIScriptObject* mObject;
	asIScriptFunction *mTick, *mUpdate, *mDraw;
};