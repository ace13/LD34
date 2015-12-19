#pragma once

#include <functional>
#include <string>
#include <unordered_map>

class Robot;

class Program
{
public:
	virtual ~Program() { }

	virtual const std::string& getName() const = 0;

	virtual bool execute(const std::string& command, Robot& actor);
	virtual const std::string& getName(const std::string& opcode) const;

	static Program* createProgramming(const std::string& name);

protected:

	void addOpcode(const std::string& opcode, const std::string& name, const std::function<void(Robot&)>& func);
	void eraseOpcode(const std::string& op);

private:
	struct OpCode
	{
		std::string Name;
		std::function<void(Robot&)> Callback;
	};

	std::unordered_map<std::string, OpCode> mOpcodes;
};

class BaseProgram : public Program
{
public:
	BaseProgram();

	const std::string& getName() const;
};