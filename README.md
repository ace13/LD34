BeatBots
========

[![Travis Badge](https://travis-ci.org/ace13/LD34.svg?branch=master)](https://travis-ci.org/ace13/LD34) | [Ludum Dare entry](http://ludumdare.com/compo/ludum-dare-34/?action=preview&uid=10247)

This game was made as part of the Ludum Dare #34 compo, under 48 hours.
Expect quality along those lines.

The themes provided were *Growing* and *Two Button Controls*, though this game only uses one at the moment.
Hopeful planning involves adding levels with growing input possibilites in the future, to manage to fit both the themes in.

Point of the game
-----------------

The game ended up as a sort of rythm-based programming game.

You control a robot that performs a series of tasks, moving mostly, by passing it instructions in the form of variable length opcodes.
The opcodes vary in length between one and four bits, and are documented partially below;
- 0 - Stop
- 1 - Move forward
- 00 - Slow forwards
- 01 - Turn left
- 10 - Turn right
- 11 - Move backwards
- 011 - Turn around
- 1011 - Do a moonwalk

Planned is for three bit opcodes to be the extending set of codes you'll aquire during the game, while four-bit ones are for robot specific features. Turning a tower on a tank-like robot, or possibly strafing and the like on robots supporting such features.

Dependencies
============

This project uses the following libraries, with noted versions on which it has been tested and proven to work. Other versions might also work, but there has been no testing done to confirm so.

- CMake
- SFML (Tested with 2.3)
- Angelscript (Tested with 2.30.2)
