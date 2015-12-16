#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "Level.hpp"
#include "Entity.hpp"
#include "Program.hpp"

#include "../ParticleManager.hpp"

#include <Core/Engine.hpp>
#include <Core/Math.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/InputStream.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
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
			uint16_t Flipped : 1;
			uint16_t Rows : 6;
			uint16_t Cols : 6;
			uint16_t : 3;

			uint64_t ObjCount : 7;
			uint64_t ContainedFiles : 7;
			uint64_t BackgroundColor : 24;
			uint64_t ForegroundColor : 24;
			uint64_t : 2;

			uint32_t OutsideColor : 24;
			uint32_t : 16;

			float Scale;

			uint8_t ScriptNameLength;
			uint16_t ScriptDataLength;
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
				struct
				{
					uint8_t NameLength;
				} Default;
				struct
				{
					uint8_t ScriptLength;
					uint8_t ObjectNameLength;
				} Script;
			};

			uint16_t SerializedDataLength;
		};

		struct ContainedFile
		{
			uint8_t NameLength;
			uint16_t FileSize;
		};
	};
#pragma pack(pop)

	static const OnDisk::Version FILE_VERSION = 4;
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

sf::Int64 Level::File::read(void* data, sf::Int64 size)
{
	size_t toRead = std::max(std::min(mSize - mGetP, size_t(size)), size_t(0));

	const char* dataP = mData + mGetP;
	std::copy(dataP, dataP + toRead, (char*)data);

	mGetP += toRead;

	return toRead;
}
sf::Int64 Level::File::seek(sf::Int64 position)
{
	mGetP = std::max(std::min(mSize, size_t(position)), size_t(0));
	return mGetP;
}
sf::Int64 Level::File::tell()
{
	return mGetP;
}
sf::Int64 Level::File::getSize()
{
	return mSize;
}

Level::Level() :
	mEngine(nullptr),
	mScale(1),
	mScriptModule(nullptr)
{
	clearLevel();
}

Level::~Level()
{
	for (auto& ent : mEntities)
		ent->release();

	if (mScriptModule)
		mScriptModule->Discard();
}

void Level::tick(const Timespan& dt)
{
	mPlayer.tick(dt);
	for (auto& it : mEntities)
		it->tick(dt);
}
void Level::update(const Timespan& dt)
{
//	mPlayer.update(dt);
	for (auto& it : mEntities)
		it->update(dt);
}
void Level::drawBackface(sf::RenderTarget& rt)
{
	sf::VertexArray foreground(sf::Quads, mSize.x * mSize.y * 4 + 4);

	int maxWidth = std::min<int>(sizeof(RowWidth) * 8, (mFlipped ? mSize.y : mSize.x));
	int maxHeight = std::min<int>(sizeof(RowWidth) * 8, (mFlipped ? mSize.x : mSize.y));

	foreground.append({
		{ 0, 0 },
		mBackground
	});
	foreground.append({
		{ maxWidth * mScale, 0 },
		mBackground
	});
	foreground.append({
		{ maxWidth * mScale, maxHeight * mScale },
		mBackground
	});
	foreground.append({
		{ 0, maxHeight * mScale },
		mBackground
	});

	for (int x = 0; x < int(mSize.x); ++x)
	{
		for (int y = 0; y < int(mSize.y); ++y)
		{
			if (!isBlocked(x, y))
				continue;

			foreground.append({
				{ x*mScale, y*mScale },
				mForeground
			});
			foreground.append({
				{ x*mScale + mScale, y*mScale },
				mForeground
			});
			foreground.append({
				{ x*mScale + mScale, y*mScale + mScale },
				mForeground
			});
			foreground.append({
				{ x*mScale, y*mScale + mScale },
				mForeground
			});
		}
	}

	rt.draw(foreground);

	for (auto& it : mEntities)
		if (it->getName() == "Pit")
			rt.draw(*it);
}
void Level::draw(sf::RenderTarget& rt)
{
	for (auto& it : mEntities)
		if (it->getName() != "Pit")
			rt.draw(*it);
	rt.draw(mPlayer);
}

Level::operator bool() const
{
	return !mBitmap.empty();
}

Engine* Level::getEngine()
{
	return mEngine;
}
const Engine* Level::getEngine() const
{
	return mEngine;
}
void Level::setEngine(Engine* eng)
{
	mEngine = eng;
}

const std::string& Level::getName() const
{
	return mLoaded;
}
void Level::setName(const std::string& name)
{
	mLoaded = name;
}

void Level::clearLevel()
{
	mLoaded.clear();

	mScale = 1;
	mSize = {};
	mBitmap.clear();
	mOutside = sf::Color::Black;
	mBackground = sf::Color::White;
	mForeground = sf::Color::Black;
	mPlayer = nullptr;

	for (auto& it : mEntities)
		it->release();
	mEntities.clear();

	if (mScriptModule)
		mScriptModule->Discard();
	mScriptModule = nullptr;

	mFileData.clear();
}

void Level::resetLevel()
{
	if (mLoaded.empty())
	{
		std::cout << "Tried to reset level, but is not loaded." << std::endl << std::endl
			<< "Why are you trying to crash the game? :'(" << std::endl;
		return;
	}

	if (mParticlesPre)
		mParticlesPre->clear();
	if (mParticlesPost)
		mParticlesPost->clear();

	mBitmap = mPristineBitmap;
	mPlayer = nullptr;

	for (auto& it : mEntities)
		it->release();
	mEntities.clear();

	auto& sman = mEngine->get<ScriptManager>();
	for (auto& it : mPristineObjects)
	{
		Entity* ent;
		if (it.Type == OnDisk::ObjDef::Type_Script)
		{
			if (!sman.hasLoaded(it.FileName))
			{
				if (hasFile(it.FileName))
				{
					auto file = getContained(it.FileName);
					sman.loadFromStream(it.FileName, file);
				}
				else
					sman.loadFromFile(it.FileName);
			}

			ent = Entity::createForScript(sman.getEngine()->GetModule(it.FileName.c_str()), it.ObjectName.c_str());
		}
		else
			ent = Entity::createFromType(it.ObjectName);

		ent->deserialize(it.Serialized.c_str(), it.Serialized.size());

		ent->setPosition(it.X * mScale * 1.5f, it.Y * mScale * 1.5f);
		ent->setRotation(it.Dir * 90);

		if (ent->getName() == "Player")
			mPlayer = (Robot*)ent;

		addEntity(ent);
	}
}

bool Level::loadFromFile(const std::string& file)
{
	clearLevel();

	std::ifstream ifs(file.c_str());
	if (!ifs)
		return false;

	ifs.seekg(0, std::ios::end);
	size_t len = size_t(ifs.tellg());
	ifs.seekg(0, std::ios::beg);

	std::vector<char> data(len);
	ifs.read(&data[0], len);

	if (loadFromMemory(&data[0], len))
	{
		mLoaded = file;
		return true;
	}
	return false;
}
bool Level::loadFromMemory(const void* data, size_t len)
{
	clearLevel();

	if (!mEngine)
		return false;

	if (len < sizeof(OnDisk::Version) + sizeof(OnDisk::Header))
		return false;

	File reader((const char*)data, len);

	OnDisk::Version version = {};
	reader.read(&version, sizeof(OnDisk::Version));

	if (version != FILE_VERSION)
		return false;

	OnDisk::Header lvlHeader = {};
	reader.read(&lvlHeader, sizeof(OnDisk::Header));

	size_t minSize = size_t(
		sizeof(OnDisk::Version) +
		sizeof(OnDisk::Header) +
		(lvlHeader.ScriptNameLength) +
		(lvlHeader.ScriptDataLength) +
		(lvlHeader.Rows * sizeof(OnDisk::Row)) +
		(lvlHeader.ObjCount * sizeof(OnDisk::ObjDef)) +
		(lvlHeader.ContainedFiles * sizeof(OnDisk::ContainedFile))
	);

	if (len < minSize)
		return false;

	std::string levelScriptName('\0', lvlHeader.ScriptNameLength);
	reader.read(&levelScriptName[0], lvlHeader.ScriptNameLength);
	std::string levelScriptData('\0', lvlHeader.ScriptDataLength);
	reader.read(&levelScriptData[0], lvlHeader.ScriptDataLength);

	std::vector<OnDisk::Row> rows(lvlHeader.Rows);
	if (!rows.empty())
		reader.read(&rows[0], lvlHeader.Rows * sizeof(OnDisk::Row));

	auto& sman = mEngine->get<ScriptManager>();

	std::vector<ObjectData> objs(lvlHeader.ObjCount);
	for (auto& it : objs)
	{
		OnDisk::ObjDef def;
		reader.read(&def, sizeof(OnDisk::ObjDef));

		it.Type = def.Type;
		it.Dir = def.Dir;
		it.X = def.PosX;
		it.Y = def.PosY;

		if (def.Type == OnDisk::ObjDef::Type_Script)
		{
			it.FileName.resize(def.Script.ScriptLength, 0);
			reader.read(&it.FileName, def.Script.ScriptLength);
			it.ObjectName.resize(def.Script.ObjectNameLength, 0);
			reader.read(&it.ObjectName, def.Script.ObjectNameLength);
		}
		else
		{
			it.ObjectName.resize(def.Default.NameLength, 0);
			reader.read(&it.ObjectName, def.Default.NameLength);
		}

		it.Serialized.resize(def.SerializedDataLength, 0);
		reader.read(&it.Serialized, def.SerializedDataLength);
	}

	std::unordered_map<std::string, std::vector<char>> fileData;
	for (int i = 0; i < lvlHeader.ContainedFiles; ++i)
	{
		OnDisk::ContainedFile file = {};
		reader.read((char*)&file, sizeof(OnDisk::ContainedFile));

		std::string name('\0', file.NameLength);
		reader.read(&name[0], file.NameLength);

		auto& dataStore = fileData[name];
		dataStore.resize(file.FileSize);

		if (reader.read(&dataStore[0], file.FileSize) != file.FileSize)
			return false;
	}
	
	if (lvlHeader.Flipped)
	{
		mSize.x = lvlHeader.Rows;
		mSize.y = lvlHeader.Cols;
	}
	else
	{
		mSize.y = lvlHeader.Rows;
		mSize.x = lvlHeader.Cols;
	}
	mFlipped = lvlHeader.Flipped;
	mScale = lvlHeader.Scale;
	mBitmap = rows;
	mOutside = sf::Color(uint32_t(lvlHeader.OutsideColor) << 8 | 0xff);
	mBackground = sf::Color(uint32_t(lvlHeader.BackgroundColor) << 8 | 0xff);
	mForeground = sf::Color(uint32_t(lvlHeader.ForegroundColor) << 8 | 0xff);

	mFileData = std::move(fileData);

	if (!levelScriptName.empty())
	{
		if (!sman.hasLoaded(levelScriptName))
		{
			if (hasFile(levelScriptName))
			{
				auto file = getContained(levelScriptName);
				sman.loadFromStream(levelScriptName, file);
			}
			else
				sman.loadFromFile(levelScriptName);
		}

		auto* eng = sman.getEngine();
		auto* mod = eng->GetModule(levelScriptName.c_str());
		asIScriptFunction* func = mod->GetFunctionByDecl("void OnLoad()");
		if (func)
		{
			auto* ctx = eng->RequestContext();
			ctx->Prepare(func);

			ctx->Execute();

			eng->ReturnContext(ctx);
		}
	}

	mPristineObjects = std::move(objs);
	mPristineBitmap = std::move(rows);

	resetLevel();

	return true;
}
bool Level::loadFromStream(sf::InputStream& file)
{
	clearLevel();

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
		head.Cols = mSize.y;
	}
	else
	{
		head.Rows = mSize.y;
		head.Cols = mSize.x;
	}

	if (mFileData.size() > std::pow(2, 7))
		return false;

	head.Scale = mScale;
	head.ContainedFiles = mFileData.size();
	head.OutsideColor = mOutside.toInteger() >> 8;
	head.BackgroundColor = mBackground.toInteger() >> 8;
	head.ForegroundColor = mForeground.toInteger() >> 8;
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
	OnDisk::PlayerObj player = {};

	auto pos = mPlayer.getPosition() / mScale;
	player.PosX = uint8_t(pos.x);
	player.PosY = uint8_t(pos.y);
	float ang = mPlayer.getRotation();
	while (ang < 0)
		ang += 360;
	player.Dir = uint8_t(ang / 90) % 4;

	std::string name = mPlayer.getProgram()->getName();
	std::copy_n(name.c_str(), name.size(), player.Programming);

	ofs.write((const char*)&player, sizeof(OnDisk::PlayerObj));
	if (!mBitmap.empty())
		ofs.write((const char*)&mBitmap[0], sizeof(OnDisk::Row) * head.Rows);

	std::vector<OnDisk::ObjDef> objs(head.ObjCount);
	int i = 0;
	for (auto& ent : mEntities)
	{
		auto& o = objs[i++];
		
		auto pos = ent->getPosition() / mScale;
		o.PosX = uint8_t(pos.x);
		o.PosY = uint8_t(pos.y);
		float ang = ent->getRotation();
		while (ang < 360)
			ang += 360;
		o.Dir =	uint8_t(ang / 90) % 4;

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
	if (!objs.empty())
		ofs.write((const char*)&objs[0], sizeof(OnDisk::ObjDef) * objs.size());

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

	if (!files.empty())
	{
		if (!ofs.write((const char*)&files[0], files.size() * sizeof(OnDisk::ContainedFile)))
			return false;

		for (auto& fileData : mFileData)
			if (!ofs.write(&fileData.second[0], fileData.second.size()))
				return false;
	}

	return true;
}

bool Level::hasFile(const std::string& file) const
{
	std::string lower;
	std::transform(file.begin(), file.end(), std::back_inserter(lower), ::tolower);

	return mFileData.count(lower) > 0;
}

std::list<std::string> Level::getFiles() const
{
	std::list<std::string> toRet;

	for (auto& f : mFileData)
		toRet.push_back(f.first);

	return toRet;
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
	ifs.read(&data[0], len);

	// Remove extraneous nullbytes
	auto it = std::remove(data.begin(), data.end(), '\0');
	if (it != data.end())
		data.erase(it, data.end());

	std::string lower;
	std::transform(file.begin(), file.end(), std::back_inserter(lower), ::tolower);

	mFileData[lower] = std::move(data);

	return true;
}

Level::File Level::getContained(const std::string& name) const
{
	std::string lower;
	std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);

	if (mFileData.count(name) > 0)
		return File(&mFileData.at(name)[0], mFileData.at(name).size());

	return File(nullptr, 0);
}

float Level::getScale() const
{
	return mScale;
}
void Level::setScale(float scale)
{
	mScale = scale;
}

const sf::Vector2u& Level::getSize() const
{
	return mSize;
}
void Level::setSize(const sf::Vector2u& s)
{
	mSize = s;

	mFlipped = mSize.x > sizeof(RowWidth) * 8;
	
	if (mFlipped)
		mBitmap.resize(mSize.x);
	else
		mBitmap.resize(mSize.y);
}

bool Level::isBlocked(uint8_t x, uint8_t y) const
{
	if (x >= mSize.x || y >= mSize.y)
		return true;

	if (mFlipped)
		return (mBitmap[x] & (1 << y)) != 0;
	return (mBitmap[y] & (1 << x)) != 0;
}
void Level::setBlocked(uint8_t x, uint8_t y, bool blocked)
{
	if (blocked)
	{
		if (mFlipped)
			mBitmap[x] |= (1 << y);
		else
			mBitmap[y] |= (1 << x);
	}
	else
	{
		if (mFlipped)
			mBitmap[x] &= ~(1 << y);
		else
			mBitmap[y] &= ~(1 << x);
	}
}

const sf::Color& Level::getOutsideColor() const
{
	return mOutside;
}
void Level::setOutsideColor(const sf::Color& col)
{
	mOutside = col;
}
const sf::Color& Level::getBackgroundColor() const
{
	return mBackground;
}
void Level::setBackgroundColor(const sf::Color& col)
{
	mBackground = col;
}
const sf::Color& Level::getForegroundColor() const
{
	return mForeground;
}
void Level::setForegroundColor(const sf::Color& col)
{
	mForeground = col;
}

Robot& Level::getPlayer()
{
	return mPlayer;
}
const Robot& Level::getPlayer() const
{
	return mPlayer;
}

bool Level::findEntities(std::list<Entity*>& out, const Entity& source)
{
	auto& pos = source.getPosition();

	bool found = false;
	for (auto& it : mEntities)
	{
		if (&it == &source)
			continue;

		auto& epos = it->getPosition();
		if (Math::Length(epos - pos) <= it->getRadius() + source.getRadius())
		{
			out.push_back(it);
			found = true;
		}
	}

	return found;
}

void Level::addEntity(Entity* ent)
{
	ent->setLevel(this);
	ent->initialize();

	mEntities.push_back(ent);
}
void Level::removeEntity(Entity* ent)
{
	auto it = std::find(mEntities.begin(), mEntities.end(), ent);
	if (it != mEntities.end())
	{
		(*it)->release();
		mEntities.erase(it);
	}
}
const std::list<Entity*>& Level::getEntities() const
{
	return mEntities;
}

int Level::getNumberOfGoals() const
{
	int num = 0;
	for (auto& e : mEntities)
	{
		if (e->isGoal())
			++num;
	}
	return num;
}
int Level::getNumberOfCompletedGoals() const
{
	int num = 0;
	for (auto& e : mEntities)
	{
		if (e->isGoal() && e->isCompleted())
			++num;
	}
	return num;
}

const ParticleManager* Level::getParticleManager(bool post) const
{
	if (post)
		return mParticlesPost;
	return mParticlesPre;
}
ParticleManager* Level::getParticleManager(bool post)
{
	if (post)
		return mParticlesPost;
	return mParticlesPre;
}
void Level::setParticleManager(ParticleManager* p, bool post)
{
	if (post)
		mParticlesPost = p;
	else
		mParticlesPre = p;
}

const asIScriptModule* Level::getScriptModule() const
{
	return mScriptModule;
}
void Level::setScriptModule(asIScriptModule* mod)
{
	if (mScriptModule)
		mScriptModule->Discard();
	mScriptModule = mod;
}
