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

#include <sstream>

namespace
{
	ResourceManager::Sound* explosionPrecache;

	void buildLevel(Level&, const std::string&);
	void bakeLevels(Level& level)
	{
		static const std::string tutorials[] = {
			"###\n"
			"#G#\n"
			"# #\n"
			"#^#\n"
			"###\n",

			"#####\n"
			"#  G#\n"
			"# ###\n"
			"#^###\n"
			"#####\n",

			"#######\n"
			"#G#####\n"
			"# #   #\n"
			"#   # #\n"
			"## ## #\n"
			"#  #> #\n"
			"#######\n",

			"#########\n"
			"#V# G   #\n"
			"#   # #G#\n"
			"## ## ###\n"
			"#  #G   #\n"
			"#G#####G#\n"
			"#   G#G #\n"
			"#########\n"
		};
		static const std::string levels[] = {
			"###############\n"
			"##  ####   # ##\n"
			"# ^ ###### # ##\n"
			"### #G ###  UG#\n"
			"### ## ##  # ##\n"
			"###        # ##\n"
			"#####    ######\n"
			"######    G ###\n"
			"###############\n",

			"###############\n"
			"#############G#\n"
			"##> K D  G    #\n"
			"############# #\n"
			"############R #\n"
			"###### ### ## #\n"
			"##R           #\n"
			"#G  #### ######\n"
			"###############\n",

			"##################\n"
			"# V#######K## ####\n"
			"#   B   W        #\n"
			"#############U## #\n"
			"#  #######G####R #\n"
			"# G######R  #### #\n"
			"# ########D##### #\n"
			"#                #\n"
			"##################\n",

			"##################\n"
			"#   ##### ####   #\n"
			"# >          D B #\n"
			"## ###### ###### #\n"
			"##W#K#### #####K #\n"
			"##D#      ###### #\n"
			"## ###### ###### #\n"
			"##G######U######U#\n"
			"##################\n",

			"####################\n"
			"#D  ################\n"
			"#B# ########   KK###\n"
			"#D# ##### ## #####G#\n"
			"#>          D   L# #\n"
			"# #### ### # ##### #\n"
			"#W######## #U#  K  #\n"
			"#G######## ### #####\n"
			"##########     #####\n"
			"####################\n",

			"######################\n"
			"###K#####RG #####K####\n"
			"###G######W######G####\n"
			"###D####N#V# ####D####\n"
			"##  R #     U # R  ###\n"
			"##R   # ## ## #   R###\n"
			"## R  # #N  # #  R ###\n"
			"### ### # B # ### ####\n"
			"#R      # KU#R      L#\n"
			"######################\n",

			"         \n"
			"         \n"
			"   #G#   \n"
			"  #####  \n"
			"  #####  \n"
			"   ###   \n"
			"    #    \n"
			"         \n"
			"    ^    \n"
		};

		std::ostringstream oss;
		int id = 0;
		for (auto& lvl : tutorials)
		{
			level.clearLevel();

			level.setOutsideColor(sf::Color::Black);
			level.setBackgroundColor(sf::Color(0x4A, 0x70, 0x23));
			level.setForegroundColor(sf::Color(0x96, 0x6F, 0x33));

			buildLevel(level, lvl);
			level.getPlayer()->setProgram(new BaseProgram());

			oss.str("");
			oss << "level/tutorial" << ++id << ".as";
			level.bakeFile(oss.str());
			level.setLevelScriptName(oss.str());
			
			oss.str("");
			oss << "Tutorial" << id << ".lvl";
			level.saveToFile(oss.str());
		}

		id = 0;
		for (auto& lvl : levels)
		{
			level.clearLevel();

			if (id != 6)
			{
				level.setOutsideColor(sf::Color(0x3F, 0x68, 0x26));
				level.setForegroundColor(sf::Color(0x3F, 0x68, 0x26));
				level.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));
			}
			else
			{
				level.setOutsideColor(sf::Color(0xA3, 0x75, 0x49));
				level.setForegroundColor(sf::Color(197, 0, 0));
				level.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));
			}

			buildLevel(level, lvl);
			level.getPlayer()->setProgram(new BaseProgram());

			oss.str("");
			oss << "level/level" << ++id << ".as";
			level.bakeFile(oss.str());
			level.setLevelScriptName(oss.str());
			
			oss.str("");
			oss << "Level" << id << ".lvl";
			level.saveToFile(oss.str());
		}

		level.clearLevel();
	}
	
	void buildLevel(Level& level, const std::string& str)
	{
		unsigned int height = std::count(str.begin(), str.end(), '\n');
		unsigned int width = str.find('\n');

		level.setScale(150.f);
		level.setSize({
			width,
			height
		});

		int x = 0;
		int y = 0;

		for (auto& obj : str)
		{
			switch (obj)
			{
			case ' ':
				break;

			case '\n':
				y++;
				x = -1;
				break;

			case 'B':
			{
				Entity* ent = Entity::createFromType("Box");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				level.addEntity(ent);
			} break;

			case 'W':
			{
				Entity* ent = Entity::createFromType("Pit");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				level.addEntity(ent);
			} break;

			case 'K':
			{
				Entity* ent = Entity::createFromType("Key");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				level.addEntity(ent);
			} break;

			case 'D':
			{
				Entity* ent = Entity::createFromType("Door");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				level.addEntity(ent);
				level.setBlocked(x, y);
			} break;

			case 'G':
			{
				Entity* ent = Entity::createFromType("Goal");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				level.addEntity(ent);
			} break;

			case '>': {
				Entity* ent = Entity::createFromType("Player");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				level.addEntity(ent);
			} break;
			case 'V': {
				Entity* ent = Entity::createFromType("Player");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				ent->setRotation(90);
				level.addEntity(ent);
			} break;
			case '<': {
				Entity* ent = Entity::createFromType("Player");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				ent->setRotation(180);
				level.addEntity(ent);
			} break;
			case '^': {
				Entity* ent = Entity::createFromType("Player");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				ent->setRotation(270);
				level.addEntity(ent);
			} break;

			case 'U': {
				Entity* ent = Entity::createFromType("BasicEnemy");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				ent->setRotation(-90);
				level.addEntity(ent);
			} break;

			case 'N': {
				Entity* ent = Entity::createFromType("BasicEnemy");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				ent->setRotation(90);
				level.addEntity(ent);
			} break;

			case 'L': {
				Entity* ent = Entity::createFromType("BasicEnemy");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				ent->setRotation(180);
				level.addEntity(ent);
			} break;

			case 'R': {
				Entity* ent = Entity::createFromType("BasicEnemy");
				ent->setPosition(x * 150.f + 75.f, y * 150.f + 75.f);
				level.addEntity(ent);
			} break;

			case '#':
			default:
				level.setBlocked(x, y);
				break;
			}

			x += 1;
		}
	}
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

	bakeLevels(mLevel);
	
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
