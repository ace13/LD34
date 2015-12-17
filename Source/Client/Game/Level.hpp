#pragma once

#include "Robot.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/Vector2.hpp>

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

class Engine;
class Entity;
class asIScriptModule;

class Level
{
public:
	class File : public sf::InputStream
	{
	public:
		~File();

		operator bool() const;

		sf::Int64 read(void* data, sf::Int64 size);
		sf::Int64 seek(sf::Int64 position);
		sf::Int64 tell();
		sf::Int64 getSize();

	private:
		File(const char* data, size_t size);

		const char* mData;
		size_t mGetP, mSize;

		friend class Level;
	};

	Level();
	~Level();

	void tick(const Timespan& dt);
	void update(const Timespan& dt);
	void drawBackface(sf::RenderTarget& rt);
	void draw(sf::RenderTarget& rt);

	operator bool() const;

	Engine* getEngine();
	const Engine* getEngine() const;
	void setEngine(Engine*);

	void clearLevel();
	void resetLevel();

	bool loadFromFile(const std::string& file);
	bool loadFromMemory(const void* data, size_t len);
	bool loadFromStream(sf::InputStream& file);

	bool saveToFile(const std::string& file) const;

	bool hasFile(const std::string& file) const;
	std::list<std::string> getFiles() const;
	bool bakeFile(const std::string& file);
	File getContained(const std::string& name) const;

	float getScale() const;
	void setScale(float scale);

	const sf::Vector2u& getSize() const;
	void setSize(const sf::Vector2u&);

	bool isBlocked(uint8_t x, uint8_t y) const;
	void setBlocked(uint8_t x, uint8_t y, bool blocked = true);

	const sf::Color& getOutsideColor() const;
	void setOutsideColor(const sf::Color&);
	const sf::Color& getBackgroundColor() const;
	void setBackgroundColor(const sf::Color&);
	const sf::Color& getForegroundColor() const;
	void setForegroundColor(const sf::Color&);

	Robot* getPlayer();
	const Robot* getPlayer() const;

	bool findEntities(std::list<Entity*>& out, const Entity& source);

	void addEntity(Entity* ent);
	void removeEntity(Entity* ent);
	const std::list<Entity*>& getEntities() const;

	int getNumberOfGoals() const;
	int getNumberOfCompletedGoals() const;

	const ParticleManager* getParticleManager(bool post = false) const;
	ParticleManager* getParticleManager(bool post = false);
	void setParticleManager(ParticleManager*, bool post = false);

	const asIScriptModule* getScriptModule() const;

	const std::string& getLevelScriptName() const;
	void setLevelScriptName(const std::string&);

private:
	struct ObjectData
	{
		uint8_t Type;

		uint8_t X, Y, Dir;

		std::string FileName;
		std::string ObjectName;

		std::string Serialized;
	};
	typedef uint32_t RowWidth;

	std::vector<ObjectData> mPristineObjects;
	std::vector<RowWidth> mPristineBitmap;

	Engine* mEngine;
	ParticleManager* mParticlesPre, *mParticlesPost;

	float mScale;
	bool mFlipped, mDirty;
	sf::Vector2u mSize;
	std::vector<RowWidth> mBitmap;
	sf::Color mBackground, mForeground, mOutside;
	Robot* mPlayer;
	
	std::list<Entity*> mEntities;
	asIScriptModule* mScriptModule;
	std::string mScriptName;

	std::unordered_map<std::string, std::vector<char>> mFileData;
};
