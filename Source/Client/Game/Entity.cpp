#include "Entity.hpp"
#include "../States/GameState.hpp"

#include <Core/ScriptManager.hpp>

Entity* Entity::createForScript(asIScriptModule* module, const char* typeName)
{
	asIObjectType* type;
	if (!module || !(type = module->GetObjectTypeByName(typeName)))
	{
		return nullptr;
	}

	auto* eng = module->GetEngine();
	auto* func = type->GetFactoryByDecl((std::string(typeName) + " @" + typeName + "()").c_str());
	auto* ctx = eng->RequestContext();
	ctx->Prepare(func);
	ctx->SetUserData((void*)0x01, 0x1EC7);

	ctx->Execute();

	auto* obj = *(asIScriptObject**)ctx->GetAddressOfReturnValue();

	ctx->Unprepare();
	eng->ReturnContext(ctx);

	auto created = *reinterpret_cast<Entity**>(obj->GetAddressOfProperty(0));

	return created;
}

Entity* Entity::createFromScript()
{
	auto* ctx = asGetActiveContext();
	asIScriptFunction *func = ctx->GetFunction(0);
	if (func->GetObjectType() == 0 || std::string(func->GetObjectType()->GetName()) != "ScriptEntity")
	{
		ctx->SetException("You can't create this object manually");
		return nullptr;
	}

	Entity* toRet = new	Entity();
	toRet->setScriptObject((asIScriptObject*)ctx->GetThisPointer());

	ScriptManager* man = (ScriptManager*)ctx->GetEngine()->GetUserData(0x4547);
	man->addPersist((asIScriptObject*)ctx->GetThisPointer(), [toRet](asIScriptObject* newObj) {
		toRet->setScriptObject(newObj);
	});

	if (ctx->GetUserData(0x1EC7) != (void*)0x01)
		((GameState*)ctx->GetEngine()->GetUserData(0x64EE))->injectEntity(toRet);

	return toRet;
}

Entity::Entity() :
	mScript(nullptr),
	mObject(nullptr),
	mRefCount(1),
	mIsGoal(false),
	mIsCompleted(false)
{

}

Entity::~Entity()
{
	if (mScript)
	{
		mObject->Release();
		mScript->Release();
	}
}

void Entity::update(const Timespan& dt)
{
	if (mScript && !mScript->Get())
	{
		auto* eng = mObject->GetEngine();
		
		auto* ctx = eng->RequestContext();
		ctx->Prepare(mUpdate);

		ctx->SetObject(mObject);
		ctx->SetArgObject(0, (Timespan*)&dt);

		ctx->Execute();

		ctx->Unprepare();
		eng->ReturnContext(ctx);
	}
}

void Entity::tick(const Timespan& dt)
{
	if (mScript && !mScript->Get())
	{
		auto* eng = mObject->GetEngine();

		auto* ctx = eng->RequestContext();
		ctx->Prepare(mTick);

		ctx->SetObject(mObject);
		ctx->SetArgObject(0, (Timespan*)&dt);

		ctx->Execute();

		ctx->Unprepare();
		eng->ReturnContext(ctx);
	}
}


void Entity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (mScript && !mScript->Get())
	{
		auto* eng = mObject->GetEngine();

		states.transform *= getTransform();

		auto* ctx = eng->RequestContext();
		ctx->Prepare(mDraw);

		ctx->SetObject(mObject);
		ctx->SetArgObject(0, &target);
		ctx->SetUserData(&states, 0x5747);

		ctx->Execute();

		ctx->Unprepare();
		eng->ReturnContext(ctx);
	}
}

bool Entity::isGoal() const
{
	return mIsGoal;
}
bool Entity::isCompleted() const
{
	return mIsCompleted;
}

void Entity::setGoal(bool isGoal)
{
	mIsGoal = isGoal;
}
void Entity::setCompleted(bool completed)
{
	mIsCompleted = completed;
}

void Entity::setScriptObject(asIScriptObject* obj)
{
	if (mScript)
	{
		auto oldObjProp = mObject->GetAddressOfProperty(0);
		auto newObjProp = obj->GetAddressOfProperty(0);

		// Move the entity pointer over to the new object
		*reinterpret_cast<Entity**>(newObjProp) = *reinterpret_cast<Entity**>(oldObjProp);
		*reinterpret_cast<Entity**>(oldObjProp) = nullptr;

		mObject->Release();
		mObject = nullptr;

		mScript->Release();
		mScript = nullptr;
	}

	mScript = obj->GetWeakRefFlag();
	mScript->AddRef();

	mObject = obj;
	mObject->AddRef();

	mDraw = mObject->GetObjectType()->GetMethodByDecl("void Draw(sf::Renderer@)");
	mTick = mObject->GetObjectType()->GetMethodByDecl("void Tick(const Timespan&in)");
	mUpdate = mObject->GetObjectType()->GetMethodByDecl("void Update(const Timespan&in)");
}

int Entity::addRef()
{
	if (!mScript)
		return 1;

	if (!mScript->Get())
		mObject->AddRef();

	return ++mRefCount;
}

int Entity::release()
{
	if (!mScript)
		return 1;

	if (!mScript->Get())
		mObject->Release();

	int refs = --mRefCount;
	if (refs == 0)
		delete this;

	return refs;
}

namespace
{
	const std::string ScriptEntityCode(
		"shared abstract class ScriptEntity {\n"
		"	private EntityType_t @mObj;\n"
		"	EntityType_t@ opImplCast() { return mObj; }\n"
		"\n"
		"	ScriptEntity() {\n"
		"		@mObj = EntityType_t();\n"
		"	}\n"
		"\n"
		"	void Tick(const Timespan&in) { print(\"I am base\\n\"); }\n"
		"	void Update(const Timespan&in) { }\n"
		"	void Draw(sf::Renderer@) { } \n"
		"\n"
		"	bool Completed {\n"
		"		get const { return mObj.Completed; }\n"
		"		set { mObj.Completed = value; }\n"
		"	}\n"
		"	bool Goal {\n"
		"		get const { return mObj.Goal; }\n"
		"		set { mObj.Goal = value; }\n"
		"	}\n"
		"\n"
		"	sf::Vec2 Origin {\n"
		"		get const { return mObj.Origin; }\n"
		"		set { mObj.Origin = value; }\n"
		"	}\n"
		"	sf::Vec2 Position {\n"
		"		get const { return mObj.Position; }\n"
		"		set { mObj.Position = value; }\n"
		"	}\n"
		"	sf::Vec2 Scale {\n"
		"		get const { return mObj.Scale; }\n"
		"		set { mObj.Scale = value; }\n"
		"	}\n"
		"	float Rotation {\n"
		"		get const { return mObj.Rotation; }\n"
		"		set { mObj.Rotation = value; }\n"
		"	}\n"
		"\n"
		"	void Move(const sf::Vec2&in v) { mObj.Move(v); }\n"
		"	void Scale(const sf::Vec2&in v) { mObj.Scale(v); }\n"
		"	void Rotate(float r) { mObj.Rotate(r); }\n"
		"}\n"
		);
}

void Entity::registerType(ScriptManager& man)
{
	man.addExtension("Entity", [](asIScriptEngine* eng) {
		AS_ASSERT(eng->RegisterObjectType("EntityType_t", 0, asOBJ_REF));

		AS_ASSERT(eng->RegisterObjectBehaviour("EntityType_t", asBEHAVE_FACTORY, "EntityType_t @f()", asFUNCTION(Entity::createFromScript), asCALL_CDECL));
		AS_ASSERT(eng->RegisterObjectBehaviour("EntityType_t", asBEHAVE_ADDREF, "void f()", asMETHOD(Entity, addRef), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectBehaviour("EntityType_t", asBEHAVE_RELEASE, "void f()", asMETHOD(Entity, release), asCALL_THISCALL));

		// sf::Transformable
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "const sf::Vec2& get_Origin() const", asMETHODPR(Entity, getOrigin, () const, const sf::Vector2f&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Origin(const sf::Vec2&in)", asMETHODPR(Entity, setOrigin, (const sf::Vector2f&), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "const sf::Vec2& get_Position() const", asMETHODPR(Entity, getPosition, () const, const sf::Vector2f&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Position(const sf::Vec2&in)", asMETHODPR(Entity, setPosition, (const sf::Vector2f&), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "const sf::Vec2& get_Scale() const", asMETHODPR(Entity, getScale, () const, const sf::Vector2f&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Scale(const sf::Vec2&in)", asMETHODPR(Entity, setScale, (const sf::Vector2f&), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "float get_Rotation() const", asMETHODPR(Entity, getRotation, () const, float), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Rotation(float)", asMETHODPR(Entity, setRotation, (float), void), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void Move(const sf::Vec2&in)", asMETHODPR(Entity, move, (const sf::Vector2f&), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void Rotate(float)", asMETHODPR(Entity, rotate, (float), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void Scale(const sf::Vec2&in)", asMETHODPR(Entity, scale, (const sf::Vector2f&), void), asCALL_THISCALL));

		// Entity
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "bool get_Completed() const", asMETHOD(Entity, isCompleted), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Completed(bool=true)", asMETHOD(Entity, setCompleted), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "bool get_Goal() const", asMETHOD(Entity, isGoal), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Goal(bool=true)", asMETHOD(Entity, setGoal), asCALL_THISCALL));

		auto* mod = eng->GetModule("ScriptEntity", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("ScriptEntity", ScriptEntityCode.c_str(), ScriptEntityCode.size());
		mod->Build();
	});
}

bool Entity::preLoadInject(asIScriptModule* mod)
{
	return mod->AddScriptSection("ScriptEntity", ScriptEntityCode.c_str(), ScriptEntityCode.size()) >= 0;
}