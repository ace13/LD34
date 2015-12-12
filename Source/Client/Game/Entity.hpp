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
class GameState;

class Entity : public sf::Transformable, public sf::Drawable
{
public:
	Entity();
	virtual ~Entity();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	int addRef();
	int release();

	static void registerType(ScriptManager&);
	static bool preLoadInject(asIScriptModule* mod);

	static GameState* CurGameState;

private:
	static Entity* createFromScript();

	void setScriptObject(asIScriptObject* obj);

	asILockableSharedBool* mScript;
	asIScriptObject* mObject;
	asIScriptFunction *mTick, *mUpdate, *mDraw;

	int mRefCount;
};