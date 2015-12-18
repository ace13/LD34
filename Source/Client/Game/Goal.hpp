#pragma once

#include "Entity.hpp"
#include "../ResourceManager.hpp"

class Goal : public Entity
{
public:
	Goal();
	~Goal();
	
	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual std::string serialize() const;
	virtual bool deserialize(const std::string&);

	virtual void initialize();

	virtual const std::type_info& getType() const;
	virtual const std::string& getName() const;

	void setCompleted(bool completed = true);

	ResourceManager::Sound& getSound();
	const ResourceManager::Sound& getSound() const;

private:
	float mTime;

	ResourceManager::Sound mGoalSound;
};