#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "Level.hpp"
#include "Entity.hpp"
#include "Program.hpp"

#include <Core/Engine.hpp>
#include <Core/Math.hpp>
#include <Core/ScriptManager.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
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
			uint16_t Flipped : 1;
			uint16_t Rows : 6;
			uint16_t Cols : 6;
			uint16_t Unused2 : 3;

			uint64_t ObjCount : 7;
			uint64_t ContainedFiles : 7;
			uint64_t BackgroundColor : 24;
			uint64_t ForegroundColor : 24;
			uint64_t Unused3 : 2;

			uint32_t OutsideColor : 24;
			uint32_t Unused4 : 16;

			float Scale;

			char ScriptFile[48];
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
			char FileName[48];
			uint16_t FileSize;
		};
	};
#pragma pack(pop)

	static const OnDisk::Version FILE_VERSION = 4;
}

namespace
{
	OnDisk::PlayerObj lastLoadedPlayer;
	std::vector<OnDisk::ObjDef> lastLoadedObjects;
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
	size_t toRead = std::max(std::min(mSize - mGetP, size_t(size)), size_t(0));

	const char* dataP = mData + mGetP;
	std::copy(dataP, dataP + toRead, (char*)data);

	mGetP += toRead;

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

	int maxWidth = std::min(sizeof(RowWidth) * 8, (mFlipped ? mSize.y : mSize.x));
	int maxHeight = std::min(sizeof(RowWidth) * 8, (mFlipped ? mSize.x : mSize.y));

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
}
void Level::draw(sf::RenderTarget& rt)
{
	for (auto& it : mEntities)
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
	mPlayer = {};
	mPlayer.setLevel(this);
	mPlayer.passParticleManager(mParticles);

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
	mPlayer = {};
	mPlayer.setLevel(this);
	mPlayer.passParticleManager(mParticles);

	mPlayer.setPosition({
		lastLoadedPlayer.PosX * mScale + mScale / 2,
		lastLoadedPlayer.PosY * mScale + mScale / 2
	});
	mPlayer.setRotation(
		float(lastLoadedPlayer.Dir) * 90
		);

	mPlayer.setProgram(Program::createProgramming(lastLoadedPlayer.Programming));
	mPlayer.initialize();

	for (auto& it : mEntities)
		it->release();
	mEntities.clear();

	auto& sman = mEngine->get<ScriptManager>();
	for (auto& it : lastLoadedObjects)
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

			ent->setPosition(it.PosX * mScale + mScale / 2, it.PosY * mScale + mScale / 2);
			ent->setRotation(float(it.Dir) * 90);

			addEntity(ent);
		}
		else
		{
			auto* ent = Entity::createFromType(it.Default.ObjType, it.ObjectData, sizeof(it.ObjectData));

			ent->setPosition(it.PosX * mScale + mScale / 2, it.PosY * mScale + mScale / 2);
			ent->setRotation(float(it.Dir) * 90);

			addEntity(ent);
		}
	}
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
		sizeof(OnDisk::PlayerObj) +
		(lvlHeader.Rows * sizeof(OnDisk::Row)) +
		(lvlHeader.ObjCount * sizeof(OnDisk::ObjDef)) +
		(lvlHeader.ContainedFiles * sizeof(OnDisk::ContainedFile))
	);

	if (len < minSize)
		return false;

	OnDisk::PlayerObj player = {};
	reader.read(&player, sizeof(OnDisk::PlayerObj));

	std::vector<OnDisk::Row> rows(lvlHeader.Rows);
	if (!rows.empty())
		reader.read(&rows[0], lvlHeader.Rows * sizeof(OnDisk::Row));

	std::vector<OnDisk::ObjDef> objs(lvlHeader.ObjCount);
	if (!objs.empty())
		reader.read(&objs[0], lvlHeader.ObjCount * sizeof(OnDisk::ObjDef));

	std::vector<OnDisk::ContainedFile> files(lvlHeader.ContainedFiles);
	if (!files.empty())
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
	clearLevel();
	
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
	mBitmap = std::move(rows);
	mOutside = sf::Color(uint32_t(lvlHeader.OutsideColor) << 8 | 0xff);
	mBackground = sf::Color(uint32_t(lvlHeader.BackgroundColor) << 8 | 0xff);
	mForeground = sf::Color(uint32_t(lvlHeader.ForegroundColor) << 8 | 0xff);
	{
		mPlayer.setPosition({
			player.PosX * mScale + mScale / 2,
			player.PosY * mScale + mScale / 2
		});
		mPlayer.setRotation(
			float(player.Dir) * 90
		);

		mPlayer.setProgram(Program::createProgramming(player.Programming));
		mPlayer.initialize();
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

			ent->setPosition(it.PosX * mScale + mScale / 2, it.PosY * mScale + mScale / 2);
			ent->setRotation(float(it.Dir) * 90);

			addEntity(ent);
		}
		else
		{
			auto* ent = Entity::createFromType(it.Default.ObjType, it.ObjectData, sizeof(it.ObjectData));

			ent->setPosition(it.PosX * mScale + mScale / 2, it.PosY * mScale + mScale / 2);
			ent->setRotation(float(it.Dir) * 90);

			addEntity(ent);
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

	lastLoadedObjects = std::move(objs);
	lastLoadedPlayer = std::move(player);

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

bool Level::findEntities(std::list<Entity*>& out, uint8_t x, uint8_t y)
{
	if (isBlocked(x, y))
		return false;

	sf::Vector2f pos{
		x * mScale + mScale / 2,
		y * mScale + mScale / 2
	};

	bool found = false;
	for (auto& it : mEntities)
	{
		auto& epos = it->getPosition();
		if (Math::Length(epos - pos) <= it->getRadius())
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

const ParticleManager* Level::getParticleManager() const
{
	return mParticles;
}
ParticleManager* Level::getParticleManager()
{
	return mParticles;
}
void Level::setParticleManager(ParticleManager* p)
{
	mParticles = p;
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