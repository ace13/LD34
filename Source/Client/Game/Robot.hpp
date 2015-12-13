#pragma once

#include <Core/Time.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>

class ParticleManager;
class Program;

class Robot : public sf::Transformable, public sf::Drawable
{
public:
	Robot();
	~Robot();

	void tick(const Timespan& dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	bool execute(const std::string& command);
	
	void passParticleManager(ParticleManager*);

	const Program* getProgram() const;
	void setProgram(Program* prog);

	void setSpeed(float speed);
	void turn(float amount);

	void initialize();

private:
	struct State
	{
		float Speed;
		float Angle;
	};

	int mTick;
	ParticleManager* mParticles;
	State mState, mTargetState;

	Program* mCurProgram;
};