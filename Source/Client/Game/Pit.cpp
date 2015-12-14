#include "Pit.hpp"

#include "Level.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Pit::Pit() :
	mFull(false)
{
	setRadius(75);
}
Pit::~Pit()
{

}

void Pit::tick(const Timespan& dt) { }
void Pit::update(const Timespan& dt) { }
void Pit::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	float scale = getLevel()->getScale();
	sf::RectangleShape shape({
		scale, scale
	});

	shape.setOrigin(scale / 2, scale / 2);
	shape.setFillColor({
		29, 29, 29
	});

	target.draw(shape, states);

	if (!mFull)
		return;

	shape.setScale(0.75, 0.75);
	shape.setFillColor({
		196, 128, 12, 128
	});

	target.draw(shape, states);
}

bool Pit::serialize(char* data, size_t size) const
{
	return true;
}
bool Pit::deserialize(const char* data, size_t size)
{
	return true;
}

void Pit::initialize()
{
	auto lpos = getPosition() / getLevel()->getScale();
	getLevel()->setBlocked(uint8_t(lpos.x), uint8_t(lpos.y));
}

const std::string& Pit::getName() const
{
	static const std::string name = "Pit";
	return name;
}

void Pit::fill()
{
	mFull = true;

	auto lpos = getPosition() / getLevel()->getScale();
	getLevel()->setBlocked(uint8_t(lpos.x), uint8_t(lpos.y), false);
}