#pragma once

#include <functional>
#include <string>
#include <vector>

class MenuPage
{
public:
	virtual ~MenuPage() { }

private:
	struct Entry
	{
		std::string Name;
		std::function<void()> Callback;
	};

	std::vector<Entry> mEntries;
};
