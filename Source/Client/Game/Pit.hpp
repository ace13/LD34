#pragma once

#include "Entity.hpp"

class Pit : public Entity
{
public:
	Pit();
	~Pit();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual bool serialize(OutputStream&) const;
	virtual bool deserialize(InputStream&);

	virtual void initialize();

	virtual const std::type_info& getType() const;
	virtual const std::string& getName() const;

	bool isFull() const;
	void fill();

private:
	bool mFull;
};
