#include "Key.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

Key::Key()
{
	setRadius(10);
}
Key::~Key() { }

void Key::tick(const Timespan& dt) { }
void Key::update(const Timespan& dt) { }
void Key::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	
	sf::CircleShape key(10);
	key.setOrigin(10, 10);
	key.setFillColor({
		255,255,0
	});


	target.draw(key, states);
}

bool Key::serialize(char* data, size_t size) const { return true; }
bool Key::deserialize(const char* data, size_t size) { return true; }
void Key::initialize() { }

const std::string& Key::getName() const
{
	static const std::string name = "Key";
	return name;
}