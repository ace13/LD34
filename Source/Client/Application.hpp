#pragma once

#include "StateMachine.hpp"

#include <Core/Engine.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Graphics/RenderWindow.hpp>

#include <list>

class Application
{
public:
	Application();
	Application(const Application&) = delete;
	~Application();

	Application& operator=(const Application&) = delete;

	void init(int argc, const char** argv);

	void run();

	sf::RenderTarget& getRT();

private:
	Engine mEngine;
	StateMachine mState;
	std::list<std::string> mArgs;
};
