#include "Pit.hpp"

#include "Level.hpp"

#include <Core/InputStream.hpp>
#include <Core/OutputStream.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Pit::Pit() :
	mFull(false)
{
}
Pit::~Pit()
{

}

void Pit::tick(const Timespan&) { }
void Pit::update(const Timespan&) { }
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

bool Pit::serialize(OutputStream& stream) const
{
	return stream << mFull;
}
bool Pit::deserialize(InputStream& stream)
{
	bool b;
	if (stream >> b)
		mFull = b;
	return stream;
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

bool Pit::isBackground() const
{
	return true;
}

bool Pit::isFull() const
{
	return mFull;
}

void Pit::fill()
{
	mFull = true;
}
