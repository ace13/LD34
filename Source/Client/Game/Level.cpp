#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "Level.hpp"
#include "Entity.hpp"
#include "ScriptEntity.hpp"

#include "../ParticleManager.hpp"

#include <Core/Engine.hpp>
#include <Core/Math.hpp>
#include <Core/OutputStream.hpp>
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
			enum {
				Flag_Flipped = 0x1
			};

			uint8_t Flags;
			
			uint8_t Rows;
			uint8_t Cols;

			uint8_t ObjCount;
			uint8_t ContainedFiles;
			uint32_t BackgroundColor;
			uint32_t ForegroundColor;
			uint32_t OutsideColor;

			float Scale;

			uint8_t ScriptNameLength;
			uint16_t ScriptDataLength;

			Header() :
				Flags(0),
				Rows(0),
				Cols(0),
				ObjCount(0),
				ContainedFiles(0),
				BackgroundColor(0),
				ForegroundColor(0),
				OutsideColor(0),
				Scale(0),
				ScriptNameLength(0),
				ScriptDataLength(0)
			{ }
		};

		struct ObjDef
		{
			enum {
				Type_Default = 0,
				Type_Script = 1
			};
			uint8_t Type;

			uint8_t PosX;
			uint8_t PosY;
			uint8_t Dir;

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
				uint16_t DATA;
			};

			uint16_t SerializedDataLength;

			ObjDef() :
				Type(0),
				PosX(0),
				PosY(0),
				Dir(0),
				DATA(0),
				SerializedDataLength(0)
			{ }
		};

		struct ContainedFile
		{
			uint8_t NameLength;
			uint16_t FileSize;

			ContainedFile() :
				NameLength(0),
				FileSize(0)
			{ }
		};
	};
#pragma pack(pop)

	class OutputFile : public OutputStream
	{
	public:
		OutputFile() : mSetP(0), mReserved(0) { }

		size_t getMaxSize() const
		{
			return mReserved;
		}
		size_t tell() const
		{
			return mSetP;
		}
		size_t write(const void* data, size_t len)
		{
			if (len == 0)
				return 0;

			if (mReserved - mSetP < len)
				return 0;

			std::copy((const char*)data, (const char*)data + len, &mData[mSetP]);
			mSetP += len;

			return len;
		}
		size_t seek(size_t pos)
		{
			if (mSetP + pos >= mReserved)
				mSetP = mReserved - 1;
			else
				mSetP += pos;

			return mSetP;
		}

		size_t reserve(size_t len)
		{
			mData.resize(len);
			mReserved = len;
			return len;
		}

		std::string str() const
		{
			return std::string(&mData[0], mSetP);
		}

	private:
		size_t mSetP, mReserved;
		std::vector<char> mData;
	};

	static const OnDisk::Version FILE_VERSION = 4;

#if SFML_VERSION_MINOR < 3
	sf::Color ColorFromInt(uint32_t color) {
		return sf::Color{
			uint8_t((color & 0xff000000) >> 24),
			uint8_t((color & 0x00ff0000) >> 16),
			uint8_t((color & 0x0000ff00) >> 8),
			uint8_t((color & 0x000000ff) >> 0)
		};
	}
	uint32_t ColorToInt(const sf::Color& c) {
		return (c.r << 24) | (c.g << 16) | (c.b << 8) | c.a;
	}
#else
#define ColorFromInt sf::Color
#define ColorToInt(a) a.toInteger()
#endif
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
	if (size == 0)
		return 0;

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

const char* Level::File::getPtr() const
{
	return mData;
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
	for (auto& it : mEntities)
	{
		it->tick(dt);

		if (mDirty)
			break;
	}
	mDirty = false;
}
void Level::update(const Timespan& dt)
{
	for (auto& it : mEntities)
	{
		it->update(dt);

		if (mDirty)
			break;
	}
	mDirty = false;
}
void Level::drawBackface(sf::RenderTarget& rt) const
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
	{
		if (it->isBackground())
			rt.draw(*it);
	}
}
void Level::draw(sf::RenderTarget& rt) const
{
	for (auto& it : mEntities)
	{
		if (!it->isBackground())
			rt.draw(*it);
	}
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

void Level::clearLevel()
{
	mScale = 1;
	mSize = {};
	mBitmap.clear();
	mScriptName.clear();
	mOutside = sf::Color::Black;
	mBackground = sf::Color::White;
	mForeground = sf::Color::Black;
	mPlayer = nullptr;

	for (auto& it : mEntities)
		it->release();
	mEntities.clear();

	if (mScriptModule)
	{
		auto& sman = mEngine->get<ScriptManager>();
		auto* fun = mScriptModule->GetFunctionByDecl("void OnUnload()");
		if (fun)
		{
			auto* ctx = sman.getEngine()->RequestContext();

			ctx->Prepare(fun);
			ctx->Execute();
			ctx->Unprepare();

			sman.getEngine()->ReturnContext(ctx);
		}

		sman.unload(mScriptModule->GetName());
	}
	mScriptModule = nullptr;

	mFileData.clear();
}

void Level::resetLevel()
{
	if (mPristineBitmap.empty() || mPristineObjects.empty())
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
	mDirty = true;

	for (auto& it : mEntities)
		it->release();
	mEntities.clear();

	auto& sman = mEngine->get<ScriptManager>();
	for (auto& it : mPristineObjects)
	{
		Entity* ent;
		if (it.Type == OnDisk::ObjDef::Type_Script)
		{
			if (!sman.isLoaded(it.FileName))
			{
				if (hasFile(it.FileName))
				{
					auto file = getContained(it.FileName);
					sman.loadFromStream(it.FileName, file);
				}
				else
					sman.loadFromFile(it.FileName);
			}

			ent = ScriptEntity::createForScript(sman.getEngine()->GetModule(it.FileName.c_str()), it.ObjectName.c_str());
		}
		else
			ent = Entity::createFromType(it.ObjectName);

		File temp(it.Serialized.c_str(), it.Serialized.length());
		ent->deserialize(temp);

		ent->setPosition(it.X * mScale + mScale / 2.f, it.Y * mScale + mScale / 2.f);
		ent->setRotation(it.Dir * 90.f);

		addEntity(ent);
	}

	if (mScriptModule)
	{
		auto* fun = mScriptModule->GetFunctionByDecl("void OnReset()");
		if (fun)
		{
			auto* ctx = sman.getEngine()->RequestContext();

			ctx->Prepare(fun);
			ctx->Execute();
			ctx->Unprepare();

			sman.getEngine()->ReturnContext(ctx);
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

	File reader(data.data(), len);

	return loadFromStream(reader);
}
bool Level::loadFromMemory(const void* data, size_t len)
{
	if (!mEngine)
		return false;

	if (len < sizeof(OnDisk::Version) + sizeof(OnDisk::Header))
		return false;

	File reader((const char*)data, len);
	return loadFromStream(reader);
}
bool Level::loadFromStream(sf::InputStream& reader)
{
	if (size_t(reader.getSize()) < sizeof(OnDisk::Version) + sizeof(OnDisk::Header))
		return false;

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

	if (size_t(reader.getSize()) < minSize)
		return false;

	std::string levelScriptName(lvlHeader.ScriptNameLength, 0);
	reader.read(&levelScriptName[0], lvlHeader.ScriptNameLength);
	std::string levelScriptData(lvlHeader.ScriptDataLength, 0);
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
			reader.read(&it.FileName[0], def.Script.ScriptLength);
			it.ObjectName.resize(def.Script.ObjectNameLength, 0);
			reader.read(&it.ObjectName[0], def.Script.ObjectNameLength);
		}
		else
		{
			it.ObjectName.resize(def.Default.NameLength, 0);
			reader.read(&it.ObjectName[0], def.Default.NameLength);
		}

		it.Serialized.resize(def.SerializedDataLength, 0);
		reader.read(&it.Serialized[0], def.SerializedDataLength);
	}

	std::unordered_map<std::string, std::vector<char>> fileData;
	for (int i = 0; i < lvlHeader.ContainedFiles; ++i)
	{
		OnDisk::ContainedFile file = {};
		reader.read((char*)&file, sizeof(OnDisk::ContainedFile));

		std::string name(file.NameLength, 0);
		reader.read(&name[0], file.NameLength);

		auto& dataStore = fileData[std::move(name)];
		dataStore.resize(file.FileSize);

		if (reader.read(&dataStore[0], file.FileSize) != file.FileSize)
			return false;
	}

	clearLevel();

	if (lvlHeader.Flags & OnDisk::Header::Flag_Flipped)
	{
		mSize.x = lvlHeader.Rows;
		mSize.y = lvlHeader.Cols;

		mFlipped = true;
	}
	else
	{
		mSize.y = lvlHeader.Rows;
		mSize.x = lvlHeader.Cols;

		mFlipped = false;
	}
	mScale = lvlHeader.Scale;
	mOutside = ColorFromInt(uint32_t(lvlHeader.OutsideColor) << 8 | 0xff);
	mBackground = ColorFromInt(uint32_t(lvlHeader.BackgroundColor) << 8 | 0xff);
	mForeground = ColorFromInt(uint32_t(lvlHeader.ForegroundColor) << 8 | 0xff);

	mFileData = std::move(fileData);

	mPristineObjects = std::move(objs);
	mPristineBitmap = std::move(rows);

	resetLevel();

	mScriptName = std::move(levelScriptName);
	if (!mScriptName.empty())
	{
		if (!sman.isLoaded(mScriptName))
		{
			if (hasFile(mScriptName))
			{
				auto file = getContained(mScriptName);
				sman.loadFromStream(mScriptName, file);
			}
			else
				sman.loadFromFile(mScriptName);
		}

		auto* eng = sman.getEngine();
		auto* mod = eng->GetModule(mScriptName.c_str());
		asIScriptFunction* func = mod->GetFunctionByDecl("void OnLoad(const string&in)");
		if (func)
		{
			auto* ctx = eng->RequestContext();
			ctx->Prepare(func);
			ctx->SetArgObject(0, &levelScriptData);

			ctx->Execute();

			eng->ReturnContext(ctx);
		}

		mScriptModule = mod;
	}

	return true;
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
		head.Flags |= OnDisk::Header::Flag_Flipped;
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
	head.OutsideColor = ColorToInt(mOutside) >> 8;
	head.BackgroundColor = ColorToInt(mBackground) >> 8;
	head.ForegroundColor = ColorToInt(mForeground) >> 8;
	head.ObjCount = mEntities.size();

	std::string scriptData;
	if (mScriptModule)
	{
		auto* eng = mEngine->get<ScriptManager>().getEngine();
		asIScriptFunction* func = mScriptModule->GetFunctionByDecl("string OnSave()");
		if (func)
		{
			auto* ctx = eng->RequestContext();
			ctx->Prepare(func);

			ctx->Execute();
			scriptData = *(std::string*)ctx->GetReturnObject();

			eng->ReturnContext(ctx);
		}
	}

	head.ScriptNameLength = mScriptName.length();
	head.ScriptDataLength = scriptData.length();

	ofs.write((const char*)&head, sizeof(OnDisk::Header));
	ofs.write(mScriptName.c_str(), mScriptName.length());
	ofs.write(scriptData.c_str(), scriptData.length());

	if (!mBitmap.empty())
		ofs.write((const char*)&mBitmap[0], sizeof(OnDisk::Row) * head.Rows);

	for (auto& ent : mEntities)
	{
		OnDisk::ObjDef o = {};
		
		auto pos = ent->getPosition() / mScale;
		o.PosX = uint8_t(pos.x);
		o.PosY = uint8_t(pos.y);
		float ang = ent->getRotation();
		while (ang < 0)
			ang += 360;
		o.Dir =	uint8_t(ang / 90) % 4;

		std::string objectName;
		std::string scriptName;

		o.Type = ent->getType() == typeid(ScriptEntity);
		if (o.Type)
		{
			auto* sent = reinterpret_cast<ScriptEntity*>(ent);
			scriptName = sent->getScriptObject()->GetObjectType()->GetModule()->GetName();
			o.Script.ScriptLength = scriptName.length();
			objectName = sent->getScriptObject()->GetObjectType()->GetName();
			o.Script.ObjectNameLength = objectName.length();
		}
		else
		{
			objectName = ent->getName();
			o.Default.NameLength = objectName.length();
		}


		OutputFile data;
		if (ent->serialize(data))
			o.SerializedDataLength = data.tell();
		else
			o.SerializedDataLength = 0;

		ofs.write((const char*)&o, sizeof(OnDisk::ObjDef));
		if (o.Type == OnDisk::ObjDef::Type_Script)
			ofs.write(scriptName.c_str(), scriptName.length());
		ofs.write(objectName.c_str(), objectName.length());
		if (o.SerializedDataLength > 0)
			ofs.write(data.str().c_str(), o.SerializedDataLength);
	}

	for (auto& fileData : mFileData)
	{
		if (fileData.second.size() > UINT16_MAX)
			return false;

		OnDisk::ContainedFile f = {};
		f.FileSize = uint16_t(fileData.second.size());
		f.NameLength = fileData.first.length();

		ofs.write((const char*)&f, sizeof(OnDisk::ContainedFile));
		ofs.write(fileData.first.c_str(), fileData.first.length());
		ofs.write(fileData.second.data(), fileData.second.size());
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

Robot* Level::getPlayer()
{
	return mPlayer;
}
const Robot* Level::getPlayer() const
{
	return mPlayer;
}

bool Level::findEntities(std::list<Entity*>& out, const Entity& source)
{
	auto& pos = source.getPosition();

	bool found = false;
	for (auto& it : mEntities)
	{
		if (it == &source)
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

	if (ent->getName() == "Player")
		mPlayer = (Robot*)ent;

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
const std::string& Level::getLevelScriptName() const
{
	return mScriptName;
}
void Level::setLevelScriptName(const std::string& name)
{
	mScriptName = name;
}
