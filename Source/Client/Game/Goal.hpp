#pragma once

#include "Entity.hpp"

class Goal : public Entity
{
public:
	Goal();
	~Goal();
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:

};