#include "GameState.hpp"

#include "../Application.hpp"
#include "../ResourceManager.hpp"

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
	sf::View tmp = rt->getView();
	tmp.zoom(0.4);
	rt->setView(tmp);

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
	sman.setPreLoadCallback(Entity::preLoadInject);
	sman.getEngine()->SetUserData(&mLevel, 0x1EE7);

	FileWatcher::recurseDirectory("Game", mScripts, "*.as");

	for (auto& script : mScripts)
	{
		sman.loadFromFile(script);
	}

	loadLevel("Tutorial1.lvl");

#define B(x,y) mLevel.setBlocked(x,y)
#define U(x,y) do {Entity* ent = Entity::createFromType("BasicEnemy", nullptr, 0); \
ent->setPosition(x*150+75, y*150+75); \
ent->setRotation(-90); \
mLevel.addEntity(ent); } while(false);
#define G(x,y) do {Entity* ent = Entity::createFromType("Goal", nullptr, 0); \
ent->setPosition(x*150+75, y*150+75); \
mLevel.addEntity(ent); } while(false);
#define P(x,y) mLevel.getPlayer().setPosition(x*150+75, y*150+75);
	
	// Level7.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color(0x3F, 0x68, 0x26));
	mLevel.setForegroundColor(sf::Color(0x3F, 0x68, 0x26));

	mLevel.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		5,
		5
	});

	std::string level =
		"#####"
		"# G #"
		"#   #"
		"# P #"
		"#####";

	for (int y = 0; y < mLevel.getSize().y; ++y)
		for (int x = 0; x < mLevel.getSize().x; ++x)
		{
			char obj = level[y*mLevel.getSize().x + x];

			switch (obj)
			{
			case ' ': break;
			case 'B':
			{
				Entity* ent = Entity::createFromType("Box", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'W':
			{
				Entity* ent = Entity::createFromType("Pit", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'K':
			{
				Entity* ent = Entity::createFromType("Key", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'D':
			{
				Entity* ent = Entity::createFromType("Door", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'G': G(x, y); break;
			case 'P': P(x, y); break;
			case 'U': U(x, y); break;

			case 'R': {
				Entity* ent = Entity::createFromType("BasicEnemy", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			} break;

			case '#':
			default:
				B(x, y);
				break;
			}
		}

	mLevel.getPlayer().setRotation(-90);
	mLevel.getPlayer().initialize();

	mLevel.bakeFile("level/level7.as");
	mLevel.saveToFile("Level7.lvl");
*/

	// Level6.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color(0x3F, 0x68, 0x26));
	mLevel.setForegroundColor(sf::Color(0x3F, 0x68, 0x26));

	mLevel.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		20,
		10
	});

	std::string level =
		"####################"
		"########RG #########"
		"##K######W######G###"
		"## ####U#P# ####D###"
		"#  R #       # R  ##"
		"#R   # ##D#U #   R##"
		"# R  # #R  # #  R ##"
		"## ### # B # ### ###"
		"#R     #   #R      #"
		"####################";

	for (int y = 0; y < mLevel.getSize().y; ++y)
		for (int x = 0; x < mLevel.getSize().x; ++x)
		{
			char obj = level[y*mLevel.getSize().x + x];

			switch (obj)
			{
			case ' ': break;
			case 'B':
			{
				Entity* ent = Entity::createFromType("Box", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'W':
			{
				Entity* ent = Entity::createFromType("Pit", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'K':
			{
				Entity* ent = Entity::createFromType("Key", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'D':
			{
				Entity* ent = Entity::createFromType("Door", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'G': G(x, y); break;
			case 'P': P(x, y); break;
			case 'U': U(x, y); break;

			case 'R': {
				Entity* ent = Entity::createFromType("BasicEnemy", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			} break;

			case '#':
			default:
				B(x, y);
				break;
			}
		}

	mLevel.getPlayer().setRotation(90);
	mLevel.getPlayer().initialize();

	mLevel.bakeFile("level/level6.as");
	mLevel.saveToFile("Level6.lvl");
*/

	// Level5.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color(0x3F, 0x68, 0x26));
	mLevel.setForegroundColor(sf::Color(0x3F, 0x68, 0x26));

	mLevel.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		20,
		10
	});

	std::string level =
		"####################"
		"####################"
		"############    K###"
		"### ###### # #######"
		"#P          D   R#G#"
		"#D#### ### # ##### #"
		"# ######## #U#  K  #"
		"#G######## ### #####"
		"##########     #####"
		"####################";

	for (int y = 0; y < mLevel.getSize().y; ++y)
		for (int x = 0; x < mLevel.getSize().x; ++x)
		{
			char obj = level[y*mLevel.getSize().x + x];

			switch (obj)
			{
			case ' ': break;
			case 'B':
			{
				Entity* ent = Entity::createFromType("Box", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'W':
			{
				Entity* ent = Entity::createFromType("Pit", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'K':
			{
				Entity* ent = Entity::createFromType("Key", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'D':
			{
				Entity* ent = Entity::createFromType("Door", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'G': G(x, y); break;
			case 'P': P(x, y); break;
			case 'U': U(x, y); break;

			case 'R': {
				Entity* ent = Entity::createFromType("BasicEnemy", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			} break;

			case '#':
			default:
				B(x, y);
				break;
			}
		}

	mLevel.getPlayer().setRotation(0);
	mLevel.getPlayer().initialize();

	mLevel.bakeFile("level/level5.as");
	mLevel.saveToFile("Level5.lvl");
*/

	// Level4.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color(0x3F, 0x68, 0x26));
	mLevel.setForegroundColor(sf::Color(0x3F, 0x68, 0x26));

	mLevel.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		18,
		9
	});

	std::string level =
		"##################"
		"#   ##### ####   #"
		"# P          D B #"
		"## ###### ###### #"
		"##W#K#### #####K #"
		"##D#      ###### #"
		"## ###### ###### #"
		"##G######U######U#"
		"##################";

	for (int y = 0; y < mLevel.getSize().y; ++y)
		for (int x = 0; x < mLevel.getSize().x; ++x)
		{
			char obj = level[y*mLevel.getSize().x + x];

			switch (obj)
			{
			case ' ': break;
			case 'B':
			{
				Entity* ent = Entity::createFromType("Box", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'W':
			{
				Entity* ent = Entity::createFromType("Pit", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'K':
			{
				Entity* ent = Entity::createFromType("Key", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'D':
			{
				Entity* ent = Entity::createFromType("Door", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'G': G(x, y); break;
			case 'P': P(x, y); break;
			case 'U': U(x, y); break;

			case 'R': {
				Entity* ent = Entity::createFromType("BasicEnemy", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			} break;

			case '#':
			default:
				B(x, y);
				break;
			}
		}

	mLevel.getPlayer().setRotation(0);
	mLevel.getPlayer().initialize();

	mLevel.bakeFile("level/level4.as");
	mLevel.saveToFile("Level4.lvl");
*/

	// Level3.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color(0x3F, 0x68, 0x26));
	mLevel.setForegroundColor(sf::Color(0x3F, 0x68, 0x26));

	mLevel.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		18,
		9
	});

	std::string level =
		"##################"
		"# P#######K## ####"
		"#   B   W        #"
		"#############U## #"
		"#  #######G####R #"
		"# G######R  #### #"
		"# ########D##### #"
		"#                #"
		"##################";

	for (int y = 0; y < mLevel.getSize().y; ++y)
		for (int x = 0; x < mLevel.getSize().x; ++x)
		{
			char obj = level[y*mLevel.getSize().x + x];

			switch (obj)
			{
			case ' ': break;
			case 'B':
			{
				Entity* ent = Entity::createFromType("Box", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'W':
			{
				Entity* ent = Entity::createFromType("Pit", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'K':
			{
				Entity* ent = Entity::createFromType("Key", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
			break;

			case 'D':
			{
				Entity* ent = Entity::createFromType("Door", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			} B(x, y);
			break;

			case 'G': G(x, y); break;
			case 'P': P(x, y); break;
			case 'U': U(x, y); break;

			case 'R': {
				Entity* ent = Entity::createFromType("BasicEnemy", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			} break;

			case '#':
			default:
				B(x, y);
				break;
			}
		}

	mLevel.getPlayer().setRotation(-90);
	mLevel.getPlayer().initialize();
	
	mLevel.bakeFile("level/level3.as");
	mLevel.saveToFile("Level3.lvl");
*/

	// Level2.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color(0x3F, 0x68, 0x26));
	mLevel.setForegroundColor(sf::Color(0x3F, 0x68, 0x26));

	mLevel.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		15,
		9
	});

	std::string level =
		"###############"
		"#############G#"
		"##P K D  G    #"
		"############# #"
		"############R #"
		"###### ### ## #"
		"##R           #"
		"#G  #### ######"
		"###############";

	for (int y = 0; y < mLevel.getSize().y; ++y)
		for (int x = 0; x < mLevel.getSize().x; ++x)
		{
			char obj = level[y*mLevel.getSize().x + x];

			switch (obj)
			{
			case ' ': break;
			case 'K': 
			{
				Entity* ent = Entity::createFromType("Key", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			}
				break;

			case 'D':
			{
				Entity* ent = Entity::createFromType("Door", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			} B(x, y);
				break;

			case 'G': G(x, y); break;
			case 'P': P(x, y); break;
			case 'U': U(x, y); break;

			case 'R': {
				Entity* ent = Entity::createFromType("BasicEnemy", nullptr, 0);
				ent->setPosition(x * 150 + 75, y * 150 + 75);
				mLevel.addEntity(ent);
			} break;

			case '#':
			default:
				B(x, y);
				break;
			}
		}

	mLevel.getPlayer().setRotation(-90);
	mLevel.getPlayer().initialize();

	mLevel.bakeFile("level/level2.as");
	mLevel.saveToFile("Level2.lvl");
*/

	// Level1.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color(0x3F, 0x68, 0x26));
	mLevel.setForegroundColor(sf::Color(0x3F, 0x68, 0x26));

	mLevel.setBackgroundColor(sf::Color(0xA3, 0x75, 0x49));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		15,
		9
	});

	std::string level =
		"###############"
		"##  ####   # ##"
		"# P ###### # ##"
		"### #G ###  UG#"
		"### ## ##  # ##"
		"###        # ##"
		"#####    ######"
		"######    G ###"
		"###############";

	for (int y = 0; y < mLevel.getSize().y; ++y)
		for (int x = 0; x < mLevel.getSize().x; ++x)
		{
			char obj = level[y*mLevel.getSize().x + x];

			switch (obj)
			{
			case ' ': break;
			case 'G': G(x, y); break;
			case 'P': P(x, y); break;
			case 'U': U(x, y); break;

			case '#':
			default:
				B(x, y);
				break;
			}
		}

	mLevel.getPlayer().setRotation(-90);
	mLevel.getPlayer().initialize();

	mLevel.bakeFile("level/level1.as");
	mLevel.saveToFile("Level1.lvl");
*/

	// Tutorial4.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color::Black);
	mLevel.setBackgroundColor(sf::Color(0x4A, 0x70, 0x23));
	mLevel.setForegroundColor(sf::Color(0x96, 0x6F, 0x33));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		9,
		9
	});

	B(0, 0); B(1, 0); B(2, 0); B(3, 0); B(4, 0); B(5, 0); B(6, 0); B(7, 0); B(8, 0);
	B(0, 1); P(1, 1); B(2, 1); B(3, 1); G(4, 1);                            B(8, 1);
	B(0, 2);          B(2, 2);                            B(6, 2);          B(8, 2);
	B(0, 3);                            B(4, 3);          B(6, 3); G(7, 3); B(8, 3);
	B(0, 4); B(1, 4);          B(3, 4); B(4, 4);          B(6, 4); B(7, 4); B(8, 4);
	B(0, 5);                   B(3, 5); G(4, 5);                            B(8, 5);
	B(0, 6); G(1, 6); B(2, 6); B(3, 6); B(4, 6); B(5, 6); B(6, 6); G(7, 6); B(8, 6);
	B(0, 7);                            G(4, 7); B(5, 7); G(6, 7);          B(8, 7);
	B(0, 8); B(1, 8); B(2, 8); B(3, 8); B(4, 8); B(5, 8); B(6, 8); B(7, 8); B(8, 8);

	mLevel.getPlayer().setRotation(90);
	mLevel.getPlayer().initialize();

	mLevel.bakeFile("level/tutorial4.as");

	mLevel.saveToFile("Tutorial4.lvl");
*/

	// Tutorial3.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color::Black);
	mLevel.setBackgroundColor(sf::Color(0x4A, 0x70, 0x23));
	mLevel.setForegroundColor(sf::Color(0x96, 0x6F, 0x33));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		7,
		7
	});

	B(0, 0); B(1, 0); B(2, 0); B(3, 0); B(4, 0); B(5, 0); B(6, 0);
	B(0, 1); G(1, 1); B(2, 1); B(3, 1); B(4, 1); B(5, 1); B(6, 1);
	B(0, 2);          B(2, 2);                            B(6, 2);
	B(0, 3);                            B(4, 3);          B(6, 3);
	B(0, 4); B(1, 4);          B(3, 4); B(4, 4);          B(6, 4);
	B(0, 5);                   B(3, 5); P(4, 5);          B(6, 5);
	B(0, 6); B(1, 6); B(2, 6); B(3, 6); B(4, 6); B(5, 6); B(6, 6);

	mLevel.getPlayer().setRotation(0);
	mLevel.getPlayer().initialize();

	mLevel.bakeFile("level/tutorial3.as");

	mLevel.saveToFile("Tutorial3.lvl");
*/

#undef B
#undef G
#undef P

	// Tutorial2.lvl
/*
	mLevel.setScale(150);
	mLevel.setOutsideColor(sf::Color::Black);
	mLevel.setBackgroundColor(sf::Color(0x4A, 0x70, 0x23));
	mLevel.setForegroundColor(sf::Color(0x96, 0x6F, 0x33));

	mLevel.getPlayer().passParticleManager(&mPreParticles);
	mLevel.getPlayer().setProgram(new BaseProgram());

	mLevel.setSize({
		5,
		5
	});

	mLevel.setBlocked(0, 0);
	mLevel.setBlocked(1, 0);
	mLevel.setBlocked(2, 0);
	mLevel.setBlocked(3, 0);
	mLevel.setBlocked(4, 0);
	mLevel.setBlocked(0, 1);
	mLevel.setBlocked(4, 1);
	mLevel.setBlocked(0, 2);
	mLevel.setBlocked(2, 2);
	mLevel.setBlocked(3, 2);
	mLevel.setBlocked(4, 2);
	mLevel.setBlocked(3, 2);
	mLevel.setBlocked(0, 3);
	mLevel.setBlocked(2, 3);
	mLevel.setBlocked(3, 3);
	mLevel.setBlocked(4, 3);
	mLevel.setBlocked(0, 4);
	mLevel.setBlocked(1, 4);
	mLevel.setBlocked(2, 4);
	mLevel.setBlocked(3, 4);
	mLevel.setBlocked(4, 4);

	mLevel.getPlayer().setPosition(225, 525);
	mLevel.getPlayer().setRotation(-90);
	mLevel.getPlayer().initialize();

	Entity* ent = Entity::createFromType("Goal", nullptr, 0);
	ent->setPosition(525, 225);
	mLevel.addEntity(ent);

	mLevel.bakeFile("level/tutorial2.as");

	mLevel.saveToFile("Tutorial2.lvl");
*/

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

	mLevel.bakeFile("level/tutorial1.as");

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

	auto* ent = Entity::createForScript(sman.getEngine()->GetModule("game\\robot.as"), "Robot");
	ent->setPosition(500, 500);
	mLevel.addEntity(ent);
	*/

	//mLevel.bakeFile("Game\\robot.as");

	
}
void GameState::exit(sf::RenderTarget*)
{
	delete explosionPrecache;

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
	else if (ev.type == sf::Event::KeyReleased && ev.key.code == sf::Keyboard::R)
		mLevel.resetLevel();
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
		mLevel.clearLevel();
		mHistory.clear();
		mCurCommand.clear();
		mPreParticles.clear();
		mPostParticles.clear();

		mEnded = false;
		mEndTimeout = 0;

		getEngine().get<ScriptManager>().runHook("OnLevelEnd");
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
			{
				auto fileS = mLevel.getContained(file);
				getEngine().get<ScriptManager>().loadFromStream(file, fileS);
			}
		}


	mLevel.getPlayer().passParticleManager(&mPreParticles);
}
