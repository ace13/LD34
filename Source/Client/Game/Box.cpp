#include "Box.hpp"
#include "Pit.hpp"
#include "Level.hpp"

#include <Core/Math.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Box::Box()
{

}
Box::~Box()
{

}

void Box::tick(const Timespan& dt)
{

}
void Box::update(const Timespan& dt)
{

}
void Box::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	float scale = getLevel()->getScale() * 0.75;

	sf::RectangleShape shape({
		scale, scale
	});
	shape.setOrigin(scale / 2, scale / 2);
	
	shape.setFillColor({
		0, 0, 0
	});

	target.draw(shape, states);
	
	shape.setScale(0.95, 0.95);
	shape.setFillColor({
		190, 190, 190
	});

	target.draw(shape, states);

	shape.setScale(0.8, 0.8);
	shape.setFillColor({
		196, 128, 12
	});

	target.draw(shape, states);

	shape.setFillColor({
		190, 190, 190
	});
	shape.setScale(1, 1);
	shape.setSize({ 10, scale + 15 });
	shape.setOrigin(5, scale / 2 + 7.5);
	shape.setRotation(-45);

	target.draw(shape, states);
}

bool Box::serialize(char* data, size_t size) const
{
	return true;
}
bool Box::deserialize(const char* data, size_t size)
{
	return true;
}

void Box::initialize()
{
	setRadius(75);
}

const std::string& Box::getName() const
{
	static const std::string name = "Box";
	return name;
}

void Box::push(float dir, float amount)
{
	auto change = sf::Vector2f(std::cos(dir * Math::DEG2RAD), std::sin(dir * Math::DEG2RAD));
	auto before = getPosition();

	move(change * amount);

	if (Math::Length(getPosition() - before) < amount / 2)
	{
		auto lpos = (getPosition() + change * (getLevel()->getScale()/2)) / getLevel()->getScale();

		std::list<Entity*> ents;
		if (getLevel()->findEntities(ents, uint8_t(lpos.x), uint8_t(lpos.y)))
		{
			for (auto& it : ents)
				if (it->getName() == "Pit")
				{
					Pit* pit = (Pit*)it;

					float dist = Math::Length(getPosition() - pit->getPosition());

					if (dist < (getRadius() + pit->getRadius() + amount*5))
					{
						pit->fill();

						getLevel()->removeEntity(this);
						
					}
				}
		}
	}
}