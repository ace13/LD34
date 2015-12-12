#include "MainMenu.hpp"

MainMenuPage::MainMenuPage()
{
	addEntry("Start Game", []() {});
	addEntry("How to play", []() {});
	addEntry("Quit Game", []() {});
}

MainMenuPage::~MainMenuPage()
{

}