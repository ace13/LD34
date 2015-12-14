#include "Goal.hpp"
#include "Level.hpp"

#include <Core/Engine.hpp>

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Goal::Goal() :
	mTime(0)
{
	setGoal();

	setRadius(40);
}
Goal::~Goal()
{

}

void Goal::tick(const Timespan& dt) { }
void Goal::update(const Timespan& dt)
{
	mTime += Time::Seconds(dt);
}
void Goal::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	sf::RectangleShape goal({ 40, 40 });
	goal.setOrigin(20, 20);
	goal.setFillColor(sf::Color::Transparent);
	goal.setOutlineColor({ 255, 255, 0 });
	goal.setOutlineThickness(4);

	if (isCompleted())
	{
		goal.setOutlineColor({ 0, 197, 0 });
		goal.scale(1.5, 1.5);

		target.draw(goal, states);

		goal.scale(0.5, 0.5);

		target.draw(goal, states);
	}
	else
	{
		goal.scale(1 + std::sin(mTime) / 2, 1 + std::sin(mTime) / 2);

		goal.rotate(mTime * 180);

		target.draw(goal, states);

		goal.rotate(mTime * 90);
		goal.scale(-0.75, -0.75);

		target.draw(goal, states);
	}
}

bool Goal::serialize(char* data, size_t size) const
{
	return true;
}
bool Goal::deserialize(const char* data, size_t size)
{
	return true;
}

void Goal::initialize()
{
	mGoalSound = getLevel()->getEngine()->get<ResourceManager>().get<sf::SoundBuffer>("goal.wav");
}

const std::string& Goal::getName() const
{
	static const std::string name = "Goal";
	return name;
}
void Goal::setCompleted(bool completed)
{
	Entity::setCompleted(completed);
}


ResourceManager::Sound& Goal::getSound()
{
	return mGoalSound;
}
const ResourceManager::Sound& Goal::getSound() const
{
	return mGoalSound;
}