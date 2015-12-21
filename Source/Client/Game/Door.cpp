#include "Door.hpp"

#include "Level.hpp"
#include "../ParticleManager.hpp"

#include <Core/Engine.hpp>
#include <Core/InputStream.hpp>
#include <Core/Math.hpp>
#include <Core/OutputStream.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <random>

namespace
{
	static const ParticleManager::Particle OPEN_PARTICLE{
		std::chrono::seconds(1),

		{ 8, 8 },
		{ 255, 128, 0 },
		{ 255, 128, 0, 0},
		{ },
		1,
		2
	};
}

Door::Door() :
	mOpen(false)
{
	setRadius(75);
}
Door::~Door()
{

}

void Door::tick(const Timespan& dt) { }
void Door::update(const Timespan& dt) { }
void Door::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (mOpen)
		return;

	states.transform *= getTransform();

	float scale = getLevel()->getScale();
	sf::RectangleShape shape({
		scale, scale
	});

	shape.setOrigin(scale/2, scale/2);
	shape.setFillColor({
		255, 128, 0
	});

	target.draw(shape, states);

	sf::CircleShape hole(20);
	hole.setOrigin(20, 20);
	hole.move(0, -20);
	hole.setFillColor({ 0, 0, 0 });

	target.draw(hole, states);

	shape.setSize({ 20, 40 });
	shape.setOrigin(10, 20);
	shape.setFillColor({});

	shape.move(0, 15);

	target.draw(shape, states);
}

bool Door::serialize(OutputStream& stream) const
{
	stream.reserve(sizeof(mOpen));

	stream << mOpen;
	return stream;
}
bool Door::deserialize(InputStream& stream)
{
	return stream >> mOpen;
}

void Door::initialize()
{
	setRadius(getLevel()->getScale());

	if (!mOpen)
	{
		auto lpos = getPosition() / getLevel()->getScale();
		getLevel()->setBlocked(uint8_t(lpos.x), uint8_t(lpos.y));
	}

	if (!mDoorSound)
		mDoorSound = getLevel()->getEngine()->get<ResourceManager>().get<sf::SoundBuffer>("door.wav");
}

const std::type_info& Door::getType() const
{
	return typeid(Door);
}

const std::string& Door::getName() const
{
	static const std::string name = "Door";
	return name;
}

bool Door::isOpen() const
{
	return mOpen;
}

void Door::open()
{
	mOpen = true;

	auto pos = getPosition() / getLevel()->getScale();
	getLevel()->setBlocked(uint8_t(pos.x), uint8_t(pos.y), false);

	auto* pman = getParticleManager(true);

	std::random_device rand;
	std::uniform_real_distribution<float> dist(0, Math::PI * 2);
	std::uniform_real_distribution<float> dist2(0, 100);

	for (int i = 0; i < 100; ++i)
	{
		float ang = dist(rand);

		pman->addParticle(OPEN_PARTICLE,
			getPosition() + sf::Vector2f{ std::cos(dist(rand)) * dist2(rand), std::sin(dist(rand)) * dist2(rand) },
			{ std::cos(ang) * dist2(rand), std::sin(ang) * dist2(rand) }, dist(rand));
	}
}

ResourceManager::Sound& Door::getSound() { return mDoorSound; }
const ResourceManager::Sound& Door::getSound() const { return mDoorSound; }
