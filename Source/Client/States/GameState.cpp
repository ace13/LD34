#include "GameState.hpp"

#include "../Application.hpp"
#include "../Game/Entity.hpp"
#include "../Game/Goal.hpp"
#include "../Game/Program.hpp"


#include <Core/Engine.hpp>
#include <Core/FileWatcher.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

GameState::GameState() :
	mEnded(false), mEndTimeout(0),
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
	sman.getEngine()->RegisterGlobalFunction("void LoadLevel(const string&in)", asMETHOD(GameState, loadLevel), asCALL_THISCALL_ASGLOBAL, this);
	sman.registerHook("OnCommand", "void f(const string&in, const string&in)");
	sman.registerHook("OnLevelEnd", "void f()");
	sman.setPreLoadCallback(Entity::preLoadInject);
	sman.getEngine()->SetUserData(&mLevel, 0x1EE7);

	FileWatcher::recurseDirectory("Game", mScripts, "*.as");

	for (auto& script : mScripts)
	{
		sman.loadFromFile(script);
	}

	//loadLevel("Tutorial1.lvl");

	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color::Black);
	mLevel.setBackgroundColor(sf::Color(0x4A, 0x70, 0x23));
	mLevel.setForegroundColor(sf::Color(0x96, 0x6F, 0x33));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		3,
		5
	});

	mLevel.setBlocked(0, 0);
	mLevel.setBlocked(1, 0);
	mLevel.setBlocked(2, 0);
	mLevel.setBlocked(0, 1);
	mLevel.setBlocked(2, 1);
	mLevel.setBlocked(0, 2);
	mLevel.setBlocked(2, 2);
	mLevel.setBlocked(0, 3);
	mLevel.setBlocked(2, 3);
	mLevel.setBlocked(0, 4);
	mLevel.setBlocked(1, 4);
	mLevel.setBlocked(2, 4);

	mLevel.getPlayer().setPosition(225, 525);
	mLevel.getPlayer().setRotation(-90);
	mLevel.getPlayer().initialize();

	Entity* ent = Entity::createFromType("Goal", nullptr, 0);
	ent->setPosition(225, 225);
	mLevel.addEntity(ent);

	// Tutorial1.lvl
	/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color::Black);
	mLevel.setBackgroundColor(sf::Color(0x4A, 0x70, 0x23));
	mLevel.setForegroundColor(sf::Color(0x96, 0x6F, 0x33));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		3,
		5
	});

	mLevel.setBlocked(0, 0);
	mLevel.setBlocked(1, 0);
	mLevel.setBlocked(2, 0);
	mLevel.setBlocked(0, 1);
	mLevel.setBlocked(2, 1);
	mLevel.setBlocked(0, 2);
	mLevel.setBlocked(2, 2);
	mLevel.setBlocked(0, 3);
	mLevel.setBlocked(2, 3);
	mLevel.setBlocked(0, 4);
	mLevel.setBlocked(1, 4);
	mLevel.setBlocked(2, 4);

	mLevel.getPlayer().setPosition(225, 525);
	mLevel.getPlayer().setRotation(-90);
	mLevel.getPlayer().initialize();

	Entity* ent = Entity::createFromType("Goal", nullptr, 0);
	ent->setPosition(225, 225);
	mLevel.addEntity(ent);

	mLevel.bakeFile("level\\tutorial1.as");

	mLevel.saveToFile("Tutorial1.lvl");
	*/
	

	/*
	
	mLevel.getPlayer().setPosition({
		225,225
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

	*

	/*
	auto* ent = Entity::createForScript(sman.getEngine()->GetModule("game\\robot.as"), "Robot");
	ent->setPosition(500, 500);
	mLevel.addEntity(ent);
	*/

	//mLevel.bakeFile("Game\\robot.as");

	
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
	if (!mLevel)
		return;

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
		auto& ply = mLevel.getPlayer();

		std::string name;
		if (ply.getProgram())
		{
			name = ply.getProgram()->getName(mCurCommand);

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

		mLevel.clearLevel();
		mHistory.clear();
		mCurCommand.clear();

		mEnded = false;
		mEndTimeout = 0;
	}
}
void GameState::update(const Timespan& dt)
{
	if (mEnded)
	{
		float dtSec = Time::Seconds(dt);
		mEndTimeout -= dtSec;
	}

	mDot = 1-Time::Seconds(mNextExec - Clock::now());

	mPreParticles.update(dt);
	mPostParticles.update(dt);
	mLevel.update(dt);
}
void GameState::draw(sf::RenderTarget& target)
{
	auto view = target.getView();
	view.move((mLevel.getPlayer().getPosition() - view.getCenter()) * 0.001f);

	if (mEnded)
	{
		view.zoom(5 / (mEndTimeout * 2));
		view.rotate((5- (mEndTimeout * 2)) * 22.5);
	}

	target.setView(view);
	target.clear(mLevel.getOutsideColor());

	mLevel.drawBackface(target);

	mPreParticles.draw(target);

	mLevel.draw(target);

	mPostParticles.draw(target);

	if (mEnded)
	{
		view.zoom((mEndTimeout * 2) /5);
		view.rotate(((mEndTimeout * 2) -5) * 22.5);
		target.setView(view);

		sf::RectangleShape shape(view.getSize());
		shape.setOrigin(view.getSize() / 2.f);
		shape.setPosition(view.getCenter());

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
	if (mLevel.loadFromFile(name))
		for (auto& file : mLevel.getFiles())
		{
			if (file.substr(file.size() - 3) == ".as")
				getEngine().get<ScriptManager>().loadFromStream(file, mLevel.getContained(file));
		}
}