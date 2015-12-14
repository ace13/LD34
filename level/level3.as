void OnLoad()
{
	NextLoad();
}

class NextLoad
{
	NextLoad()
	{
		Hooks::Add("OnLevelEnd", "levelEnd");
	}
	~NextLoad()
	{
		unhook();
	}

	void levelEnd()
	{
		unhook();

		LoadLevel("Level4.lvl");
	}

	void unhook()
	{
		Hooks::Remove("OnLevelEnd");
	}
}
