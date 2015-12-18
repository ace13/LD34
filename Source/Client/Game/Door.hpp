#pragma once

#include "Entity.hpp"
#include "../ResourceManager.hpp"

class Door : public Entity
{
public:
	Door();
	~Door();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual std::string serialize() const;
	virtual bool deserialize(const std::string&);

	virtual void initialize();

	virtual const std::type_info& getType() const;
	virtual const std::string& getName() const;

	bool isOpen() const;
	void open();

	ResourceManager::Sound& getSound();
	const ResourceManager::Sound& getSound() const;

private:
	bool mOpen;

	ResourceManager::Sound mDoorSound;
};