#pragma once

#include "MenuPage.hpp"

class InfoMenu : public MenuPage
{
public:
	InfoMenu();
	~InfoMenu();

	virtual void event(const sf::Event& ev);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:

};