#include "Goal.hpp"

Goal::Goal()
{
	setGoal();
}
Goal::~Goal()
{

}

void Goal::tick(const Timespan& dt) { }
void Goal::update(const Timespan& dt)
{


}
void Goal::draw(sf::RenderTarget& target, sf::RenderStates states) const
{

}

void Goal::setCompleted(bool completed)
{
	Entity::setCompleted(completed);
}