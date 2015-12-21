#pragma once

#include <Core/Time.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>

class Engine;
class InputStream;
class OutputStream;

class ScriptManager;
class ParticleManager;
class Level;

class Entity : public sf::Transformable, public sf::Drawable
{
public:
	Entity();
	virtual ~Entity();

	virtual void tick(const Timespan& dt) = 0;
	virtual void update(const Timespan& dt) = 0;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;

	virtual bool serialize(OutputStream&) const = 0;
	virtual bool deserialize(InputStream&) = 0;

	virtual void initialize() = 0;

	virtual const std::type_info& getType() const = 0;
	virtual const std::string& getName() const = 0;
	virtual bool isGoal() const;
	virtual bool isCompleted() const;


	// Overridden transformable functions
	void move(float, float);
	void move(const sf::Vector2f&);

	// Ref counting
	int addRef();
	int release();

	// Entity features
	float getRadius() const;
	void setRadius(float);

	const ParticleManager* getParticleManager(bool post = false) const;
	ParticleManager* getParticleManager(bool post=false);

	const Engine* getEngine() const;
	Engine* getEngine();

	const Level* getLevel() const;
	Level* getLevel();

	static Entity* createFromType(const std::string& name);

protected:
	void setGoal(bool isGoal = true);
	void setCompleted(bool completed = true);

private:
	void setLevel(Level*);

	std::string mName;

	Level* mLevel;

	bool mIsGoal, mIsCompleted;
	float mRadius;

	int mRefCount;

	friend class Level;
};
