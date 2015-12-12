#include "GameState.hpp"

#include "../Game/Program.hpp"

#include <Core/Engine.hpp>
#include <Core/FileWatcher.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

GameState::GameState() :
	mNextExec(Clock::now())
{
	mRobot.setProgram(new BaseProgram());
}
GameState::~GameState()
{

}

void GameState::enter(sf::RenderTarget*)
{
	auto& sman = getEngine().get<ScriptManager>();

	FileWatcher::recurseDirectory("Game", mScripts, "*.as");

	for (auto& script : mScripts)
		sman.loadFromFile(script);
}
void GameState::exit(sf::RenderTarget*)
{
	auto& sman = getEngine().get<ScriptManager>();
	
	for (auto& script : mScripts)
		sman.unload(script);
}

void GameState::event(const sf::Event& ev)
{
	if (ev.type == sf::Event::TextEntered)
	{
		if (ev.text.unicode == '0' || ev.text.unicode == '1')
		{
			mCurCommand += char(ev.text.unicode);
		}
	}
}
void GameState::tick(const Timespan& dt)
{
	if (Clock::now() > mNextExec)
	{
		mRobot.execute(mCurCommand);
		mCurCommand.clear();

		mNextExec = Clock::now() + std::chrono::seconds(1);
	}

	mRobot.tick(dt);
}
void GameState::update(const Timespan& dt)
{

}
void GameState::draw(sf::RenderTarget& target)
{
	auto view = target.getView(), oldView = view;
	view.zoom(0.5);
	target.setView(view);

	target.draw(mRobot);

	target.setView(oldView);
}
void GameState::drawUI(sf::RenderTarget& target)
{
	sf::Text commandString(mCurCommand, getEngine().get<sf::Font>());

	auto bounds = commandString.getLocalBounds();
	commandString.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	commandString.setPosition(sf::Vector2f(target.getView().getSize().x, 0) - sf::Vector2f(bounds.width + 10, -bounds.height));

	target.draw(commandString);


}