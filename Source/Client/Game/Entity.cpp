#include "Entity.hpp"
#include "Door.hpp"
#include "Enemy.hpp"
#include "Goal.hpp"
#include "Key.hpp"
#include "Level.hpp"
#include "Box.hpp"
#include "Pit.hpp"

#include <Core/Engine.hpp>
#include <Core/Math.hpp>
#include <Core/ScriptManager.hpp>

Entity* Entity::createFromType(const std::string& type)
{
	Entity* toRet = nullptr;

	if (type == "Goal")
		toRet = new Goal();
	else if (type == "BasicEnemy")
		toRet = new Enemy();
	else if (type == "Key")
		toRet = new Key();
	else if (type == "Door")
		toRet = new Door();
	else if (type == "Pit")
		toRet = new Pit();
	else if (type == "Box")
		toRet = new Box();
	else if (type == "Player")
		toRet = new Robot();

	return toRet;
}

Entity* Entity::createForScript(asIScriptModule* module, const std::string& typeName)
{
	asIObjectType* type;
	if (!module || !(type = module->GetObjectTypeByName(typeName.c_str())))
	{
		return nullptr;
	}

	auto* eng = module->GetEngine();
	auto* func = type->GetFactoryByDecl((typeName + " @" + typeName + "()").c_str());
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

	if (ctx->GetUserData(0x1EC7) != (void*)0x01)
		((Level*)ctx->GetEngine()->GetUserData(0x1EE7))->addEntity(toRet);

	return toRet;
}

Entity::Entity() :
	mLevel(nullptr),
	mScript(nullptr),
	mObject(nullptr),
	mIsGoal(false),
	mIsCompleted(false),
	mRadius(1),
	mRefCount(1)
{

}

Entity::~Entity()
{
	if (mScript)
	{
		if (!mScript->Get())
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

void Entity::move(const sf::Vector2f& vec)
{
	auto checkPos = (getPosition() + (Math::Normalized(vec) * getRadius()) + vec) / mLevel->getScale();
	if (!mLevel->isBlocked(uint8_t(checkPos.x), uint8_t(checkPos.y)))
		sf::Transformable::move(vec);
}

std::string Entity::serialize() const
{
	if (mScript && !mScript->Get())
	{
		auto* eng = mObject->GetEngine();

		auto* ctx = eng->RequestContext();
		ctx->Prepare(mObject->GetObjectType()->GetMethodByDecl("string Serialize() const"));

		ctx->SetObject(mObject);

		ctx->Execute();

		std::string ret = *(std::string*)ctx->GetReturnObject();

		ctx->Unprepare();
		eng->ReturnContext(ctx);

		return ret;
	}

	return "";
}
bool Entity::deserialize(const std::string& data)
{
	if (mScript && !mScript->Get())
	{
		auto* eng = mObject->GetEngine();

		auto* ctx = eng->RequestContext();
		ctx->Prepare(mObject->GetObjectType()->GetMethodByDecl("bool Deserialize(const string&in)"));

		ctx->SetObject(mObject);
		ctx->SetArgObject(0, (std::string*)&data);

		ctx->Execute();

		bool ret = ctx->GetReturnByte() != 0;

		ctx->Unprepare();
		eng->ReturnContext(ctx);

		return ret;
	}

	return true;
}

void Entity::initialize()
{

}

const std::string& Entity::getName() const
{
	static std::string defaultName = "Entity";

	if (mScript)
		return mName;
	return defaultName;
}

bool Entity::isScriptEntity() const
{
	return (mScript != nullptr);
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

float Entity::getRadius() const
{
	return mRadius;
}
void Entity::setRadius(float r)
{
	mRadius = r;
}

const ParticleManager* Entity::getParticleManager(bool post) const
{
	return mLevel->getParticleManager(post);
}
ParticleManager* Entity::getParticleManager(bool post)
{
	return mLevel->getParticleManager(post);
}

const Level* Entity::getLevel() const
{
	return mLevel;
}
Level* Entity::getLevel()
{
	return mLevel;
}
void Entity::setLevel(Level* l)
{
	mLevel = l;
}

const asIScriptObject* Entity::getScriptObject() const
{
	return mObject;
}

void Entity::setScriptObject(asIScriptObject* obj)
{
	auto& man = getLevel()->getEngine()->get<ScriptManager>();

	if (!obj)
	{
		if (!mScript->Get())
			mObject->Release();
		mScript->Release();

		man.removeChangeNotice(mObject);

		mScript = nullptr;
		mObject = nullptr;

		mDraw = nullptr;
		mTick = nullptr;
		mUpdate = nullptr;

		mName.clear();

		return;
	}

	if (mScript)
	{
		auto oldObjProp = mObject->GetAddressOfProperty(0);
		auto newObjProp = obj->GetAddressOfProperty(0);

		// Move the entity pointer over to the new object
		*reinterpret_cast<Entity**>(newObjProp) = *reinterpret_cast<Entity**>(oldObjProp);
		*reinterpret_cast<Entity**>(oldObjProp) = nullptr;

		man.removeChangeNotice(mObject);

		mObject->Release();
		mObject = nullptr;

		mScript->Release();
		mScript = nullptr;
	}

	mScript = obj->GetWeakRefFlag();
	mScript->AddRef();

	mObject = obj;
	mObject->AddRef();

	mName = mObject->GetObjectType()->GetName();

	mDraw = mObject->GetObjectType()->GetMethodByDecl("void Draw(sf::Renderer@)");
	mTick = mObject->GetObjectType()->GetMethodByDecl("void Tick(const Timespan&in)");
	mUpdate = mObject->GetObjectType()->GetMethodByDecl("void Update(const Timespan&in)");

	man.addChangeNotice(mObject, [this](asIScriptObject* newObj) {
		setScriptObject(newObj);
	});
}

int Entity::addRef()
{
	if (mScript && !mScript->Get())
		mObject->AddRef();

	return ++mRefCount;
}

int Entity::release()
{
	if (mScript && !mScript->Get())
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
		"	string Serialize() const { return \"\"; }\n"
		"	bool Deserialize(const string&in) { return true; }\n"
		"\n"
		"	float Radius {\n"
		"		get const final { return mObj.Radius; }\n"
		"		set final { mObj.Radius = value; }\n"
		"	}\n"
		"	bool Completed {\n"
		"		get const final { return mObj.Completed; }\n"
		"		set final { mObj.Completed = value; }\n"
		"	}\n"
		"	bool Goal {\n"
		"		get const final { return mObj.Goal; }\n"
		"		set final { mObj.Goal = value; }\n"
		"	}\n"
		"\n"
		"	const sf::Vec2& get_Origin() const final { return mObj.Origin; }\n"
		"	void set_Origin(const sf::Vec2&in v) final { mObj.Origin = v; }\n"
		"	const sf::Vec2& get_Position() const final { return mObj.Position; }\n"
		"	void set_Position(const sf::Vec2&in v) final { mObj.Position = v; }\n"
		"	const sf::Vec2& get_Scale() const final { return mObj.Scale; }\n"
		"	void set_Scale(const sf::Vec2&in v) final { mObj.Scale = v; }\n"
		"	float Rotation {\n"
		"		get const final { return mObj.Rotation; }\n"
		"		set final { mObj.Rotation = value; }\n"
		"	}\n"
		"\n"
		"	\n"
		"	void Move(const sf::Vec2&in v) final { mObj.Move(v); }\n"
		"	void Scale(const sf::Vec2&in v) final { mObj.Scale(v); }\n"
		"	void Rotate(float r) final { mObj.Rotate(r); }\n"
		"	bool IsBlocked(const sf::Vec2&in v) const final { return mObj.IsBlocked(v); }\n"
		"}\n"
		);
}

namespace
{
	bool checkBlock(Entity* ent, const sf::Vector2f& pos)
	{
		float scale = ent->getLevel()->getScale();
		return ent->getLevel()->isBlocked(uint8_t(pos.x / scale), uint8_t(pos.y / scale));
	}
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
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "float get_Radius() const", asMETHOD(Entity, getRadius), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Radius(float)", asMETHOD(Entity, setRadius), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "bool get_Completed() const", asMETHOD(Entity, isCompleted), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Completed(bool=true)", asMETHOD(Entity, setCompleted), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "bool get_Goal() const", asMETHOD(Entity, isGoal), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Goal(bool=true)", asMETHOD(Entity, setGoal), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "bool IsBlocked(const sf::Vec2&in) const", asFUNCTION(checkBlock), asCALL_CDECL_OBJFIRST));

		// Base Entity
		auto* mod = eng->GetModule("ScriptEntity", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("ScriptEntity", ScriptEntityCode.c_str(), ScriptEntityCode.size());
		mod->Build();
	});
}

bool Entity::preLoadInject(asIScriptModule* mod)
{
	return mod->AddScriptSection("ScriptEntity", ScriptEntityCode.c_str(), ScriptEntityCode.size()) >= 0;
}
