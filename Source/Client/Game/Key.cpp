#include "Key.hpp"
#include "Level.hpp"

#include <Core/Engine.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

Key::Key() :
	mTime(0)
{
	setRadius(35);
}
Key::~Key() { }

void Key::tick(const Timespan& dt) { }
void Key::update(const Timespan& dt)
{
	mTime += Time::Seconds(dt);
}
void Key::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (getRadius() < 0)
		return;

	states.transform *= getTransform();

	sf::Sprite key(*mKeyTexture);
	auto size = sf::Vector2f(key.getTexture()->getSize());

	key.setOrigin(size / 2.f);
	key.setScale(0.2, 0.2);

	key.move((std::cos(mTime * 1.5)) * -10, (std::sin(mTime * 2)) * 10);
	key.rotate(std::cos(mTime*3) * -5);

	target.draw(key, states);
}

bool Key::serialize(OutputStream&) const
{
	return true;
}
bool Key::deserialize(InputStream&)
{
	return true;
}

void Key::initialize() 
{
	if (!mKeyTexture)
		mKeyTexture = getLevel()->getEngine()->get<ResourceManager>().get<sf::Texture>("key.png");
	if (!mKeySound)
		mKeySound = getLevel()->getEngine()->get<ResourceManager>().get<sf::SoundBuffer>("key.wav");
}

const std::type_info& Key::getType() const
{
	return typeid(Key);
}
const std::string& Key::getName() const
{
	static const std::string name = "Key";
	return name;
}

void Key::take()
{
	setRadius(-200);
}

ResourceManager::Sound& Key::getSound() { return mKeySound; }
const ResourceManager::Sound& Key::getSound() const { return mKeySound; }
