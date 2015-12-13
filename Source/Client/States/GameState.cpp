#include "GameState.hpp"

#include "../Application.hpp"
#include "../Game/Entity.hpp"
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

}
GameState::~GameState()
{

}

void GameState::enter(sf::RenderTarget* rt)
{
	sf::View tmp = rt->getView();
	tmp.zoom(0.5);
	rt->setView(tmp);

	mLevel.setEngine(&getEngine());

	mTick = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("tick.wav");
	mTickFail = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("tick-fail.wav");
	mTickSucceed = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("tick-success.wav");

	auto& sman = getEngine().get<ScriptManager>();
	sman.registerHook("OnCommand", "void f(const string&in, const string&in)");
	sman.setPreLoadCallback(Entity::preLoadInject);
	sman.getEngine()->SetUserData(&mLevel, 0x1EE7);

	FileWatcher::recurseDirectory("Game", mScripts, "*.as");

	mLevel.loadFromFile("Test4");

	for (auto& script : mScripts)
	{
		sman.loadFromFile(script);
	}

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	
	/*
	mLevel.getPlayer().setPosition({
		200,200
	});

	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({ 32, 32 });
	for (int x = 0; x < 32; ++x)
	{
		for (int y = 0; y < 32; ++y)
		{
			if ((x > 0 && x < 31) && (y != 0 && y != 31))
				continue;

			mLevel.setBlocked(x, y);
		}
	}

	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color::Black);
	mLevel.setBackgroundColor(sf::Color(0x4A, 0x70, 0x23));
	mLevel.setForegroundColor(sf::Color(0x96, 0x6F, 0x33));

	auto* ent = Entity::createForScript(sman.getEngine()->GetModule("game\\robot.as"), "Robot");
	ent->setPosition(500, 500);
	mLevel.addEntity(ent);

	mLevel.bakeFile("Game\\robot.as");

	mLevel.saveToFile("Test4");
	*/
}
void GameState::exit(sf::RenderTarget*)
{
	auto& sman = getEngine().get<ScriptManager>();
	sman.clearPreLoadCallback();
	sman.getEngine()->SetUserData(nullptr, 0x64EE);
	
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
	if (Clock::now() >= mNextExec)
	{
		std::string name = mLevel.getPlayer().getProgram()->getName(mCurCommand);
		if (mLevel.getPlayer().execute(mCurCommand) || !name.empty())
		{
			mHistory.push_front(name);
			if (mHistory.size() > 5)
				mHistory.pop_back();

			if (!mCurCommand.empty())
				getEngine().get<ScriptManager>().runHook<const std::string*, const std::string*>("OnCommand", &mCurCommand, &name);
			
			if (name != "NOP")
			{
				mTickSound.setBuffer(*mTickSucceed);
				mTickSound.setVolume(25);
			}
			else
			{
				mTickSound.setBuffer(*mTickFail);
				mTickSound.setVolume(40);
			}
		}
		else
		{
			mTickSound.setBuffer(*mTick);
			mTickSound.setVolume(20);
		}

		mTickSound.play();

		mCurCommand.clear();

		mNextExec = Clock::now() + std::chrono::seconds(1);

		if (mDir > 0)
		{
			mDir = -1;
			mOff = 100;
		}
		else
		{
			mDir = 1;
			mOff = 0;
		}
	}

	mLevel.tick(dt);
}
void GameState::update(const Timespan& dt)
{
	mDot = 1-Time::Seconds(mNextExec - Clock::now());

	mPreParticles.update(dt);
	mPostParticles.update(dt);
}
void GameState::draw(sf::RenderTarget& target)
{
	auto view = target.getView();
	view.move((mLevel.getPlayer().getPosition() - view.getCenter()) * 0.001f);

	target.setView(view);
	target.clear(mLevel.getOutsideColor());

	mLevel.drawBackface(target);

	mPreParticles.draw(target);

	mLevel.draw(target);

	mPostParticles.draw(target);

}
void GameState::drawUI(sf::RenderTarget& target)
{
	sf::CircleShape dot(10);

	dot.setPosition(target.getView().getSize().x - 25 - mDot * 100 * mDir - mOff, 5);
	float scale = std::abs(0.5f - mDot);
	dot.scale((1 - scale) / 2, 0.5f + scale);

	target.draw(dot);

	sf::Text commandString(mCurCommand, getEngine().get<sf::Font>());

	auto bounds = commandString.getLocalBounds();
	commandString.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
	commandString.setPosition(sf::Vector2f(target.getView().getSize().x, 0) - sf::Vector2f(50, -bounds.height));

	target.draw(commandString);

	commandString.setCharacterSize(12);
	commandString.setPosition(target.getView().getSize().x - 75, 50);

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