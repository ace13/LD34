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
		LoadLevel("Tutorial3.lvl");
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
				uiText.String = "Congratulations on your first success!\n<insert party noises here>";
				break;

			case 1:
				uiText.String = "This next task\nis a bit more difficult.";
				break;

			case 2:
				uiText.String = "You can't order the robot forwards with abandon,\nas it will run straight into a wall.";
				break;

			case 3:
				uiText.String = "Luckily there's a command the robot will understand that can help.\n'1 0' will cause the robot to turn right.";
				break;

			case 4:
				uiText.String = "(First type '1', then wait for a bit and type '1 0')";
				break;
		}
		
		uiText.CharacterSize = 14;

		uiText.Move(sf::Vec2(10, 10));

		rend.Draw(uiText);
	}

	private uint page = 0;
	private float time = 0;
}
