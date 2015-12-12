LD34
=====

My Ludum Dare 34 entry.

The themes are *Two Button Controls* and *Growing*

Point of the game
-----------------

My game ended up sort of like a rythm-based programming game.

You control a robot that performs a series of tasks, all by passing it instructions in the form of variable length opcodes.
The opcodes start at one bit and move up to a full word.

Example Opcodes
---------------

- 0 - Stop
- 1 - Move forward
- 01 - Turn left
- 10 - Turn right
- 11 - Move backwards

Dependencies
============

- CMake
- SFML 3.2
- Angelscript 2.30
