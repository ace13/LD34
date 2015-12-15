#pragma once

#include "Entity.hpp"

class Box : public Entity
{
public:
	Box();
	~Box();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual bool serialize(char* data, size_t size) const;
	virtual bool deserialize(const char* data, size_t size);

	virtual void initialize();

	virtual const std::string& getName() const;

	bool getPenetration(const sf::Vector2f& pos, float radius, sf::Vector2f& penetrationVec);
	void push(const sf::Vector2f& amount);

private:
};
