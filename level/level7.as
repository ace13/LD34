NextLoad@ l;

void OnLoad(const string&in)
{
	@l = NextLoad();
}

class NextLoad
{
	NextLoad()
	{
		Hooks::Add("Update", "update");
		Hooks::Add("DrawUI", "draw");
		Hooks::Add("OnLevelEnd", "levelEnd");
	}
	~NextLoad()
	{
		unhook();
	}

	void levelEnd()
	{
		unhook();

		LoadLevel("Tutorial1.lvl");
	}

	void unhook()
	{
		Hooks::Remove("OnLevelEnd");
		Hooks::Remove("DrawUI");
		Hooks::Remove("Update");
	}


	void update(const Timespan&in dt)
	{
		time += dt.Seconds;

		if (time > 10)
		{
			page = (page + 1) % 5;
			time = 0;
		}
	}

	void draw(sf::Renderer@ rend)
	{
		sf::Text uiText;

		switch (page)
		{
			case 0:
				uiText.String = "Well done, you've successfully completed my LD#34 entry.";
				break;

			case 1:
				uiText.String = "Hope you enjoyed this finite amount of rythm-controlled robots";
				break;

			case 2:
				uiText.String = "Hopefully you'll get to see more updates of this game in the future";
				break;

			case 3:
				uiText.String = "More types of robots, a \"branching\" storyline like World of Goo";
				break;

			case 4:
				uiText.String = "The sky's the limit really...\n\n\n(If you want a second go, just enter the goal)";
				break;
		}
		
		uiText.CharacterSize = 14;

		uiText.Move(sf::Vec2(10, 10));

		rend.Draw(uiText);
	}

	private uint page = 0;
	private float time = 0;
}
