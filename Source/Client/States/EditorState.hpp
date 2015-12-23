#pragma once

#include "IState.hpp"

class EditorState : public IState
{
public:
	EditorState();
	~EditorState();

	virtual void enter(sf::RenderTarget*);
	virtual void exit(sf::RenderTarget*);

	virtual void event(const sf::Event&);
	virtual void tick(const Timespan&);
	virtual void update(const Timespan&);
	virtual void draw(sf::RenderTarget&);
	virtual void drawUI(sf::RenderTarget&);

private:

};
