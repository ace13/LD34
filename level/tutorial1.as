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
		LoadLevel("Tutorial2.lvl");
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
				uiText.String = "Hello <subject name>,\nwelcome to BeatBots.";
				break;

			case 1:
				uiText.String = "You have been selected as\nrobot controllant #6852,\nfor your excellent ability in\n<subject skill>.";
				break;

			case 2:
				uiText.String = "Your task - as you've chosen to accept it\nis to control the robot seen in front of you.";
				break;

			case 3:
				uiText.String = "The robot is stupid, so you have to guide it.\nYou guide the robot by giving it commands,\nthe robot will execute them on a set interval.\n\nHence the timer in the corner.";
				break;

			case 4:
				uiText.String = "For now, just type a '1'.\nThe robot will see it as an order\nto go ahead and complete his task.\n\nThe lazy bastard.";
				break;
		}
		
		uiText.CharacterSize = 14;

		uiText.Move(sf::Vec2(10, 10));

		rend.Draw(uiText);
	}

	private uint page = 0;
	private float time = 0;
}
