#pragma once

#include <SFML/System/InputStream.hpp>

#include <string>
#include <unordered_map>
#include <vector>

class Level
{
public:
	class File : public sf::InputStream
	{
	public:
		~File();

		operator bool() const;

		int64_t read(void* data, int64_t size);
		int64_t seek(int64_t position);
		int64_t tell();
		int64_t getSize();

	private:
		File(const char* data, size_t size);

		const char* mData;
		size_t mGetP, mSize;

		friend class Level;
	};

	Level();
	~Level();

	bool loadFromFile(const std::string& file);
	bool loadFromMemory(const void* data, size_t len);
	bool loadFromStream(sf::InputStream& file);

	bool saveToFile(const std::string& file);

	bool bakeFile(const std::string& file);
	File&& getContained(const std::string& name) const;

private:
#pragma pack(push, 1)
	struct OnDisk
	{
		struct Header
		{
			uint64_t Rows : 8;
			uint64_t ObjCount : 10;
			uint64_t ContainedFiles : 6;

			uint64_t BackgroundColor : 24;

			char ScriptFile[24];
		};

		struct PlayerObj
		{
			uint64_t PosX : 8;
			uint64_t PosY : 8;
			uint64_t Dir : 2;

			char Programming[72];
		};

		struct ObjDef
		{
			uint8_t Type : 1;

			union
			{
				struct DefObjDef
				{
					uint32_t PosX : 8;
					uint32_t PosY : 8;
					uint32_t Dir : 2;

					char ObjType[16];
				};
				struct ScriptObjDef
				{
					char ScriptFile[48];
					char ScriptObject[24];
				};
			};
		};

		struct ContainedFile
		{
			char FileName[12];
			uint16_t FileSize;
			uint16_t FileOffset;
		};
	};
#pragma pack(pop)

	//Header mHeader;
	//PlayerData mPlayerData;
	//std::vector<ObjDef> mObjects;
	std::unordered_map<std::string, std::vector<char>> mFiles;

};