NextLoad@ l;

void OnLoad(const string&in)
{
	@l = NextLoad();
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

		LoadLevel("Level7.lvl");
	}

	void unhook()
	{
		Hooks::Remove("OnLevelEnd");
	}
}
