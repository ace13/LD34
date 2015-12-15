#include "Box.hpp"
#include "Pit.hpp"
#include "Level.hpp"

#include <Core/Math.hpp>

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <algorithm>

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

	float scale = getRadius();

	sf::RectangleShape shape({
		scale * 2, scale * 2
	});
	shape.setOrigin(scale, scale);
	
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
	setRadius(getLevel()->getScale() / 2);
}

const std::string& Box::getName() const
{
	static const std::string name = "Box";
	return name;
}

bool Box::getPenetration(const sf::Vector2f& pos, float radius, sf::Vector2f& out)
{
	sf::FloatRect aabb{
		getPosition().x-getRadius(),
		getPosition().y-getRadius(),
		getRadius()*2,
		getRadius()*2
	};

	sf::Vector2f closest{
		std::max(aabb.left, std::min(pos.x, aabb.left + aabb.width)),
		std::max(aabb.top, std::min(pos.y, aabb.top + aabb.height))
	};

	auto diff = closest - pos,
		ndiff = Math::Normalized(diff);

	if (Math::Length(diff) > radius)
		return false;

	static const sf::Vector2f dirs[]= {
		{ 1, 0 },
		{ 0, 1 },
		{ -1, 0 },
		{ 0, -1 }
	};

	float highestDot = -1;
	for (auto& dir : dirs)
	{
		float dot = Math::Dot(ndiff, dir);
		if (dot > highestDot)
		{
			highestDot = dot;

			out = dir * Math::Length(dir * diff);
		}
	}

	return true;
}

void Box::push(const sf::Vector2f& amount)
{
	auto change = Math::Normalized(amount);
	auto before = getPosition();

	move(amount);

	if (Math::Length(getPosition() - before) < Math::Length(amount) / 2)
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

					if (dist < (getRadius() + pit->getRadius() + Math::Length(amount) * 2))
					{
						pit->fill();

						getLevel()->removeEntity(this);
					}
				}
		}
	}
}
