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
	mRobot.setProgram(new BaseProgram());
}
GameState::~GameState()
{

}

void GameState::enter(sf::RenderTarget* rt)
{
	sf::View tmp = rt->getView();
	tmp.zoom(0.5);
	rt->setView(tmp);

	mRobot.passParticleManager(&mPreParticles);

	mTickResource = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("tick.wav");
	mTick.setBuffer(*mTickResource);

	auto& sman = getEngine().get<ScriptManager>();
	sman.registerHook("OnCommand", "void f(const string&in, const string&in)");
	sman.setPreLoadCallback(Entity::preLoadInject);
	Entity::CurGameState = this;

	FileWatcher::recurseDirectory("Game", mScripts, "*.as");

	for (auto& script : mScripts)
		sman.loadFromFile(script);

}
void GameState::exit(sf::RenderTarget*)
{
	auto& sman = getEngine().get<ScriptManager>();
	sman.clearPreLoadCallback();
	
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
		std::string name = mRobot.getProgram()->getName(mCurCommand);
		if (mRobot.execute(mCurCommand) || !name.empty())
		{
			mHistory.push_front(name);
			if (mHistory.size() > 5)
				mHistory.pop_back();

			if (!mCurCommand.empty())
				getEngine().get<ScriptManager>().runHook<const std::string*, const std::string*>("OnCommand", &mCurCommand, &name);
			mTick.setVolume(50);
		}
		else
			mTick.setVolume(20);

		mTick.play();

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

	mRobot.tick(dt);
	for (auto& it : mEntities)
		it->tick(dt);
}
void GameState::update(const Timespan& dt)
{
	mDot = 1-Time::Seconds(mNextExec - Clock::now());

	mPreParticles.update(dt);
	mPostParticles.update(dt);
	for (auto& it : mEntities)
		it->update(dt);
}
void GameState::draw(sf::RenderTarget& target)
{
	auto view = target.getView();
	view.move((mRobot.getPosition() - view.getCenter()) * 0.001f);

	target.setView(view);
	target.clear(sf::Color(0x4A, 0x70, 0x23));

	mPreParticles.draw(target);

	for (auto& it : mEntities)
		it->draw(target, {});

	target.draw(mRobot);

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

void GameState::injectEntity(Entity* ent)
{
	ent->addRef();

	mEntities.push_back(ent);
}