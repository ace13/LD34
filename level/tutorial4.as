void OnLoad()
{
	UI();
}

class UI
{
	UI()
	{
		print("New tutorial UI created.\n");

		Hooks::Add("Update", "update");
		Hooks::Add("DrawUI", "draw");

		Hooks::Add("OnLevelEnd", "levelEnd");
	}
	~UI()
	{
		print("Tutorial UI going 'Buh-bye'\n");
		
		unhook();
	}

	void levelEnd()
	{
		unhook();
		LoadLevel("Level1.lvl");
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
			page = (page + 1);
			if (page > 4)
				page = 4;
			time = 0;
		}
	}

	void draw(sf::Renderer@ rend)
	{
		sf::Text uiText;

		switch (page)
		{
			case 0:
				uiText.String = "Great work, time to give you a final challenge\nbefore we let you out into the wild.";
				break;

			case 1:
				uiText.String = "(You have to touch all the targets.)";
				break;

			case 2:
			case 3:
			case 4:
				uiText.String = "Here's a little recap;\n1 - Move forward\n0 - Stop moving\n0 0 - Slow forwards\n0 1 - Turn left\n1 0 - Turn right\n1 1 - Move backwards";
				break;
		}
		
		uiText.CharacterSize = 14;

		uiText.Move(sf::Vec2(10, 10));

		rend.Draw(uiText);
	}

	private uint page = 0;
	private float time = 0;
}
