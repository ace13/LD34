#pragma once

#include <Core/Time.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>

class Program;

class Robot : public sf::Drawable
{
public:
	Robot();
	~Robot();

	void tick(const Timespan& dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	void execute(const std::string& command);
	
	void setProgram(Program* prog);

	void setSpeed(float speed);
	void turn(float amount);

private:
	struct State
	{
		float Speed;
		float Angle;
	};

	State mState, mTargetState;
	sf::Vector2f mPosition;

	Program* mCurProgram;
};