#include "Program.hpp"
#include "Robot.hpp"

#include <Core/Math.hpp>

bool Program::execute(const std::string& command, Robot& actor)
{
	if (mOpcodes.count(command))
	{
		mOpcodes.at(command).Callback(actor);
		return true;
	}

	return false;
}

const std::string& Program::getName(const std::string& opcode) const
{
	static const std::string Invalid = "";
	static const std::string Nop = "NOP";

	if (mOpcodes.count(opcode))
		return mOpcodes.at(opcode).Name;
	else if (opcode.empty())
		return Invalid;
	else
		return Nop;
}

void Program::addOpcode(const std::string& op, const std::string& name, const std::function<void(Robot&)>& func)
{
	mOpcodes[op] = { name, func };
}

BaseProgram::BaseProgram()
{
	addOpcode("0", "STOP", [](Robot& r) { r.setSpeed(0); });
	addOpcode("1", "MOVE", [](Robot& r) { r.setSpeed(1); });
	addOpcode("01", "TURN_LEFT", [](Robot& r) { r.turn(Math::PI / -2); });
	addOpcode("10", "TURN_RIGHT", [](Robot& r) { r.turn(Math::PI / 2); });
	addOpcode("11", "BACK_THAT_ASS_UP", [](Robot& r) { r.setSpeed(-0.5); });
}