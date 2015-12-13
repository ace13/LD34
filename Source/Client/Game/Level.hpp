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

	void tick(const Timespan& dt);
	void update(const Timespan& dt);
	void draw(sf::RenderTarget& rt);

	void setEngine(Engine*);

	void clearLevel();
	bool loadFromFile(const std::string& file);
	bool loadFromMemory(const void* data, size_t len);
	bool loadFromStream(sf::InputStream& file);

	bool saveToFile(const std::string& file) const;

	bool hasFile(const std::string& file) const;
	bool bakeFile(const std::string& file);
	File&& getContained(const std::string& name) const;

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

	Robot& getPlayer();
	const Robot& getPlayer() const;

	void addEntity(Entity* ent);
	void removeEntity(Entity* ent);
	const std::list<Entity*>& getEntities() const;

	int getNumberOfGoals() const;
	int getNumberOfCompletedGoals() const;

	const asIScriptModule* getScriptModule() const;
	void setScriptModule(asIScriptModule* mod);

private:
	typedef uint32_t RowWidth;

	Engine* mEngine;

	float mScale;
	bool mFlipped;
	sf::Vector2u mSize;
	std::vector<RowWidth> mBitmap;
	sf::Color mBackground, mForeground, mOutside;
	Robot mPlayer;
	
	std::list<Entity*> mEntities;
	asIScriptModule* mScriptModule;

	std::unordered_map<std::string, std::vector<char>> mFileData;
};