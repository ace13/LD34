#include "GameState.hpp"

#include "../Game/Program.hpp"

#include <Core/Engine.hpp>
#include <Core/FileWatcher.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

GameState::GameState() :
	mNextExec(Clock::now()), mDot(0), mDir(-1)
{
	mRobot.setProgram(new BaseProgram());
}
GameState::~GameState()
{

}

void GameState::enter(sf::RenderTarget*)
{
	mTickResource = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("tick.wav");
	mTick.setBuffer(*mTickResource);

	auto& sman = getEngine().get<ScriptManager>();
	sman.registerHook("OnCommand", "void f(const string&in, const string&in)");

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
		if (mRobot.execute(mCurCommand))
		{
			std::string name = mRobot.getProgram()->getName(mCurCommand);

			mHistory.push_front(name);
			if (mHistory.size() > 5)
				mHistory.pop_back();

			getEngine().get<ScriptManager>().runHook<const std::string*, const std::string*>("OnCommand", &mCurCommand, &name);
			mTick.setVolume(50);
		}
		else
			mTick.setVolume(20);

		mTick.play();

		mCurCommand.clear();

		mNextExec = Clock::now() + std::chrono::seconds(1);

		if (mDir > 0)
			mDir = -1;
		else
			mDir = 1;
	}

	mRobot.tick(dt);
}
void GameState::update(const Timespan& dt)
{
	mDot += std::min(std::max(mDir * Time::Seconds(dt), -1.f), 1.f);
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
	sf::CircleShape dot(10);

	dot.setPosition(target.getView().getSize().x - 25 - mDot * 100, 5);
	float scale = std::abs(0.5 - mDot);
	dot.scale((1 - scale) / 2, 0.5 + scale);

	target.draw(dot);

	sf::Text commandString(mCurCommand, getEngine().get<sf::Font>());

	auto bounds = commandString.getLocalBounds();
	commandString.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	commandString.setPosition(sf::Vector2f(target.getView().getSize().x, 0) - sf::Vector2f(50, -bounds.height));

	target.draw(commandString);

	commandString.setCharacterSize(12);
	commandString.setPosition(target.getView().getSize().x - 75, 30);

	for (auto& it : mHistory)
	{
		commandString.setString(it);
		auto rect = commandString.getLocalBounds();
		commandString.setOrigin(rect.width / 2, rect.height / 2);

		target.draw(commandString);

		auto col = commandString.getColor();
		col.a -= 50;
		commandString.setColor(col);

		commandString.move(0, rect.height + 5);
	}
}