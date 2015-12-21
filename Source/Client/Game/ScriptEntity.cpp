#include "ScriptEntity.hpp"
#include "Level.hpp"

#include <Core/Engine.hpp>
#include <Core/ScriptManager.hpp>

ScriptEntity* ScriptEntity::createForScript(asIScriptModule* module, const std::string& typeName)
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

	auto created = *reinterpret_cast<ScriptEntity**>(obj->GetAddressOfProperty(0));

	return created;
}

ScriptEntity* ScriptEntity::createFromScript()
{
	auto* ctx = asGetActiveContext();
	asIScriptFunction *func = ctx->GetFunction(0);
	if (func->GetObjectType() == 0 || std::string(func->GetObjectType()->GetName()) != "ScriptEntity")
	{
		ctx->SetException("You can't create this object manually");
		return nullptr;
	}

	ScriptEntity* toRet = new ScriptEntity();
	toRet->setScriptObject((asIScriptObject*)ctx->GetThisPointer());

	if (ctx->GetUserData(0x1EC7) != (void*)0x01)
		((Level*)ctx->GetEngine()->GetUserData(0x1EE7))->addEntity(toRet);

	return toRet;
}


ScriptEntity::ScriptEntity() :
	mScript(nullptr),
	mObject(nullptr),
	mTick(nullptr),
	mUpdate(nullptr),
	mDraw(nullptr)
{

}
ScriptEntity::~ScriptEntity()
{
	setScriptObject(nullptr);
}

void ScriptEntity::update(const Timespan& dt)
{
	if (!mScript->Get())
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

void ScriptEntity::tick(const Timespan& dt)
{
	if (!mScript->Get())
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


void ScriptEntity::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (!mScript->Get())
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

bool ScriptEntity::serialize(OutputStream& stream) const
{
	return true;

	// TODO: FIXME
	/*
	if (!mScript->Get())
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
	*/
}
bool ScriptEntity::deserialize(InputStream& data)
{
	return true;
	// TODO: FIXME
	/*
	if (!mScript->Get())
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
	*/
}

void ScriptEntity::initialize()
{

}

const std::type_info& ScriptEntity::getType() const
{
	return typeid(ScriptEntity);
}

const std::string& ScriptEntity::getName() const
{
	return mName;
}


const asIScriptObject* ScriptEntity::getScriptObject() const
{
	return mObject;
}

void ScriptEntity::setScriptObject(asIScriptObject* obj)
{
	auto& man = getEngine()->get<ScriptManager>();

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
		*reinterpret_cast<ScriptEntity**>(newObjProp) = *reinterpret_cast<ScriptEntity**>(oldObjProp);
		*reinterpret_cast<ScriptEntity**>(oldObjProp) = nullptr;

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
		"	void Move(float x, float y) final { mObj.Move(x,y); }\n"
		"	void Move(const sf::Vec2&in v) final { mObj.Move(v); }\n"
		"	void Scale(const sf::Vec2&in v) final { mObj.Scale(v); }\n"
		"	void Rotate(float r) final { mObj.Rotate(r); }\n"
		"\n"
		"	bool IsBlocked(const sf::Vec2&in v) const final { return mObj.IsBlocked(v); }\n"
		"}\n"
	);

	bool checkBlock(Entity* ent, const sf::Vector2f& pos)
	{
		float scale = ent->getLevel()->getScale();
		return ent->getLevel()->isBlocked(uint8_t(pos.x / scale), uint8_t(pos.y / scale));
	}
}

void ScriptEntity::registerType(ScriptManager& man)
{
	man.addExtension("Entity", [](asIScriptEngine* eng) {
		AS_ASSERT(eng->RegisterObjectType("EntityType_t", 0, asOBJ_REF));

		AS_ASSERT(eng->RegisterObjectBehaviour("EntityType_t", asBEHAVE_FACTORY, "EntityType_t @f()", asFUNCTION(ScriptEntity::createFromScript), asCALL_CDECL));
		AS_ASSERT(eng->RegisterObjectBehaviour("EntityType_t", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptEntity, addRef), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectBehaviour("EntityType_t", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptEntity, release), asCALL_THISCALL));

		// sf::Transformable
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "const sf::Vec2& get_Origin() const", asMETHODPR(ScriptEntity, getOrigin, () const, const sf::Vector2f&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Origin(const sf::Vec2&in)", asMETHODPR(ScriptEntity, setOrigin, (const sf::Vector2f&), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "const sf::Vec2& get_Position() const", asMETHODPR(ScriptEntity, getPosition, () const, const sf::Vector2f&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Position(const sf::Vec2&in)", asMETHODPR(ScriptEntity, setPosition, (const sf::Vector2f&), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "const sf::Vec2& get_Scale() const", asMETHODPR(ScriptEntity, getScale, () const, const sf::Vector2f&), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Scale(const sf::Vec2&in)", asMETHODPR(ScriptEntity, setScale, (const sf::Vector2f&), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "float get_Rotation() const", asMETHODPR(ScriptEntity, getRotation, () const, float), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Rotation(float)", asMETHODPR(ScriptEntity, setRotation, (float), void), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void Move(float,float)", asMETHODPR(ScriptEntity, move, (float,float), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void Move(const sf::Vec2&in)", asMETHODPR(ScriptEntity, move, (const sf::Vector2f&), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void Rotate(float)", asMETHODPR(ScriptEntity, rotate, (float), void), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void Scale(const sf::Vec2&in)", asMETHODPR(ScriptEntity, scale, (const sf::Vector2f&), void), asCALL_THISCALL));

		// Entity
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "float get_Radius() const", asMETHOD(ScriptEntity, getRadius), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Radius(float)", asMETHOD(ScriptEntity, setRadius), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "bool get_Completed() const", asMETHOD(ScriptEntity, isCompleted), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Completed(bool=true)", asMETHOD(ScriptEntity, setCompleted), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "bool get_Goal() const", asMETHOD(ScriptEntity, isGoal), asCALL_THISCALL));
		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "void set_Goal(bool=true)", asMETHOD(ScriptEntity, setGoal), asCALL_THISCALL));

		AS_ASSERT(eng->RegisterObjectMethod("EntityType_t", "bool IsBlocked(const sf::Vec2&in) const", asFUNCTION(checkBlock), asCALL_CDECL_OBJFIRST));

		// Base Entity
		auto* mod = eng->GetModule("!!ScriptEntity!!", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("!!ScriptEntity!!", ScriptEntityCode.c_str(), ScriptEntityCode.size());
		mod->Build();
	});
}

bool ScriptEntity::preLoadInject(asIScriptModule* mod)
{
	int r = mod->AddScriptSection("ScriptEntity", ScriptEntityCode.c_str(), ScriptEntityCode.size());
	return (r >= 0);
}
