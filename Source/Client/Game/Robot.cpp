#include "Robot.hpp"
#include "Program.hpp"

#include "../ParticleManager.hpp"

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
	static const ParticleManager::Particle TRACK_PARTICLE{
		std::chrono::seconds(2),
		{ 8, 2 },
		{ 64, 64, 64, 197 },
		{ 64, 64, 64, 0 },
		{ 0, 0 },
		0, 0
	};
}

Robot::Robot() : 
	mTick(0),
	mParticles(nullptr),
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

	if (mParticles && (mTick++ % 3 == 0) && std::abs(mState.Speed) >= 0.1)
	{
		sf::Vector2f tmp{
			cos(mState.Angle + (3.14159f / 2)),
			sin(mState.Angle + (3.14159f / 2))
		};

		mParticles->addParticle(TRACK_PARTICLE, mPosition + (tmp * 10.f), {}, mState.Angle);
		mParticles->addParticle(TRACK_PARTICLE, mPosition - (tmp * 10.f), {}, mState.Angle);
	}
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

bool Robot::execute(const std::string& command)
{
	if (mCurProgram)
		return mCurProgram->execute(command, *this);

	return false;
}

void Robot::passParticleManager(ParticleManager* pman)
{
	mParticles = pman;
}

const sf::Vector2f& Robot::getPosition() const
{
	return mPosition;
}
void Robot::setPosition(const sf::Vector2f& pos)
{
	mPosition = pos;
}

const Program* Robot::getProgram() const
{
	return mCurProgram;
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