#pragma once

#include "Entity.hpp"

class Goal : public Entity
{
public:
	Goal();
	~Goal();
	
	void tick(const Timespan& dt);
	void update(const Timespan& dt);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	void setCompleted(bool completed = true);

private:
};