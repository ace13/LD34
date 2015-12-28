#include "Robot.hpp"
#include "Program.hpp"
#include "Level.hpp"
#include "Goal.hpp"
#include "Door.hpp"
#include "Key.hpp"
#include "Enemy.hpp"
#include "Box.hpp"
#include "Pit.hpp"

#include "../ParticleManager.hpp"

#include <Core/Engine.hpp>
#include <Core/Math.hpp>
#include <Core/OutputStream.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cstring>

namespace
{
	static const ParticleManager::Particle TRACK_PARTICLE{
		std::chrono::seconds(4),
		{ 0, 0, 8, 4 },
		{ 64, 64, 64, 197 },
		{ 64, 64, 64, 0 },
		{ 0, 0 },
		0, 0
	};
}

Robot::Robot() : 
	mTick(0),
	mKeyCount(0),
	mState { },
	mTargetState { },
	mCurProgram(nullptr)
{
	setRadius(10);
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

	auto newPos = sf::Vector2f(cos(mState.Angle), sin(mState.Angle)) * mState.Speed * getLevel()->getScale() * dt;

	if (!move(newPos))
	{
		mTargetState.Speed = 0;
		mState.Speed = mState.Speed / -2;
	}

	std::list<Entity*> standingOn;
	if (getLevel()->findEntities(standingOn, *this))
		for (auto& it : standingOn)
		{
			auto& otherpos = it->getPosition();
			float dist = Math::Length(getPosition() - otherpos);

			if (dist > getRadius() + it->getRadius() && it->getName() != "Box")
				continue;

			if (it->getName() == "Goal")
			{
				Goal* test = dynamic_cast<Goal*>(it);
				if (!test->isCompleted())
				{
					test->setCompleted();

					mPlayerSound.setBuffer(*test->getSound());
					mPlayerSound.play();
				}
			}
			else if (it->getName() == "BasicEnemy")
			{
				if (getLevel()->getNumberOfCompletedGoals() < getLevel()->getNumberOfGoals())
				{
					//Enemy* test = (Enemy*)it;

					mPlayerSound.setBuffer(*mExplodeSound);
					mPlayerSound.play();

					getLevel()->resetLevel();
					return;
				}
			}
			else if (it->getName() == "Door")
			{
				Door* test = (Door*)it;

				if ((dist <= getRadius() + test->getRadius()/1.5) && !test->isOpen() && mKeyCount > 0)
				{
					--mKeyCount;
					test->open();

					mPlayerSound.setBuffer(*test->getSound());
					mPlayerSound.play();
				}
			}
			else if (it->getName() == "Key")
			{
				Key* test = (Key*)it;
				test->take();

				mPlayerSound.setBuffer(*test->getSound());
				mPlayerSound.play();

				++mKeyCount;
			}
			else if (it->getName() == "Box")
			{
				Box* test = (Box*)it;

				sf::Vector2f pushVec;
				if (test->getPenetration(getPosition(), getRadius(), pushVec))
				{
					pushVec /= 2.f;

					if (!test->push(pushVec))
						pushVec *= 2.f;

					move(pushVec * -1.f);
				}
			}
			else if (it->getName() == "Pit")
			{
				Pit* test = (Pit*)it;
				if (!test->isFull() && dist < getRadius() + it->getRadius() * 0.75 && getLevel()->getNumberOfCompletedGoals() < getLevel()->getNumberOfGoals())
				{
					mPlayerSound.setBuffer(*mExplodeSound);
					mPlayerSound.play();

					getLevel()->resetLevel();
					return;
				}
			}
		}

	setRotation(mState.Angle * Math::RAD2DEG);

	if ((mTick++ % 3 == 0) && std::abs(mState.Speed) >= 0.1)
	{
		sf::Vector2f x{
			std::cos(mState.Angle),
			std::sin(mState.Angle)
		};
		sf::Vector2f y{
			std::cos(mState.Angle + Math::PI2),
			std::sin(mState.Angle + Math::PI2)
		};

		auto& pos = getPosition();
		getLevel()->getParticleManager()->addParticle(TRACK_PARTICLE, pos - (x * 15.f) + (y * 8.f), {}, mState.Angle);
		getLevel()->getParticleManager()->addParticle(TRACK_PARTICLE, pos - (x * 15.f) - (y * 8.f), {}, mState.Angle);
	}
}

void Robot::update(const Timespan&)
{
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

void Robot::initialize()
{
	mTargetState.Speed = 0;
	mTargetState.Angle = getRotation() * Math::DEG2RAD;
	mState = mTargetState;

	if (!mExplodeSound)
		mExplodeSound = getLevel()->getEngine()->get<ResourceManager>().get<sf::SoundBuffer>("explode.wav");
}

bool Robot::serialize(OutputStream& stream) const
{
	stream.reserve(mCurProgram->getName().length() + sizeof(int32_t) + sizeof(State)*2);

	stream << mCurProgram->getName();
	stream << mState << mTargetState;

	return stream;
}
bool Robot::deserialize(InputStream& stream)
{
	std::string name;
	if (stream >> name)
	{
		mCurProgram = Program::createProgramming(name.c_str());

		stream >> mState >> mTargetState;
	}

	return stream;
}

const std::type_info& Robot::getType() const
{
	return typeid(Robot);
}
const std::string& Robot::getName() const
{
	static const std::string name = "Player";
	return name;
}

bool Robot::execute(const std::string& command)
{
	if (mCurProgram)
		return mCurProgram->execute(command, *this);

	return false;
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
