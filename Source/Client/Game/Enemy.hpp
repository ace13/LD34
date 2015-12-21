#pragma once

#include "Entity.hpp"

class Enemy : public Entity
{
public:
	Enemy();
	~Enemy();

	virtual void tick(const Timespan& dt);
	virtual void update(const Timespan& dt);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	virtual bool serialize(OutputStream&) const;
	virtual bool deserialize(InputStream&);

	virtual void initialize();

	virtual const std::type_info& getType() const;
	virtual const std::string& getName() const;

private:
	enum State
	{
		State_Moving,
		State_Turning,
	};

	State mCurState;
	float mTargetAng, mSpeed;
	int mTick;
};
