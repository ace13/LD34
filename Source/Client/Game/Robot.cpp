#include "Robot.hpp"
#include "Program.hpp"

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Robot::Robot() : 
	mCurProgram(nullptr),
	mState { },
	mTargetState { }
{

}
Robot::~Robot()
{
	if (mCurProgram)
		delete mCurProgram;
}

void Robot::tick(const Timespan& span)
{
	const float MoveSpeed = 250;
	const float AccelerationSpeed = 2;
	const float RotationSpeed = 2;

	float dt = Time::Seconds(span);

	mState.Angle += (mTargetState.Angle - mState.Angle) * dt * RotationSpeed;
	mState.Speed += (mTargetState.Speed - mState.Speed) * dt * AccelerationSpeed;

	mPosition += sf::Vector2f(cos(mState.Angle) * mState.Speed, sin(mState.Angle) * mState.Speed) * MoveSpeed * dt;
}

void Robot::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	sf::ConvexShape shape(5);
	shape.setPoint(0, { 20, 0 });
	shape.setPoint(1, { 0, 10 });
	shape.setPoint(2, { -20, 10 });
	shape.setPoint(3, { -20, -10 });
	shape.setPoint(4, { 0, -10 });

	shape.setPosition(mPosition);
	shape.setRotation(mState.Angle * (180 / 3.14159f));

	target.draw(shape, states);
}

void Robot::execute(const std::string& command)
{
	if (mCurProgram)
		mCurProgram->execute(command, *this);
}

void Robot::setProgram(Program* prog)
{
	if (mCurProgram)
		delete mCurProgram;

	mCurProgram = prog;
}

void Robot::setSpeed(float speed)
{
	mTargetState.Speed = speed;
}
void Robot::turn(float amount)
{
	mTargetState.Angle = mTargetState.Angle + amount;
}