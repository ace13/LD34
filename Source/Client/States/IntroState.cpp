#include "IntroState.hpp"
#include "MenuState.hpp"

#include "../Menus/MainMenu.hpp"

IntroState::IntroState() : 
	mTime(0)
{

}
IntroState::~IntroState()
{

}

void IntroState::event(const sf::Event&)
{

}
void IntroState::tick(const Timespan&)
{

}
void IntroState::update(const Timespan& dt)
{
	mTime += dt;

	if (mTime > std::chrono::seconds(15))
		getStateMachine()->changeState(new MenuState(new MainMenuPage()), true);
}
void IntroState::draw(sf::RenderTarget& rt)
{
	
}
void IntroState::drawUI(sf::RenderTarget& rt)
{

}