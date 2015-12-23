#include "GameState.hpp"

#include "../Application.hpp"
#include "../ResourceManager.hpp"

#include "../Game/Entity.hpp"
#include "../Game/Goal.hpp"
#include "../Game/Program.hpp"
#include "../Game/ScriptEntity.hpp"

#include <Core/Engine.hpp>
#include <Core/FileWatcher.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

namespace
{
	ResourceManager::Sound* explosionPrecache;
}

GameState::GameState() :
	mEnded(false), mEndTimeout(0),
	mNextExec(Clock::now()), mDot(0), mDir(-1)
{
	explosionPrecache = new	ResourceManager::Sound;
}
GameState::~GameState()
{
	
}

void GameState::enter(sf::RenderTarget* rt)
{
	mRT = rt;

	sf::View tmp = mRT->getView();
	tmp.zoom(0.4f);
	mRT->setView(tmp);

	mLevel.setEngine(&getEngine());
	mLevel.setParticleManager(&mPreParticles);
	mLevel.setParticleManager(&mPostParticles, true);

	*explosionPrecache = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("explode.wav");
	mTick = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("tick.wav");
	mTickFail = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("tick-fail.wav");
	mTickSucceed = getEngine().get<ResourceManager>().get<sf::SoundBuffer>("tick-success.wav");

	auto& sman = getEngine().get<ScriptManager>();
	sman.getEngine()->RegisterGlobalFunction("void LoadLevel(const string&in)", asMETHOD(GameState, loadLevel), asCALL_THISCALL_ASGLOBAL, this);
	sman.registerHook("OnCommand", "void f(const string&in, const string&in)");
	sman.registerHook("OnLevelEnd", "void f()");
	sman.addPreLoadCallback("ScriptEntity", ScriptEntity::preLoadInject);
	sman.getEngine()->SetUserData(&mLevel, 0x1EE7);

	FileWatcher::recurseDirectory("Game", mScripts, "*.as");

	for (auto& script : mScripts)
	{
		sman.loadFromFile(script);
	}

	loadLevel("Tutorial1.lvl");
}
void GameState::exit(sf::RenderTarget*)
{
	delete explosionPrecache;

	auto& sman = getEngine().get<ScriptManager>();
	sman.removePreLoadCallback("ScriptEntity");
	sman.getEngine()->SetUserData(nullptr, 0x64EE);
	
	for (auto& script : mScripts)
		sman.unload(script);
}

void GameState::event(const sf::Event& ev)
{
	if (!mLevel)
		return;

	if (ev.type == sf::Event::TextEntered)
	{
		if (ev.text.unicode == '0' || ev.text.unicode == '1')
		{
			mCurCommand += char(ev.text.unicode);
		}
	}
	else if (ev.type == sf::Event::KeyReleased && ev.key.code == sf::Keyboard::R)
		mLevel.resetLevel();
}
void GameState::tick(const Timespan& dt)
{
	auto* ply = mLevel.getPlayer();

	if (Clock::now() >= mNextExec)
	{
		std::string name;
		if (ply && ply->getProgram())
		{
			name = ply->getProgram()->getName(mCurCommand);

			if (ply->execute(mCurCommand) || !name.empty())
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
		}

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

	if (mLevel && !mEnded && mLevel.getNumberOfCompletedGoals() >= mLevel.getNumberOfGoals())
	{
		mEnded = true;
		mEndTimeout = 2.5;
	}
	else if (mEnded && mEndTimeout <= 0)
	{
		getEngine().get<ScriptManager>().runHook("OnLevelEnd");
		ply = nullptr;
	}

	if (ply)
	{
		auto view = mRT->getView();
		view.move((ply->getPosition() - view.getCenter()) * 0.01f);
		mRT->setView(view);
	}
}
void GameState::update(const Timespan& dt)
{
	if (mEnded)
	{
		float dtSec = Time::Seconds(dt);
		mEndTimeout = std::max(mEndTimeout - dtSec, 0.f);
	}

	mDot = 1-Time::Seconds(mNextExec - Clock::now());

	mPreParticles.update(dt);
	mPostParticles.update(dt);
	mLevel.update(dt);
}
void GameState::draw(sf::RenderTarget& target)
{
	auto view = target.getView();
	auto before = view;

	if (mEnded)
	{
		view.zoom(5 / (mEndTimeout * 2));
		view.rotate((5- (mEndTimeout * 2)) * 22.5f);
	}

	target.setView(view);
	target.clear(mLevel.getOutsideColor());

	mLevel.drawBackface(target);

	mPreParticles.draw(target);

	mLevel.draw(target);

	mPostParticles.draw(target);

	if (mEnded)
	{
		target.setView(before);

		sf::RectangleShape shape(before.getSize());
		shape.setOrigin(before.getSize() / 2.f);
		shape.setPosition(before.getCenter());

		shape.setFillColor({
			0,0,0,
			uint8_t(0 - 255 * (mEndTimeout / 2.5))
		});

		target.draw(shape);
	}

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

void GameState::loadLevel(const std::string& name)
{
	mHistory.clear();
	mCurCommand.clear();
	mPreParticles.clear();
	mPostParticles.clear();

	mLevel.loadFromFile(name);

	mEnded = false;
	mEndTimeout = 0;
}
