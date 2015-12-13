#ifdef _MSC_VER
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "Level.hpp"

#include <SFML/System/InputStream.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <vector>

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
	if (len < sizeof(OnDisk::Header))
		return false;

	OnDisk::Header lvlHeader = *(const OnDisk::Header*)data;

	if (len < sizeof(OnDisk::Header) +
		sizeof(OnDisk::PlayerObj) +
		(lvlHeader.Rows * sizeof(uint64_t)) +
		(lvlHeader.ObjCount * sizeof(OnDisk::ObjDef)))
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

	mFiles[lower] = std::move(data);

	return true;
}

Level::File&& Level::getContained(const std::string& name) const
{
	std::string lower;
	std::transform(name.begin(), name.end(), std::back_inserter(lower), ::tolower);

	if (mFiles.count(name) > 0)
		return std::move(File(&mFiles.at(name)[0], mFiles.at(name).size()));

	return std::move(File(nullptr, 0));
}