#include "MenuState.hpp"
#include "../Menus/MenuPage.hpp"

MenuState::MenuState(MenuPage* page)
{
	if (page)
		mPages.push_back(page);
}
MenuState::~MenuState()
{

}

void MenuState::event(const sf::Event&)
{

}
void MenuState::tick(const Timespan&)
{

}
void MenuState::update(const Timespan&)
{

}
void MenuState::draw(sf::RenderTarget&)
{

}
void MenuState::drawUI(sf::RenderTarget&)
{

}