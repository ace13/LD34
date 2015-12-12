#pragma once

#include <Core/Time.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>

class asILockableSharedBool;
class asIScriptModule;
class asIScriptObject;
class asIScriptFunction;
class ScriptManager;

class Entity : public sf::Transformable, public sf::Drawable
{
public:
	Entity();
	virtual ~Entity();

	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	int addRef();
	int release();

	static void registerType(ScriptManager&);
	static bool preLoadInject(asIScriptModule* mod);

private:
	static Entity* createFromScript();

	void setScriptObject(asIScriptObject* obj);

	asILockableSharedBool* mScript;
	asIScriptObject* mObject;
	asIScriptFunction *mUpdate, *mDraw;

	int mRefCount;
};