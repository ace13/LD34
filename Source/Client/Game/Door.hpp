#pragma once

#include "Entity.hpp"

class Door : public Entity
{
public:
	Door();
	~Door();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual bool serialize(char* data, size_t size) const;
	virtual bool deserialize(const char* data, size_t size);

	virtual void initialize();

	virtual const std::string& getName() const;

	bool isOpen() const;
	void open();

private:
	bool mOpen;
};