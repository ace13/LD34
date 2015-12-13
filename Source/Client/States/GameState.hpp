#pragma once

#include "IState.hpp"
#include "../Game/Entity.hpp"
#include "../Game/Robot.hpp"

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

	void injectEntity(Entity* ent);

private:
	std::string mCurCommand;
	std::list<std::string> mScripts;
	std::list<std::string> mHistory;
	std::list<Entity*> mEntities;

	float mDot, mDir, mOff;

	sf::Sound mTickSound;
	ResourceManager::Sound mTick, mTickFail, mTickSucceed;

	Timestamp mNextExec;
	Robot mRobot;

	ParticleManager mPreParticles;
	ParticleManager mPostParticles;
};