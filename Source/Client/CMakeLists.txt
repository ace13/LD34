SET(HEADERS
	Application.hpp
	ParticleManager.hpp
	ResourceManager.hpp
	ResourceManager.inl
	StateMachine.hpp
	StateMachine.inl

	Game/Entity.hpp
	Game/Enemy.hpp
	Game/Goal.hpp
	Game/Level.hpp
	Game/Program.hpp
	Game/Robot.hpp

	Menus/MenuPage.hpp
	Menus/InfoMenu.hpp
	Menus/InGameMenu.hpp
	Menus/MainMenu.hpp
	Menus/OptionMenu.hpp

	States/IState.hpp
	States/GameState.hpp
	States/IntroState.hpp
	States/MenuState.hpp
)

SET(SOURCES
	Application.cpp
	ParticleManager.cpp
	ResourceManager.cpp
	StateMachine.cpp

	Game/Entity.cpp
	Game/Enemy.cpp
	Game/Goal.cpp
	Game/Level.cpp
	Game/Program.cpp
	Game/Robot.cpp

	Menus/MenuPage.cpp
	Menus/InfoMenu.cpp
	Menus/InGameMenu.cpp
	Menus/MainMenu.cpp
	Menus/OptionMenu.cpp
	
	States/GameState.cpp
	States/IntroState.cpp
	States/MenuState.cpp

	main.cpp
)

SOURCE_GROUP("Header files\\Game" REGULAR_EXPRESSION "Game/[^./]+.(hpp|inl)")
SOURCE_GROUP("Source files\\Game" REGULAR_EXPRESSION "Game/[^./]+.cpp")
SOURCE_GROUP("Header files\\Menus" REGULAR_EXPRESSION "Menus/[^./]+.(hpp|inl)")
SOURCE_GROUP("Source files\\Menus" REGULAR_EXPRESSION "Menus/[^./]+.cpp")
SOURCE_GROUP("Header files\\States" REGULAR_EXPRESSION "States/[^./]+.(hpp|inl)")
SOURCE_GROUP("Source files\\States" REGULAR_EXPRESSION "States/[^./]+.cpp")

SOURCE_GROUP("Header files\\" REGULAR_EXPRESSION "[^./]+.(hpp|inl)")
SOURCE_GROUP("Source files\\" REGULAR_EXPRESSION "[^./]+.cpp")

ADD_EXECUTABLE(Client ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(Client
	Core
)
