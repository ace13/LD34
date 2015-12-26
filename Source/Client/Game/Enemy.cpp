#include "Enemy.hpp"
#include "Level.hpp"

#include "../ParticleManager.hpp"

#include <Core/InputStream.hpp>
#include <Core/Math.hpp>
#include <Core/OutputStream.hpp>

#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cstring>

namespace
{
	static const ParticleManager::Particle TRACK_PARTICLE{
		std::chrono::seconds(4),
		{ 0, 0, 8, 16 },
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
	setRadius(35);
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

		if (!move(newPos))
		{
			mCurState = State_Turning;
			mTargetAng = std::fmod((getRotation() + 180.f), 360.f);
		}
		else if ((mTick++ % 3 == 0))
		{
			auto* particles = getParticleManager();

			sf::Vector2f x{
				std::cos(getRotation() * Math::DEG2RAD),
				std::sin(getRotation() * Math::DEG2RAD)
			};
			sf::Vector2f y{
				std::cos(getRotation() * Math::DEG2RAD + Math::PI2),
				std::sin(getRotation() * Math::DEG2RAD + Math::PI2)
			};

			auto& pos = getPosition();
			particles->addParticle(TRACK_PARTICLE, pos - (x * 5.f) + (y * 16.f), {}, getRotation() * Math::DEG2RAD);
			particles->addParticle(TRACK_PARTICLE, pos - (x * 5.f) - (y * 16.f), {}, getRotation() * Math::DEG2RAD);
		}
	}
}
void Enemy::update(const Timespan&)
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

bool Enemy::serialize(OutputStream& stream) const
{
	stream.reserve(sizeof(State) + sizeof(float));

	stream << mCurState;
	if (mCurState == State_Turning)
		stream << mTargetAng;

	return stream;
}
bool Enemy::deserialize(InputStream& stream)
{
	if (!stream >> mCurState)
		return false;

	if (mCurState == State_Turning)
		stream >> mTargetAng;

	return stream;
}

void Enemy::initialize()
{

}

const std::type_info& Enemy::getType() const
{
	return typeid(Enemy);
}

const std::string& Enemy::getName() const
{
	static const std::string name = "BasicEnemy";
	return name;
}
