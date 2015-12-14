#include "Enemy.hpp"
#include "Level.hpp"

#include "../ParticleManager.hpp"

#include <Core/Math.hpp>

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace
{
	static const ParticleManager::Particle TRACK_PARTICLE{
		std::chrono::seconds(4),
		{ 4, 8 },
		{ 64, 64, 64, 197 },
		{ 64, 64, 64, 0 },
		{ 0, 0 },
		0, 0
	};
}


Enemy::Enemy() :
	mCurState(State_Moving),
	mTargetAng(0),
	mSpeed(150),
	mTick(0)
{
	setRadius(30);
}
Enemy::~Enemy()
{

}

void Enemy::tick(const Timespan& span)
{
	float dt = Time::Seconds(span);

	if (mCurState == State_Turning)
	{
		rotate((mTargetAng - getRotation()) * dt * 4);

		if (std::abs(mTargetAng - getRotation()) < 0.5)
		{
			setRotation(mTargetAng);
			mCurState = State_Moving;
		}
	}
	else
	{
		auto newPos = sf::Vector2f(cos(getRotation() * Math::DEG2RAD), sin(getRotation() * Math::DEG2RAD)) * mSpeed * dt;

		auto checkPos = (getPosition() + newPos * getRadius()) / getLevel()->getScale();
		if (getLevel()->isBlocked(uint8_t(checkPos.x), uint8_t(checkPos.y)))
		{
			mCurState = State_Turning;
			mTargetAng = std::fmod((getRotation() + 180.f), 360.f);
		}
		else
		{
			move(newPos);

			if ((mTick++ % 3 == 0))
			{
				auto* particles = getParticleManager();

				sf::Vector2f x{
					cos(getRotation() * Math::DEG2RAD),
					sin(getRotation() * Math::DEG2RAD)
				};
				sf::Vector2f y{
					cos(getRotation() * Math::DEG2RAD + Math::PI2),
					sin(getRotation() * Math::DEG2RAD + Math::PI2)
				};

				auto& pos = getPosition();
				particles->addParticle(TRACK_PARTICLE, pos - (x * 5.f) + (y * 16.f), {}, getRotation() * Math::DEG2RAD);
				particles->addParticle(TRACK_PARTICLE, pos - (x * 5.f) - (y * 16.f), {}, getRotation() * Math::DEG2RAD);
			}
		}
	}
}
void Enemy::update(const Timespan& dt)
{

}
void Enemy::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	sf::ConvexShape shape(5);
	shape.setPoint(0, { 20, 0 });
	shape.setPoint(1, { 0, 10 });
	shape.setPoint(2, { -20, 10 });
	shape.setPoint(3, { -20, -10 });
	shape.setPoint(4, { 0, -10 });

	shape.setFillColor(sf::Color::Red);
	shape.setScale(2.5, 2.5);

	target.draw(shape, states);
}

bool Enemy::serialize(char* data, size_t size) const
{
	return true;
}
bool Enemy::deserialize(const char* data, size_t size)
{
	return true;
}

void Enemy::initialize()
{

}

const std::string& Enemy::getName() const
{
	static const std::string name = "BasicEnemy";
	return name;
}