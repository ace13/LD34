#pragma once

#include "IState.hpp"

#include <vector>

class MenuPage;

class MenuState : public IState
{
public:
	MenuState(MenuPage* startingPage);
	~MenuState();

	void pushPage(MenuPage* page);
	void popPage();
	MenuPage* peekPage();
	const MenuPage* peekPage() const;

	virtual void event(const sf::Event&);
	virtual void tick(const Timespan&);
	virtual void update(const Timespan&);
	virtual void draw(sf::RenderTarget&);
	virtual void drawUI(sf::RenderTarget&);

private:
	std::vector<MenuPage*> mPages;
};