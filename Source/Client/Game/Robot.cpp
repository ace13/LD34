#include "Robot.hpp"
#include "Program.hpp"

#include "../ParticleManager.hpp"

#include <Core/Math.hpp>

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
	const float MoveSpeed = 150;
	const float AccelerationSpeed = 2;
	const float RotationSpeed = 2;

	float dt = Time::Seconds(span);

	mState.Angle += (mTargetState.Angle - mState.Angle) * dt * RotationSpeed;
	mState.Speed += (mTargetState.Speed - mState.Speed) * dt * AccelerationSpeed;

	move(sf::Vector2f(cos(mState.Angle) * mState.Speed, sin(mState.Angle) * mState.Speed) * MoveSpeed * dt);
	setRotation(mState.Angle * Math::RAD2DEG);

	if (mParticles && (mTick++ % 3 == 0) && std::abs(mState.Speed) >= 0.1)
	{
		sf::Vector2f tmp{
			cos(mState.Angle + Math::PI2),
			sin(mState.Angle + Math::PI2)
		};

		auto& pos = getPosition();
		mParticles->addParticle(TRACK_PARTICLE, pos + (tmp * 10.f), {}, mState.Angle);
		mParticles->addParticle(TRACK_PARTICLE, pos - (tmp * 10.f), {}, mState.Angle);
	}
}

void Robot::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	sf::ConvexShape shape(5);
	shape.setPoint(0, { 20, 0 });
	shape.setPoint(1, { 0, 10 });
	shape.setPoint(2, { -20, 10 });
	shape.setPoint(3, { -20, -10 });
	shape.setPoint(4, { 0, -10 });

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

void Robot::initialize()
{
	mTargetState.Speed = 0;
	mTargetState.Angle = getRotation() * Math::DEG2RAD;
	mState = mTargetState;
}