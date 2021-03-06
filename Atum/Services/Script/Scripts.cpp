
#include "Scripts.h"
#include "Services/Core/Core.h"
#include "Libs/scriptarray.h"
#include "Libs/scriptdictionary.h"
#include "Libs/scripthandle.h"
#include "Libs/scriptmath.h"
#include "Libs/scriptstdstring.h"
#include "Services/Scene/Scene.h"
#include "Services/Scene/SceneObject.h"

Scripts scripts;
Scene* hack_scene = nullptr;

void MessageCallback(const asSMessageInfo *msg, void *param)
{
	const char *type = "ERR ";
	if (msg->type == asMSGTYPE_WARNING)
		type = "WARN";
	else if (msg->type == asMSGTYPE_INFORMATION)
		type = "INFO";

	core.Log("Script", "%s (%d, %d) : %s : %s\n", msg->section, msg->row, msg->col, type, msg->message);
}

void PrintString_Generic(asIScriptGeneric *gen)
{
	string* arg = (string*)(gen->GetArgAddress(0));
	core.Log("Script", arg->c_str());
}

void RenderPrint_Generic(asIScriptGeneric *gen)
{
	string* arg = (string*)(gen->GetArgAddress(2));
	render.DebugPrintText(Vector2(gen->GetArgFloat(0), gen->GetArgFloat(1)), COLOR_WHITE, arg->c_str());
}

void RenderDrawLine2D_Generic(asIScriptGeneric *gen)
{
	float scale = render.GetDevice()->GetHeight() / 1024.0f;
	Vector2 p1 = Vector2(gen->GetArgFloat(0), gen->GetArgFloat(1)) * scale - Sprite::cam_pos * scale + Vector2((float)render.GetDevice()->GetWidth(), (float)render.GetDevice()->GetHeight()) * 0.5f;
	Vector2 p2 = Vector2(gen->GetArgFloat(2), gen->GetArgFloat(3)) * scale - Sprite::cam_pos * scale + Vector2((float)render.GetDevice()->GetWidth(), (float)render.GetDevice()->GetHeight()) * 0.5f;

	render.DebugLine2D(p1, COLOR_WHITE, p2, COLOR_WHITE);
}

void RenderGetWidth_Generic(asIScriptGeneric *gen)
{
	gen->SetReturnDWord(render.GetDevice()->GetWidth());
}

void RenderGetHeight_Generic(asIScriptGeneric *gen)
{
	gen->SetReturnDWord(render.GetDevice()->GetHeight());
}

void Control_GetAliasIndex(asIScriptGeneric *gen)
{
	string* arg = (string*)(gen->GetArgAddress(0));
	gen->SetReturnDWord(controls.GetAlias(arg->c_str()));
}

void Control_GetState(asIScriptGeneric *gen)
{
	gen->SetReturnDWord(controls.GetAliasState(gen->GetArgDWord(0), (Controls::AliasAction)gen->GetArgDWord(1)));
}

void Control_GetValue(asIScriptGeneric *gen)
{
	gen->SetReturnFloat(controls.GetAliasValue(gen->GetArgDWord(0), (Controls::AliasAction)gen->GetArgDWord(1)));
}

void Control_GetDebugState(asIScriptGeneric *gen)
{
	string* arg = (string*)(gen->GetArgAddress(0));
	gen->SetReturnDWord(controls.DebugKeyPressed(arg->c_str(), (Controls::AliasAction)gen->GetArgDWord(1)));
}

void Control_IsGamepadConnected(asIScriptGeneric *gen)
{
	gen->SetReturnDWord((controls.IsGamepadConnected() > 0) ? 1 : 0);
}

void Scene_Group_SetState(asIScriptGeneric *gen)
{
	string* arg = (string*)(gen->GetArgAddress(0));
	int state = gen->GetArgDWord(1);

	Scene::Group& group = hack_scene->GetGroup(arg->c_str());

	for (auto obj : group.objects)
	{
		obj->SetState(state);
	}
}

void Scene_Raycast2D(asIScriptGeneric *gen)
{
	PhysScene::RaycastDesc rcdesc;

	float scale = 1.0f / 50.0f;

	rcdesc.origin = Vector(gen->GetArgFloat(0) * scale, -gen->GetArgFloat(1) * scale, 0.0f);
	rcdesc.dir = Vector(gen->GetArgFloat(2), -gen->GetArgFloat(3), 0.0f);
	rcdesc.length = gen->GetArgFloat(4) * scale;

	if (hack_scene->pscene->RayCast(rcdesc))
	{
		*((float*)(gen->GetArgAddress(5))) = rcdesc.hitPos.x * 50.0f;
		*((float*)(gen->GetArgAddress(6))) = -rcdesc.hitPos.y * 50.0f;

		*((float*)(gen->GetArgAddress(7))) = rcdesc.hitNormal.x;
		*((float*)(gen->GetArgAddress(8))) = -rcdesc.hitNormal.y;
		*((string*)(gen->GetArgAddress(9))) = rcdesc.userdata->object->GetName();
		*((int*)(gen->GetArgAddress(10))) = rcdesc.userdata->index;

		gen->SetReturnDWord(1);

		return;
	}

	gen->SetReturnDWord(0);
}

void IsPointInTriangle_Generic(asIScriptGeneric *gen)
{
	Vector2 s(gen->GetArgFloat(0), gen->GetArgFloat(1));
	Vector2 a(gen->GetArgFloat(2), gen->GetArgFloat(3));
	Vector2 b(gen->GetArgFloat(4), gen->GetArgFloat(5));
	Vector2 c(gen->GetArgFloat(6), gen->GetArgFloat(7));

	Vector2 an = a - s;
	Vector2 bn = b - s;
	Vector2 cn = c - s;

	bool orientation = an.Cross(bn) > 0;

	if ((bn.Cross(cn) > 0) != orientation)
	{
		gen->SetReturnDWord(0);
		return;
	}

	gen->SetReturnDWord((cn.Cross(an) > 0) == orientation ? 1 : 0);
}

void Scripts::Init()
{
}

void Scripts::Start()
{
	engine = asCreateScriptEngine();

	engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL);

	RegisterStdString(engine);
	RegisterScriptArray(engine, true);
	RegisterStdStringUtils(engine);
	RegisterScriptDictionary(engine);
	RegisterScriptHandle(engine);
	RegisterScriptMath(engine);

	engine->RegisterGlobalFunction("void Log(string&in text)", asFUNCTION(PrintString_Generic), asCALL_GENERIC);

	engine->RegisterGlobalFunction("void Render_Print(float x, float y, string&in text)", asFUNCTION(RenderPrint_Generic), asCALL_GENERIC);
	engine->RegisterGlobalFunction("void Render_DrawLine2D(float x, float y, float to_x, float to_y)", asFUNCTION(RenderDrawLine2D_Generic), asCALL_GENERIC);
	engine->RegisterGlobalFunction("int Render_GetWidth()", asFUNCTION(RenderGetWidth_Generic), asCALL_GENERIC);
	engine->RegisterGlobalFunction("int Render_GetHeight()", asFUNCTION(RenderGetHeight_Generic), asCALL_GENERIC);

	engine->RegisterGlobalFunction("int Control_GetAlias(string&in alias)", asFUNCTION(Control_GetAliasIndex), asCALL_GENERIC);
	engine->RegisterGlobalFunction("int Control_GetState(int alias, int action)", asFUNCTION(Control_GetState), asCALL_GENERIC);
	engine->RegisterGlobalFunction("float Control_GetValue(int alias, int delta)", asFUNCTION(Control_GetValue), asCALL_GENERIC);
	engine->RegisterGlobalFunction("int Control_GetDebugState(string&in alias, int action)", asFUNCTION(Control_GetDebugState), asCALL_GENERIC);
	engine->RegisterGlobalFunction("int Control_IsGamepadConnected()", asFUNCTION(Control_IsGamepadConnected), asCALL_GENERIC);

	engine->RegisterGlobalFunction("void Scene_Group_SetState(string&in alias, int state)", asFUNCTION(Scene_Group_SetState), asCALL_GENERIC);
	engine->RegisterGlobalFunction("int Scene_Raycast2D(float origin_x, float origin_y, float dir_x, float dir_y, float dist, float&out hit_y, float&out hit_x, float&out normal_x, float&out normal_y, string&out object, int&out index)", asFUNCTION(Scene_Raycast2D), asCALL_GENERIC);

	engine->RegisterGlobalFunction("int IsPointInTriangle(float pos_x, float pos_y, float p1_x, float p1_y, float p2_x, float p2_y, float p3_x, float p3_y)", asFUNCTION(IsPointInTriangle_Generic), asCALL_GENERIC);

	for (const auto& decl : ClassFactorySceneObject::Decls())
	{
		SceneObject* obj = decl->Create();
		obj->script_class_name = decl->GetShortName();
		obj->BindClassToScript();
		obj->Release();
	}

}

ScriptContext* Scripts::CreateContext()
{
	return new ScriptContext(engine->CreateContext());
}

void Scripts::Stop()
{
	engine->ShutDownAndRelease();
	engine = nullptr;
}