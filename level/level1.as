void OnLoad()
{
	NextLoad();
}

class NextLoad
{
	NextLoad()
	{
		Hooks::Add("OnLevelEnd", "levelEnd");

		Hooks::Add("Update", "update");
		Hooks::Add("DrawUI", "draw");
	}
	~NextLoad()
	{
		unhook();
	}

	void levelEnd()
	{
		unhook();

		LoadLevel("Level2.lvl");
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
			page = (page + 1) % 3;
			time = 0;
		}
	}

	void draw(sf::Renderer@ rend)
	{
		sf::Text uiText;

		switch (page)
		{
			case 0:
				uiText.String = "Very well, you've proven your competence.\nWe have a proper task for you now.";
				break;

			case 1:
				uiText.String = "Several robots managed to get lost in the woods.\nWe want you to rescue them.";
				break;

			case 2:
				uiText.String = "Beware the rogue dozerbots that escaped into the woords a while back.\nThey're surprisingly good at crushing unwary robots.";
				break;
		}
		
		uiText.CharacterSize = 14;

		uiText.Move(sf::Vec2(10, 10));

		rend.Draw(uiText);
	}

	private float time = 0;
}
