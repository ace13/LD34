#pragma once

#include "IState.hpp"

class MenuState : public IState
{
public:
	MenuState();
	~MenuState();

	virtual void event(const sf::Event&);
	virtual void tick(const Timespan&);
	virtual void update(const Timespan&);
	virtual void draw(sf::RenderTarget&);
	virtual void drawUI(sf::RenderTarget&);

private:

};