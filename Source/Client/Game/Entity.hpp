#pragma once

#include <Core/Time.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>

class asILockableSharedBool;
class asIScriptModule;
class asIScriptObject;
class asIScriptFunction;
class ScriptManager;
class Level;

class Entity : public sf::Transformable, public sf::Drawable
{
public:
	Entity();
	virtual ~Entity();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	void move(const sf::Vector2f&);

	virtual bool serialize(char* data, size_t size) const;
	virtual bool deserialize(const char* data, size_t size);

	virtual const std::string& getName() const;
	virtual bool isScriptEntity() const;
	virtual bool isGoal() const;
	virtual bool isCompleted() const;

	virtual void initialize();

	int addRef();
	int release();

	static void registerType(ScriptManager&);
	static bool preLoadInject(asIScriptModule* mod);

	float getRadius() const;
	void setRadius(float);

	const Level* getLevel() const;
	Level* getLevel();
	void setLevel(Level*);
	const asIScriptObject* getScriptObject() const;

	static Entity* createFromType(const char* type, const char* data, size_t len);
	static Entity* createForScript(asIScriptModule* module, const char* type);

protected:
	void setGoal(bool isGoal = true);
	void setCompleted(bool completed = true);

	void setScriptObject(asIScriptObject* obj);

private:
	static Entity* createFromScript();

	std::string mName;

	Level* mLevel;
	asILockableSharedBool* mScript;
	asIScriptObject* mObject;
	asIScriptFunction *mTick, *mUpdate, *mDraw;

	bool mIsGoal, mIsCompleted;
	float mRadius;

	int mRefCount;
};