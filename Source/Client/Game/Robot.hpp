#pragma once

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

class Robot : public sf::Transformable, public sf::Drawable
{
public:
	Robot();
	~Robot();

	void tick(const Timespan& dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	bool execute(const std::string& command);
	
	void passParticleManager(ParticleManager*);

	const Level* getLevel() const;
	Level* getLevel();
	void setLevel(Level*);

	const Program* getProgram() const;
	void setProgram(Program* prog);

	float getRadius() const;
	void setRadius(float);

	void setSpeed(float speed);
	void turn(float amount);

	void initialize();

private:
	struct State
	{
		float Speed;
		float Angle;
	};

	int mTick, mKeyCount;
	Level* mLevel;
	ParticleManager* mParticles;
	State mState, mTargetState;

	ResourceManager::Sound mExplodeSound;
	sf::Sound mPlayerSound;

	float mRadius;

	Program* mCurProgram;
};