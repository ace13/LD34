#include "Entity.hpp"
#include "Door.hpp"
#include "Enemy.hpp"
#include "Goal.hpp"
#include "Key.hpp"
#include "Level.hpp"
#include "Box.hpp"
#include "Pit.hpp"

#include <Core/Engine.hpp>
#include <Core/Math.hpp>

Entity* Entity::createFromType(const std::string& type)
{
	Entity* toRet = nullptr;

	if (type == "Goal")
		toRet = new Goal();
	else if (type == "BasicEnemy")
		toRet = new Enemy();
	else if (type == "Key")
		toRet = new Key();
	else if (type == "Door")
		toRet = new Door();
	else if (type == "Pit")
		toRet = new Pit();
	else if (type == "Box")
		toRet = new Box();
	else if (type == "Player")
		toRet = new Robot();

	return toRet;
}


Entity::Entity() :
	mLevel(nullptr),
	mIsGoal(false),
	mIsCompleted(false),
	mRadius(0),
	mRefCount(1)
{

}

Entity::~Entity() { }


bool Entity::move(float x, float y)
{
	sf::Vector2f vec{ x, y };
	return move(vec);
}
bool Entity::move(const sf::Vector2f& vec, float scale)
{
	auto checkPos = (getPosition() + (Math::Normalized(vec) * getRadius()) + vec * scale) / mLevel->getScale();
	if (!mLevel->isBlocked(uint8_t(checkPos.x), uint8_t(checkPos.y)))
	{
		sf::Transformable::move(vec);
		return true;
	}
	else if (scale > 0.25f)
		return move(vec, scale / 2.f);

	return false;
}

int Entity::addRef()
{
	return ++mRefCount;
}

int Entity::release()
{
	int refs = --mRefCount;
	if (refs == 0)
		delete this;

	return refs;
}


bool Entity::isGoal() const
{
	return mIsGoal;
}
bool Entity::isCompleted() const
{
	return mIsCompleted;
}

void Entity::setGoal(bool isGoal)
{
	mIsGoal = isGoal;
}
void Entity::setCompleted(bool completed)
{
	mIsCompleted = completed;
}

float Entity::getRadius() const
{
	return mRadius;
}
void Entity::setRadius(float r)
{
	mRadius = r;
}

const ParticleManager* Entity::getParticleManager(bool post) const
{
	if (!mLevel)
		return nullptr;
	return mLevel->getParticleManager(post);
}
ParticleManager* Entity::getParticleManager(bool post)
{
	if (!mLevel)
		return nullptr;
	return mLevel->getParticleManager(post);
}

const Engine* Entity::getEngine() const
{
	if (!mLevel)
		return nullptr;
	return mLevel->getEngine();
}
Engine* Entity::getEngine()
{
	if (!mLevel)
		return nullptr;
	return mLevel->getEngine();
}

const Level* Entity::getLevel() const
{
	return mLevel;
}
Level* Entity::getLevel()
{
	return mLevel;
}
void Entity::setLevel(Level* l)
{
	mLevel = l;
}
