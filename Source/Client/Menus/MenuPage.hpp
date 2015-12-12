#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <functional>
#include <string>
#include <vector>

namespace sf { class Event; class Font; }

class MenuPage : public sf::Drawable, public sf::Transformable
{
public:
	MenuPage();
	virtual ~MenuPage() { }

	virtual void event(const sf::Event& ev);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	void setFont(const sf::Font& f);

protected:
	void addEntry(const std::string& name, const std::function<void()>& callback);

private:
	struct Entry
	{
		std::string Name;
		std::function<void()> Callback;
	};

	std::vector<Entry> mEntries;
	Entry* mSelectedEntry;
	int mSelectedIndex;
	const sf::Font* mFont;
};
