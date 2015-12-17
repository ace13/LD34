#pragma once

#include "Entity.hpp"
#include "../ResourceManager.hpp"

#include <Core/Time.hpp>

#include <SFML/Audio/Sound.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>

class ParticleManager;
class Program;
class Level;

class Robot : public Entity
{
public:
	Robot();
	~Robot();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual std::string serialize() const;
	virtual bool deserialize(const std::string&);

	virtual const std::string& getName() const;

	bool execute(const std::string& command);
	
	const Program* getProgram() const;
	void setProgram(Program* prog);

	void setSpeed(float speed);
	void turn(float amount);

	void initialize();
	void reset();

private:
	struct State
	{
		float Speed;
		float Angle;
	};

	int mTick, mKeyCount;
	State mState, mTargetState;

	ResourceManager::Sound mExplodeSound;
	sf::Sound mPlayerSound;

	Program* mCurProgram;
};
