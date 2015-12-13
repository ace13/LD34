#pragma once

#include "Door.hpp"

#include "Level.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Door::Door() :
	mOpen(false)
{

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
}

bool Door::serialize(char* data, size_t size) const { return true; }
bool Door::deserialize(const char* data, size_t size) { return true; }

void Door::initialize()
{
	setRadius(getLevel()->getScale());
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
}