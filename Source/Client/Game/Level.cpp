#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "Level.hpp"
#include "Entity.hpp"

#include <Core/Engine.hpp>
#include <Core/Math.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/System/InputStream.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>

namespace
{
#pragma pack(push, 1)
	struct OnDisk
	{
		typedef uint8_t Version;
		typedef uint32_t Row;

		struct Header
		{
			uint8_t Flipped : 1;
			uint8_t Rows : 6;
			uint8_t Unused : 1;

			uint64_t ObjCount : 7;
			uint64_t ContainedFiles : 7;
			uint64_t BackgroundColor : 24;
			uint64_t ForegroundColor : 24;
			uint64_t Unused2 : 2;

			float Scale;

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
			enum {
				Type_Default = 0,
				Type_Script = 1
			};
			uint8_t Type : 1;

			uint32_t PosX : 8;
			uint32_t PosY : 8;
			uint32_t Dir : 2;

			union
			{
				struct DefObjDef
				{
					char ObjType[72];
				} Default;
				struct ScriptObjDef
				{
					char ScriptFile[48];
					char ScriptObject[24];
				} Script;
			};

			char ObjectData[32];
		};

		struct ContainedFile
		{
			char FileName[12];
			uint16_t FileSize;
		};
	};
#pragma pack(pop)

	static const OnDisk::Version FILE_VERSION = 1;
}

Level::File::File(const char* data, size_t size) :
	mData(data), mGetP(0), mSize(size)
{

}
Level::File::~File()
{

}

Level::File::operator bool() const
{
	return mData != nullptr && mSize > 0;
}

int64_t Level::File::read(void* data, int64_t size)
{
	size_t toRead = std::max(std::min(mSize - mGetP, mGetP + size_t(size)), size_t(0));
	std::copy(mData + mGetP, mData + mGetP + toRead, (char*)data);

	return toRead;
}
int64_t Level::File::seek(int64_t position)
{
	mGetP = std::max(std::min(mSize, size_t(position)), size_t(0));
	return mGetP;
}
int64_t Level::File::tell()
{
	return mGetP;
}
int64_t Level::File::getSize()
{
	return mSize;
}

Level::Level() :
	mEngine(nullptr),
	mScale(1),
	mScriptModule(nullptr)
{

}

Level::~Level()
{
	for (auto& ent : mEntities)
		ent->release();

	if (mScriptModule)
		mScriptModule->Discard();
}

void Level::setEngine(Engine* eng)
{
	mEngine = eng;
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
	if (!mEngine)
		return false;

	if (len < sizeof(OnDisk::Version) + sizeof(OnDisk::Header))
		return false;

	File reader((const char*)data, len);

	OnDisk::Version version;
	reader.read(&version, sizeof(OnDisk::Version));

	if (version != FILE_VERSION)
		return false;

	OnDisk::Header lvlHeader;
	reader.read(&lvlHeader, sizeof(OnDisk::Header));

	if (len < sizeof(OnDisk::Version) +
		sizeof(OnDisk::Header) +
		sizeof(OnDisk::PlayerObj) +
		(lvlHeader.Rows * sizeof(OnDisk::Row)) +
		(lvlHeader.ObjCount * sizeof(OnDisk::ObjDef)) +
		(lvlHeader.ContainedFiles * sizeof(OnDisk::ContainedFile))
		)
		return false;

	OnDisk::PlayerObj player;
	reader.read(&player, sizeof(OnDisk::PlayerObj));

	std::vector<OnDisk::Row> rows(lvlHeader.Rows);
	reader.read(&rows[0], lvlHeader.Rows * sizeof(OnDisk::Row));

	std::vector<OnDisk::ObjDef> objs(lvlHeader.ObjCount);
	reader.read(&objs[0], lvlHeader.ObjCount * sizeof(OnDisk::ObjDef));

	std::vector<OnDisk::ContainedFile> files(lvlHeader.ContainedFiles);
	reader.read(&files[0], lvlHeader.ContainedFiles * sizeof(OnDisk::ContainedFile));

	std::unordered_map<std::string, std::vector<char>> fileData;
	for (auto& file : files)
	{
		auto& dataStore = fileData[file.FileName];
		dataStore.resize(file.FileSize);

		if (reader.read(&dataStore[0], file.FileSize) != file.FileSize)
			return false;
	}

	// Load succeeded

	if (lvlHeader.Flipped)
	{
		mSize.x = lvlHeader.Rows;
		mSize.y = sizeof(RowWidth) * 8;
	}
	else
	{
		mSize.y = lvlHeader.Rows;
		mSize.x = sizeof(RowWidth) * 8;
	}
	mScale = lvlHeader.Scale;
	mBitmap = std::move(rows);
	mBackground = sf::Color(lvlHeader.BackgroundColor);
	mForeground = sf::Color(lvlHeader.ForegroundColor);
	{
		mPlayer.setPosition({
			player.PosX * mScale,
			player.PosY * mScale
		});
		mPlayer.setRotation(
			player.Dir * Math::PI2
		);
	}

	mFileData = std::move(fileData);

	auto& sman = mEngine->get<ScriptManager>();
	for (auto& it : objs)
	{
		if (it.Type == OnDisk::ObjDef::Type_Script)
		{
			if (!sman.hasLoaded(it.Script.ScriptFile))
			{
				if (hasFile(it.Script.ScriptFile))
					sman.loadFromStream(it.Script.ScriptFile, getContained(it.Script.ScriptFile));
				else
					sman.loadFromFile(it.Script.ScriptFile);
			}

			auto* ent = Entity::createForScript(sman.getEngine()->GetModule(it.Script.ScriptFile), it.Script.ScriptObject);
			ent->deserialize(it.ObjectData, sizeof(it.ObjectData));

			ent->setPosition(it.PosX * mScale, it.PosY * mScale);
			ent->setRotation(it.Dir * 90);

			mEntities.push_back(ent);
		}
		else
		{
			auto* ent = Entity::createFromType(it.Default.ObjType, it.ObjectData);

			ent->setPosition(it.PosX * mScale, it.PosY * mScale);
			ent->setRotation(it.Dir * 90);

			mEntities.push_back(ent);
		}
	}

	std::string script(lvlHeader.ScriptFile);
	if (!script.empty())
	{
		if (!sman.hasLoaded(script))
		{
			if (hasFile(script))
				sman.loadFromStream(script, getContained(script));
			else
				sman.loadFromFile(script);
		}

		auto* eng = sman.getEngine();
		auto* mod = eng->GetModule(script.c_str());
		asIScriptFunction* func = mod->GetFunctionByDecl("void OnLoad()");
		if (func)
		{
			auto* ctx = eng->RequestContext();
			ctx->Prepare(func);

			ctx->Execute();

			eng->ReturnContext(ctx);
		}
	}


	return true;
}
bool Level::loadFromStream(sf::InputStream& file)
{
	size_t len = size_t(file.getSize());
	std::vector<char> data(len);
	file.read(&data[0], len);

	return loadFromMemory(&data[0], len);
}

bool Level::saveToFile(const std::string& file) const
{
	std::ofstream ofs(file.c_str());
	if (!ofs)
		return false;

	ofs.write((const char*)&FILE_VERSION, sizeof(OnDisk::Version));
	OnDisk::Header head{};
	if (mSize.x > sizeof(RowWidth) * 8)
	{
		head.Flipped = true;
		head.Rows = mSize.x;
	}
	else
		head.Rows = mSize.y;

	if (mFileData.size() > std::pow(2, 7))
		return false;

	head.Scale = mScale;
	head.ContainedFiles = mFileData.size();
	head.BackgroundColor = mBackground.toInteger();
	head.ForegroundColor = mForeground.toInteger();
	head.ObjCount = mEntities.size();

	if (mScriptModule)
	{
		auto* eng = mEngine->get<ScriptManager>().getEngine();
		asIScriptFunction* func = mScriptModule->GetFunctionByDecl("void OnSave()");
		if (func)
		{
			auto* ctx = eng->RequestContext();
			ctx->Prepare(func);

			ctx->Execute();

			eng->ReturnContext(ctx);
		}

		std::string name = mScriptModule->GetName();
		std::copy_n(name.c_str(), name.size(), head.ScriptFile);
	}

	ofs.write((const char*)&head, sizeof(OnDisk::Header));
	OnDisk::PlayerObj player;

	auto pos = mPlayer.getPosition() / mScale;
	player.PosX = uint8_t(pos.x);
	player.PosX = uint8_t(pos.y);
	player.Dir = uint8_t(mPlayer.getRotation() / Math::PI2) % 4;

	ofs.write((const char*)&player, sizeof(OnDisk::PlayerObj));
	ofs.write((const char*)&mBitmap[0], sizeof(OnDisk::Row) * head.Rows);

	std::vector<OnDisk::ObjDef> objs(head.ObjCount);
	int i = 0;
	for (auto& ent : mEntities)
	{
		auto& o = objs[i];
		
		auto pos = ent->getPosition() / mScale;
		o.PosX = uint8_t(pos.x);
		o.PosY = uint8_t(pos.y);
		o.Dir =	uint8_t(ent->getRotation() / 90) % 4;

		o.Type = ent->isScriptEntity();
		if (ent->isScriptEntity())
		{
			std::string name = ent->getScriptObject()->GetObjectType()->GetModule()->GetName();
			std::copy_n(name.c_str(), name.size(), o.Script.ScriptFile);
			name = ent->getScriptObject()->GetObjectType()->GetName();
			std::copy_n(name.c_str(), name.size(), o.Script.ScriptObject);
		}
		else
		{
			std::string name = ent->getName();
			std::copy_n(name.c_str(), name.size(), o.Default.ObjType);
		}

		ent->serialize(o.ObjectData, sizeof(o.ObjectData));
	}

	std::vector<OnDisk::ContainedFile> files(head.ContainedFiles);
	i = 0;
	for (auto& fileData : mFileData)
	{
		if (fileData.second.size() > UINT16_MAX)
			return false;

		auto& f = files[i++];
		f.FileSize = uint16_t(fileData.second.size());

		std::copy_n(fileData.first.c_str(), fileData.first.size(), f.FileName);
	}

	if (!ofs.write((const char*)&files[0], files.size() * sizeof(OnDisk::ContainedFile)))
		return false;

	for (auto& fileData : mFileData)
		if (!ofs.write(&fileData.second[0], fileData.second.size()))
			return false;

	return true;
}

bool Level::hasFile(const std::string& file) const
{
	std::string lower;
	std::transform(file.begin(), file.end(), std::back_inserter(lower), ::tolower);

	return mFileData.count(lower) > 0;
}

bool Level::bakeFile(const std::string& file)
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

	std::string lower;
	std::transform(file.begin(), file.end(), std::back_inserter(lower), ::tolower);

	mFileData[lower] = std::move(data);

	return true;
}

Level::File&& Level::getContained(const std::string& name) const
{
	std::string lower;
	std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);

	if (mFileData.count(name) > 0)
		return std::move(File(&mFileData.at(name)[0], mFileData.at(name).size()));

	return std::move(File(nullptr, 0));
}