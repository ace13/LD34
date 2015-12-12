#include "MenuPage.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/Event.hpp>

MenuPage::MenuPage() :
	mSelectedEntry(nullptr),
	mSelectedIndex(-1),
	mFont(nullptr)
{

}

void MenuPage::event(const sf::Event& ev)
{
	if (!(ev.type == sf::Event::MouseButtonPressed) ||
		ev.type == sf::Event::MouseButtonReleased ||
		ev.type == sf::Event::MouseMoved)
		return;

	if (ev.type == sf::Event::MouseMoved)
	{
		sf::Vector2f mPos{ float(ev.mouseMove.x), float(ev.mouseMove.y) };

		getTransform().transformPoint(mPos);

		sf::Text text;
		if (mFont)
			text.setFont(*mFont);

		bool found = false;
		int i = 0;
		for (auto& entry : mEntries)
		{
			text.setString(entry.Name);

			if (text.getGlobalBounds().contains(mPos))
			{
				mSelectedIndex = i;
				mSelectedEntry = &entry;
				found = true;

				break;
			}

			text.move(0, text.getLocalBounds().height + 5);
			++i;
		}

		if (!found)
		{
			mSelectedEntry = nullptr;
			mSelectedIndex = -1;
		}
	}
}

void MenuPage::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	
	sf::Text text;
	if (mFont)
		text.setFont(*mFont);

	for (auto& entry : mEntries)
	{
		text.setString(entry.Name);
		
		if (&entry == mSelectedEntry)
			text.setColor(sf::Color(255, 255, 0));

		target.draw(text, states);

		if (&entry == mSelectedEntry)
			text.setColor(sf::Color::White);

		text.move(0, text.getLocalBounds().height + 5);
	}
}