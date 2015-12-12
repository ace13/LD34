#pragma once

#include <functional>
#include <string>
#include <unordered_map>

class Robot;

class Program
{
public:
	virtual ~Program() { }

	virtual void execute(const std::string& command, Robot& actor);

protected:
	void addOpcode(const std::string& opcode, const std::function<void(Robot&)>& func);

private:
	std::unordered_map<std::string, std::function<void(Robot&)>> mOpcodes;
};

class BaseProgram : public Program
{
public:
	BaseProgram();
};