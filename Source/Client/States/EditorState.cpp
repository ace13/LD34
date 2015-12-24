#include "EditorState.hpp"

#include "../Game/Level.hpp"
#include "../Game/Program.hpp"

#include "../States/GameState.hpp"

#include <Core/Engine.hpp>
#include <Core/ScriptManager.hpp>

#include <sstream>

namespace
{

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

EditorState::EditorState()
{

}
EditorState::~EditorState()
{

}

void EditorState::enter(sf::RenderTarget*)
{
	Level l;
	l.setEngine(&getEngine());

	bakeLevels(l);

	getEngine().get<ScriptManager>().unloadAll();
	getStateMachine()->changeState<GameState>(true);
}
void EditorState::exit(sf::RenderTarget*)
{

}

void EditorState::event(const sf::Event&)
{

}
void EditorState::tick(const Timespan&)
{

}
void EditorState::update(const Timespan& dt)
{
}
void EditorState::draw(sf::RenderTarget& rt)
{

}
void EditorState::drawUI(sf::RenderTarget& rt)
{

}
