#include "Robot.hpp"
#include "Program.hpp"
#include "Level.hpp"
#include "Goal.hpp"

#include "../ParticleManager.hpp"

#include <Core/Math.hpp>

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
	static const ParticleManager::Particle TRACK_PARTICLE{
		std::chrono::seconds(4),
		{ 4, 2 },
		{ 64, 64, 64, 197 },
		{ 64, 64, 64, 0 },
		{ 0, 0 },
		0, 0
	};
}

Robot::Robot() : 
	mTick(0),
	mLevel(nullptr),
	mParticles(nullptr),
	mCurProgram(nullptr),
	mState { },
	mTargetState { },
	mRadius(10)
{

}
Robot::~Robot()
{
	if (mCurProgram)
		delete mCurProgram;
}

void Robot::tick(const Timespan& span)
{
	const float AccelerationSpeed = 4;
	const float RotationSpeed = 4;

	float dt = Time::Seconds(span);

	mState.Angle += (mTargetState.Angle - mState.Angle) * dt * RotationSpeed;
	mState.Speed += (mTargetState.Speed - mState.Speed) * dt * AccelerationSpeed;

	auto newPos = sf::Vector2f(cos(mState.Angle), sin(mState.Angle)) * mState.Speed * mLevel->getScale() * dt;

	auto checkPos = getPosition() + newPos * mRadius;
	if (!mLevel->isBlocked(checkPos.x / mLevel->getScale(), checkPos.y / mLevel->getScale()))
	{
		move(newPos);

		auto levelPos = getPosition() / mLevel->getScale();
		std::list<Entity*> standingOn;
		if (mLevel->findEntities(standingOn, uint8_t(levelPos.x), uint8_t(levelPos.y)))
			for (auto& it : standingOn)
			{
				Goal* test = dynamic_cast<Goal*>(it);
				if (test && !test->isCompleted())
				{
					test->setCompleted();

					mPlayerSound.setBuffer(*test->getSound());
					mPlayerSound.play();
				}
			}
	}
	else
	{
		mTargetState.Speed = 0;
		mState.Speed = mState.Speed / -2;
	}

	setRotation(mState.Angle * Math::RAD2DEG);

	if (mParticles && (mTick++ % 3 == 0) && std::abs(mState.Speed) >= 0.1)
	{
		sf::Vector2f x{
			cos(mState.Angle),
			sin(mState.Angle)
		};
		sf::Vector2f y{
			cos(mState.Angle + Math::PI2),
			sin(mState.Angle + Math::PI2)
		};

		auto& pos = getPosition();
		mParticles->addParticle(TRACK_PARTICLE, pos - (x * 15.f) + (y * 8.f), {}, mState.Angle);
		mParticles->addParticle(TRACK_PARTICLE, pos - (x * 15.f) - (y * 8.f), {}, mState.Angle);
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


const Level* Robot::getLevel() const
{
	return mLevel;
}
Level* Robot::getLevel()
{
	return mLevel;
}
void Robot::setLevel(Level* level)
{
	mLevel = level;
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

float Robot::getRadius() const
{
	return mRadius;
}
void Robot::setRadius(float r)
{
	mRadius = r;
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