#include "Program.hpp"
#include "Robot.hpp"

void Program::execute(const std::string& command, Robot& actor)
{
	if (mOpcodes.count(command))
		mOpcodes.at(command).Callback(actor);
}

std::string Program::getName(const std::string& opcode)
{
	if (mOpcodes.count(opcode))
		return mOpcodes.at(opcode).Name;
	return "";
}

void Program::addOpcode(const std::string& op, const std::string& name, const std::function<void(Robot&)>& func)
{
	mOpcodes[op] = { name, func };
}

BaseProgram::BaseProgram()
{
	addOpcode("0", "STOP", [](Robot& r) { r.setSpeed(0); });
	addOpcode("1", "MOVE", [](Robot& r) { r.setSpeed(1); });
	addOpcode("01", "TURN_LEFT",  [](Robot& r) { r.turn(3.14159 / -2); });
	addOpcode("10", "TURN_RIGHT", [](Robot& r) { r.turn(3.14159 / 2); });
	addOpcode("11", "BACK_THAT_ASS_UP", [](Robot& r) { r.setSpeed(-0.5); });
}