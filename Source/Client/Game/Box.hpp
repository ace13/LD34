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

	virtual bool serialize(OutputStream&) const;
	virtual bool deserialize(InputStream&);

	virtual void initialize();

	virtual const std::type_info& getType() const;
	virtual const std::string& getName() const;

	bool getPenetration(const sf::Vector2f& pos, float radius, sf::Vector2f& penetrationVec);
	void push(const sf::Vector2f& amount);

private:
};
