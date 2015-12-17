UI@ t;

void OnLoad(const string&in)
{
	@t = UI();
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
		LoadLevel("Tutorial4.lvl");
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
				uiText.String = "Well done. You're clearly\ngetting the hang of this.";
				break;

			case 1:
				uiText.String = "Here's a bit more of a challenge for you.\nLead the robot through the \"maze\".";
				break;

			case 2:
				uiText.String = "(The command '0 1' will turn left)";
				break;

			case 3:
				uiText.String = "Here's a little recap;\n1 - Move forward\n0 - Stop moving\n0 1 - Turn left\n1 0 - Turn right";
				break;

			case 4:
				uiText.String = "(You can also use '1 1' to move backwards,\nor '0 0' to move forward carefully)";
				break;
		}
		
		uiText.CharacterSize = 14;

		uiText.Move(sf::Vec2(10, 10));

		rend.Draw(uiText);
	}

	private uint page = 0;
	private float time = 0;
}
