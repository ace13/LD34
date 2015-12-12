#include "Entity.hpp"

#include <Core/ScriptManager.hpp>

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

	return toRet;
}

Entity::Entity() :
	mScript(nullptr),
	mObject(nullptr),
	mRefCount(1)
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
		ctx->SetObject(mObject);
		ctx->SetArgObject(0, (Timespan*)&dt);

		ctx->Prepare(mUpdate);

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
		ctx->SetObject(mObject);
		ctx->SetArgObject(0, &target);
		ctx->SetUserData(&states, 0x5747);

		ctx->Prepare(mDraw);

		ctx->Execute();

		ctx->Unprepare();
		eng->ReturnContext(ctx);
	}
}

void Entity::setScriptObject(asIScriptObject* obj)
{
	if (mScript)
		return;

	mScript = obj->GetWeakRefFlag();
	mScript->AddRef();

	mObject = obj;
	mObject->AddRef();

	mDraw = mObject->GetObjectType()->GetMethodByDecl("void Draw(sf::Renderer@)");
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

		auto* mod = eng->GetModule("ScriptEntity", asGM_ALWAYS_CREATE);

		const std::string code(
			"shared abstract class ScriptEntity {\n"
			"	private EntityType_t @mObj;\n"
			"	EntityType_t@ opImplCast() { return mObj; }\n"
			"\n"
			"	ScriptEntity() {\n"
			"		@mObj = EntityType_t();\n"
			"	}\n"
			"\n"
			"	void Update(const Timespan&in) { }\n"
			"	void Draw(sf::Renderer@) { } \n"
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

		mod->AddScriptSection("ScriptEntity", code.c_str(), code.size());
		mod->Build();
	});
}

bool Entity::preLoadInject(asIScriptModule* mod)
{
	static const std::string code(
		"shared abstract class ScriptEntity {\n"
		"	EntityType_t@ opImplCast();\n"
		"	ScriptEntity();\n"
		"	void Update(const Timespan&in);\n"
		"	void Draw(sf::Renderer@);\n"
		"\n"
		"	sf::Vec2 Origin {\n"
		"		get const; set;\n"
		"	}\n"
		"	sf::Vec2 Position {\n"
		"		get const; set;\n"
		"	}\n"
		"	sf::Vec2 Scale {\n"
		"		get const; set;\n"
		"	}\n"
		"	float Rotation {\n"
		"		get const; set;\n"
		"	}\n"
		"\n"
		"	void Move(const sf::Vec2&in v);\n"
		"	void Scale(const sf::Vec2&in v);\n"
		"	void Rotate(float r);\n"
		"}\n"
		);

	return mod->AddScriptSection("ScriptEntity", code.c_str(), code.size()) >= 0;
}