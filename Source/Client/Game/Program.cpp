#include "Program.hpp"
#include "Robot.hpp"

#include <Core/Math.hpp>

#include <iostream>


Program::Program()
{

}

Program::~Program()
{

}

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

Program* Program::createProgramming(const std::string& name)
{
	if (name == "BaseProgramming")
		return new BaseProgram();

	std::cout << "Unknown programming " << name << std::endl;
	return new BaseProgram();
}

void Program::addOpcode(const std::string& op, const std::string& name, const std::function<void(Robot&)>& func)
{
	mOpcodes[op] = { name, func };
}
void Program::eraseOpcode(const std::string& op)
{
	mOpcodes.erase(op);
}

BaseProgram::BaseProgram()
{
	addOpcode("0", "STOP", [](Robot& r) { r.setSpeed(0); });
	addOpcode("1", "FULL_FORWARD", [](Robot& r) { r.setSpeed(1); });

	addOpcode("00", "SLOW_FORWARD", [](Robot& r) { r.setSpeed(0.5); });
	addOpcode("01", "TURN_LEFT", [](Robot& r) { r.turn(-Math::PI2); });
	addOpcode("10", "TURN_RIGHT", [](Robot& r) { r.turn(Math::PI2); });
	addOpcode("11", "BACK_THAT_ASS_UP", [](Robot& r) { r.setSpeed(-0.5); });

	addOpcode("011", "TURN_AROUND", [](Robot& r) { r.turn(Math::PI); });

	addOpcode("1011", "MOONWALK", [](Robot& r) { r.turn(Math::PI * 2); r.setSpeed(-1); });
}

const std::string& BaseProgram::getName() const
{
	static const std::string name = "BaseProgram";
	return name;
}