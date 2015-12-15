#pragma once

#include "IState.hpp"
#include "../Game/Level.hpp"

#include "../ParticleManager.hpp"
#include "../ResourceManager.hpp"

#include <SFML/Audio/Sound.hpp>

#include <list>

class GameState : public IState
{
public:
	GameState();
	~GameState();

	virtual void enter(sf::RenderTarget*);
	virtual void exit(sf::RenderTarget*);

	virtual void event(const sf::Event&);
	virtual void tick(const Timespan&);
	virtual void update(const Timespan&);
	virtual void draw(sf::RenderTarget&);
	virtual void drawUI(sf::RenderTarget&);

	void loadLevel(const std::string& name);

private:
	std::string mCurCommand;
	std::list<std::string> mScripts;
	std::list<std::string> mHistory;
	Level mLevel;

	bool mEnded;
	float mEndTimeout;
	float mDot, mDir, mOff;

	sf::RenderTarget* mRT;
	sf::Sound mTickSound;
	ResourceManager::Sound mTick, mTickFail, mTickSucceed;

	Timestamp mNextExec;

	ParticleManager mPreParticles;
	ParticleManager mPostParticles;
};