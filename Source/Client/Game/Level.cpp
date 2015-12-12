#include "Level.hpp"

#include <SFML/System/InputStream.hpp>

#include <fstream>
#include <vector>

Level::Level()
{

}

Level::~Level()
{

}

bool Level::loadFromFile(const std::string& file)
{
	std::ifstream ifs(file.c_str());
	if (!ifs)
		return false;

	ifs.seekg(0, std::ios::end);
	size_t len = size_t(ifs.tellg());
	ifs.seekg(0, std::ios::beg);

	std::vector<char> data(len);
	if (!ifs.read(&data[0], len))
		return false;

	return loadFromMemory(&data[0], len);
}
bool Level::loadFromMemory(const void* data, size_t len)
{
	if (len < sizeof(Header))
		return false;

	Header lvlHeader = *(const Header*)data;

	if (len < sizeof(Header) +
		sizeof(PlayerData) +
		(lvlHeader.Rows * sizeof(uint64_t)) +
		(lvlHeader.ObjCount * sizeof(ObjDef)))
		return false;



	return true;
}
bool Level::loadFromStream(sf::InputStream& file)
{
	size_t len = size_t(file.getSize());
	std::vector<char> data(len);
	file.read(&data[0], len);

	return loadFromMemory(&data[0], len);
}

bool Level::saveToFile(const std::string& file)
{
	// TODO: Save the level, to reuse as a save-game format

	return false;
}