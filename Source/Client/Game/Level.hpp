#pragma once

#include <string>

namespace sf { class InputStream; }

class Level
{
public:
	Level();
	~Level();

	bool loadFromFile(const std::string& file);
	bool loadFromMemory(const void* data, size_t len);
	bool loadFromStream(sf::InputStream& file);

	bool saveToFile(const std::string& file);

private:
#pragma pack(push, 1)
	struct Header
	{
		uint32_t Rows : 8;
		uint32_t ObjCount : 16;
	};

	struct PlayerData
	{
		uint64_t PosX : 8;
		uint64_t PosY : 8;
		uint64_t Dir : 2;

		// MORE DATA IS NEEDED
	};

	struct ObjDef
	{
		// The market for data has crashed 
	};
#pragma pack(pop)


};