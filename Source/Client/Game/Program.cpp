#include "Program.hpp"
#include "Robot.hpp"

void Program::execute(const std::string& command, Robot& actor)
{
	if (mOpcodes.count(command))
		mOpcodes.at(command)(actor);
}

void Program::addOpcode(const std::string& op, const std::function<void(Robot&)>& func)
{
	mOpcodes[op] = func;
}

BaseProgram::BaseProgram()
{
	addOpcode("0", [](Robot& r) { r.setSpeed(0); });
	addOpcode("1", [](Robot& r) { r.setSpeed(1); });
	addOpcode("01", [](Robot& r) { r.turn(3.14159 / -2); });
	addOpcode("10", [](Robot& r) { r.turn(3.14159 / 2); });
	addOpcode("11", [](Robot& r) { r.setSpeed(-0.5); });
}