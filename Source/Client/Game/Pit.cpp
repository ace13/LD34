#include "Pit.hpp"

#include "Level.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Pit::Pit() :
	mFull(false)
{
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
		29, 29, 29, 96
	});

	for (int i = 0; i < 10; ++i)
	{
		target.draw(shape, states);
		shape.scale(0.9f, 0.9f);
	}

	if (!mFull)
		return;

	shape.setScale(0.7, 0.7);
	shape.setFillColor({
		95, 95, 95
	});

	target.draw(shape, states);

	shape.setScale(0.6, 0.6);
	shape.setFillColor({
		98, 64, 6
	});

	target.draw(shape, states);

	shape.setFillColor({
		95, 95, 95
	});
	shape.setScale(0.75, 0.75);
	shape.setSize({ 10, scale + 15 });
	shape.setOrigin(5, scale / 2 + 7.5);
	shape.setRotation(-45);

	target.draw(shape, states);
}

std::string Pit::serialize() const
{
	return "";
}
bool Pit::deserialize(const std::string& str)
{
	return true;
}

void Pit::initialize()
{
	setRadius(getLevel()->getScale() / 2);
}

const std::type_info& Pit::getType() const
{
	return typeid(Pit);
}

const std::string& Pit::getName() const
{
	static const std::string name = "Pit";
	return name;
}

bool Pit::isFull() const
{
	return mFull;
}

void Pit::fill()
{
	mFull = true;
}