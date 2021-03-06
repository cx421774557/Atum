#include "SceneScript.h"
#include "Services/Core/Core.h"

#ifdef EDITOR

#include "Editor/EditorDrawer.h"

#endif

#include "Services/Script/Libs/scriptarray.h"

CLASSREG(SceneObject, SceneScript, "Script")

META_DATA_DESC(SceneScript::NodeSceneObject)
STRING_PROP(SceneScript::NodeSceneObject, name, "", "Property", "Name")
META_DATA_DESC_END()

META_DATA_DESC(SceneScript::NodeScriptProperty)
STRING_PROP(SceneScript::NodeScriptProperty, name, "", "Property", "Name")
META_DATA_DESC_END()

META_DATA_DESC(SceneScript::LinkToMethod)
STRING_PROP(SceneScript::LinkToMethod, param, "", "Property", "Params")
META_DATA_DESC_END()

META_DATA_DESC(SceneScript::NodeScriptMethod)
STRING_PROP(SceneScript::NodeScriptMethod, name, "", "Property", "Name")
ENUM_PROP(SceneScript::NodeScriptMethod, param_type, 0, "Property", "ParamType")
	ENUM_ELEM("None", 0)
	ENUM_ELEM("Int", 1)
	ENUM_ELEM("String", 2)
	ENUM_ELEM("EveryFrame", 3)
ENUM_END
META_DATA_DESC_END()

#ifdef EDITOR
void StartScriptEdit(void* owner)
{
	SceneScript* script = (SceneScript*)owner;

	string filename;
	script->GetScriptFileName(script->GetUID(), filename);

	ShellExecuteA(nullptr, "open", filename.c_str(), NULL, NULL, SW_SHOW);
}
#endif

META_DATA_DESC(SceneScript)
BASE_SCENE_OBJ_PROP(SceneScript)
STRING_PROP(SceneScript, main_class, "", "Prop", "main_class")
#ifdef EDITOR
CALLBACK_PROP(SpriteAsset, StartScriptEdit, "Prop", "EditScript")
#endif
META_DATA_DESC_END()

void SceneScript::Node::Load(JSONReader& loader)
{
	loader.Read("name", name);
	loader.Read("pos", pos);
}

void SceneScript::Node::Save(JSONWriter& saver)
{
	saver.Write("type", (int&)type);
	saver.Write("name", name.c_str());
	saver.Write("pos", pos);
}

void SceneScript::NodeSceneObject::Load(JSONReader& loader)
{
	Node::Load(loader);

	loader.Read("object_uid", object_uid);
	loader.Read("object_child_uid", object_child_uid);
}

void SceneScript::NodeSceneObject::Save(JSONWriter& saver)
{
	Node::Save(saver);

	saver.Write("object_uid", object_uid);
	saver.Write("object_child_uid", object_child_uid);
}

void SceneScript::NodeScriptMethod::Load(JSONReader& loader)
{
	Node::Load(loader);

	loader.Read("param_type", param_type);

	int link_count = 0;
	loader.Read("Count", link_count);
	links.resize(link_count);

	for (auto& link : links)
	{
		loader.EnterBlock("Link");

		link = new LinkToMethod();

		loader.Read("node", link->node);
		loader.Read("param", ((LinkToMethod*)link)->param);

		loader.LeaveBlock();
	}
}

void SceneScript::NodeScriptMethod::Save(JSONWriter& saver)
{
	Node::Save(saver);

	saver.Write("param_type", param_type);

	int link_count = (int)links.size();
	saver.Write("Count", link_count);

	saver.StartArray("Link");

	for (auto link : links)
	{
		saver.StartBlock(nullptr);

		saver.Write("node", link->node);
		saver.Write("param", ((LinkToMethod*)link)->param.c_str());

		saver.FinishBlock();
	}

	saver.FinishArray();
}

void SceneScript::GetScriptFileName(uint32_t id,string& filename)
{
	char str[1024];

	StringUtils::Printf(str, 1024, "%s%u.sns", owner->GetScenePath(), id);
	filename = str;
}

void SceneScript::Init()
{
	Tasks(false)->AddTask(-100, this, (Object::Delegate)&SceneScript::Work);

#ifdef EDITOR
	RenderTasks(true)->AddTask(ExecuteLevels::Sprites, this, (Object::Delegate)&SceneScript::EditorWork);
#endif
}

void SceneScript::Load(JSONReader& loader)
{
	GetMetaData()->Prepare(this);
	GetMetaData()->Load(loader);

	int count = 0;
	loader.Read("Count", count);
	nodes.resize(count);

	int type = 0;

	for (auto& node : nodes)
	{
		loader.EnterBlock("Node");

		loader.Read("type", type);

		if (type == ScnCallback)
		{
			node = new NodeSceneObject();
		}

		if (type == ScriptProperty)
		{
			node = new NodeScriptProperty();
		}

		if (type == ScriptMethod)
		{
			node = new NodeScriptMethod();
		}

		node->type = (NodeType)type;
		node->Load(loader);

		loader.LeaveBlock();
	}
}

void SceneScript::Save(JSONWriter& saver)
{
	GetMetaData()->Prepare(this);
	GetMetaData()->Save(saver);

	int count = (int)nodes.size();
	saver.Write("Count", count);

	saver.StartArray("Node");

	for (auto node : nodes)
	{
		saver.StartBlock(nullptr);

		node->Save(saver);

		saver.FinishBlock();
	}

	saver.FinishArray();
}

void SceneScript::SetName(const char* set_name)
{
	SceneObject::SetName(set_name);

	if (set_name[0])
	{
		string filename;
		GetScriptFileName(GetUID(), filename);
#ifdef EDITOR
		FILE* fl = fopen(filename.c_str(), "a");
		fclose(fl);
#endif
	}
}

bool SceneScript::Play()
{
	string filename;
	GetScriptFileName(GetUID(), filename);

	FileInMemory file;

	if (!file.Load(filename.c_str()))
	{
		return false;
	}

	mod = scripts.engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", (const char*)file.GetData(), file.GetSize());

	mod->Build();

	if (main_class.c_str()[0])
	{
		for (uint32_t i = 0; i < mod->GetObjectTypeCount(); i++)
		{
			asITypeInfo* tp = mod->GetObjectTypeByIndex(i);

			if (StringUtils::IsEqual(main_class.c_str(), tp->GetName()))
			{
				class_type = tp;
				break;
			}
		}

		if (!class_type)
		{
			core.Log("ScriptErr", "Class %s not found", main_class.c_str());
			return false;
		}
	}
	else
	{
		class_type = mod->GetObjectTypeByIndex(0);
	}

	class_inst = (asIScriptObject*)scripts.engine->CreateScriptObject(class_type);

	for (auto node : nodes)
	{
		if (node->type == ScriptProperty)
		{
			NodeScriptProperty* node_prop = (NodeScriptProperty*)node;
			
			for (int i = 0; i < (int)class_inst->GetPropertyCount(); i++)
			{
				if (StringUtils::IsEqual(class_inst->GetPropertyName(i), node_prop->name.c_str()))
				{
					if (!node_prop->object)
					{
						node_prop->object = owner->FindByUID(node_prop->object_uid, node_prop->object_child_uid, false);
					}
					
					if (node_prop->object)
					{
						auto type = scripts.engine->GetTypeInfoById(class_inst->GetPropertyTypeId(i));
						
						if (!node_prop->object->InjectIntoScript(type->GetName(), class_inst->GetAddressOfProperty(i)))
						{
							core.Log("ScriptErr", "Object %s of type %s can't be injected into %s of type %s", node_prop->object->GetName(), node_prop->object->script_class_name, class_inst->GetPropertyName(i), type->GetName());
							return false;
						}
					}
				}
			}
		}
		else
		if (node->type == ScriptMethod)
		{
			NodeScriptMethod* node_method = (NodeScriptMethod*)node;

			if (node_method->param_type == 3)
			{
				char prototype[256];
				StringUtils::Printf(prototype, 256, "void %s(float)", node_method->name.c_str());

				node_method->method = class_type->GetMethodByDecl(prototype);
			}
			else
			{
				for (auto link : node_method->links)
				{
					NodeSceneObject* scene_prop = (NodeSceneObject*)nodes[link->node];

					if (!scene_prop->object)
					{
						scene_prop->object = owner->FindByUID(scene_prop->object_uid, scene_prop->object_child_uid, false);
					}

					if (scene_prop->object && scene_prop->object->script_callbacks.size() > 0)
					{
						if (node_method->param_type == 1)
						{
							scene_prop->object->script_callbacks[0].SetIntParam(atoi(link->param.c_str()));
						}
						else
						if (node_method->param_type == 2)
						{
							scene_prop->object->script_callbacks[0].SetStringParam(link->param);
						}

						scene_prop->object->script_callbacks[0].Prepare(class_type, class_inst, node_method->name.c_str());
					}
				}
			}
		}
		else
		{
			NodeSceneObject* scene_node = (NodeSceneObject*)node;

			if (!scene_node->object)
			{
				scene_node->object = owner->FindByUID(scene_node->object_uid, scene_node->object_child_uid, false);
			}
		}
	}

	return true;
}

void SceneScript::Work(float dt)
{
	if (!Playing())
	{
		return;
	}

	for (auto& node : nodes)
	{
		if (node->type == ScriptMethod)
		{
			NodeScriptMethod* node_method = (NodeScriptMethod*)node;

			if (node_method->param_type == 3)
			{
				Script()->ctx->Prepare(node_method->method);
				Script()->ctx->SetArgFloat(0, core.GetDeltaTime());
				Script()->ctx->SetObject(class_inst);
				if (Script()->ctx->Execute() < 0)
				{
					core.Log("ScriptErr", "Error occured in call of '%s'", node_method->name.c_str());
				}
			}
		}
	}
}

void SceneScript::Stop()
{
	RELEASE(class_inst);
}

void SceneScript::Release()
{
	for (auto node : nodes)
	{
		if (node->type == ScriptMethod)
		{
			NodeScriptMethod* node_method = (NodeScriptMethod*)node;

			for (auto& link : node_method->links)
			{
				delete link;
			}
		}

		delete node;
	}
}

bool SceneScript::UsingCamera2DPos()
{
	return true;
}

Vector2& SceneScript::Camera2DPos()
{
	return camera_pos;
}

#ifdef EDITOR

void SceneScript::EditorWork(float dt)
{
	int index = 0;
	for (auto node : nodes)
	{
		Color color = COLOR_CYAN;
		if ((sel_node == index || target_link == index) && sel_link == -1)
		{
			color = COLOR_GREEN;
		}

		editor_drawer.DrawSprite(editor_drawer.node_tex, node->pos - Sprite::ed_cam_pos, nodeSize, color);
		editor_drawer.PrintText(node->pos + Vector2(5.0f + (node->type == ScriptMethod ? 15.0f : 0.0f), 30.0f) - Sprite::ed_cam_pos, COLOR_WHITE, node->name.c_str());

		if (node->type == ScnCallback)
		{
			Color color = (sel_link != -1 && ((NodeScriptMethod*)nodes[sel_node])->links[sel_link]->node == index) ? COLOR_GREEN : COLOR_WHITE;

			if (link_drag && sel_node == index)
			{
				color = COLOR_GREEN;
			}

			editor_drawer.DrawSprite(editor_drawer.arrow_tex, node->pos + Vector2(nodeSize.x - 15.0f, 30.0f) - Sprite::ed_cam_pos, linkSize, color);
		}
		else
		if (node->type == ScriptMethod)
		{
			Color color = (link_drag && target_link == index) ? COLOR_GREEN : COLOR_WHITE;
			editor_drawer.DrawSprite(editor_drawer.arrow_tex, node->pos + Vector2(5.0f, 30.0f) - Sprite::ed_cam_pos, linkSize, color);
		}

		const char* names[] = {"Callback", "Property", "Method"};
		editor_drawer.PrintText(node->pos + Vector2(5.0f, 3.0f) - Sprite::ed_cam_pos, COLOR_WHITE, names[node->type]);

		if (node->type == ScnCallback || node->type == ScriptProperty)
		{
			NodeScriptProperty* scene_node = (NodeScriptProperty*)node;

			if (!scene_node->object)
			{
				scene_node->object = owner->FindByUID(scene_node->object_uid, scene_node->object_child_uid, false);
			}

			editor_drawer.PrintText(node->pos + Vector2(5.0f, 50.0f) - Sprite::ed_cam_pos, COLOR_WHITE, scene_node->object ? scene_node->object->GetName() : "NULL");
		}

		index++;
	}

	index = 0;
	for (auto node : nodes)
	{
		if (node->type == ScriptMethod)
		{
			int link_index = 0;
			for (auto link : ((NodeScriptMethod*)node)->links)
			{
				Vector2 p1 = nodes[link->node]->pos + Vector2(nodeSize.x, 37.0f) - Sprite::ed_cam_pos;
				Vector2 p2 = node->pos + Vector2(0.0f, 37.0f) - Sprite::ed_cam_pos;

				editor_drawer.DrawCurve(p1, p2, (index == sel_node && link_index == sel_link) ? COLOR_GREEN : COLOR_WHITE);

				link->arrow_pos = nodes[link->node]->pos + Vector2(nodeSize.x - 15.0f, 30.0f) - Sprite::ed_cam_pos;
				link_index++;
			}
		}

		index++;
	}

	if (sel_node != -1 && link_drag)
	{
		editor_drawer.DrawCurve(nodes[sel_node]->pos + Vector2(nodeSize.x, 37.0f) - Sprite::ed_cam_pos, ms_pos, COLOR_WHITE);
	}
}

void SceneScript::OnMouseMove(Vector2 delta_ms)
{
	ms_pos += delta_ms;

	if (in_drag)
	{
		if (sel_node != -1)
		{
			nodes[sel_node]->pos += delta_ms;
		}
	}

	if (link_drag)
	{
		target_link = -1;
		int index = 0;
		for (auto& node : nodes)
		{
			if (node->type != ScriptMethod)
			{
				index++;
				continue;
			}

			if (node->pos.x - Sprite::ed_cam_pos.x < ms_pos.x && ms_pos.x < node->pos.x + 15.0f - Sprite::ed_cam_pos.x &&
				node->pos.y + 30.0f - Sprite::ed_cam_pos.y < ms_pos.y && ms_pos.y < node->pos.y + 45.0f - Sprite::ed_cam_pos.y)
			{
				target_link = index;
			}

			index++;
		}
	}
}

void SceneScript::OnLeftMouseDown(Vector2 ms)
{
	ShowProperties(false);

	sel_node = -1;
	sel_link = -1;
	link_drag = false;
	in_drag = false;

	int index = 0;
	for (auto& node : nodes)
	{
		if (node->pos.x - Sprite::ed_cam_pos.x < ms.x && ms.x < node->pos.x + nodeSize.x - Sprite::ed_cam_pos.x &&
			node->pos.y - Sprite::ed_cam_pos.y < ms.y && ms.y < node->pos.y + nodeSize.y - Sprite::ed_cam_pos.y)
		{
			sel_node = index;
			sel_link = -1;
		}

		index++;
	}

	index = 0;
	for (auto& node : nodes)
	{
		if (node->type == ScriptMethod)
		{
			int link_index = 0;

			for (auto& link : ((NodeScriptMethod*)node)->links)
			{
				if (link->arrow_pos.x < ms.x && ms.x < link->arrow_pos.x + linkSize.x &&
					link->arrow_pos.y < ms.y && ms.y < link->arrow_pos.y + linkSize.y)
				{
					sel_node = index;
					sel_link = link_index;
				}

				link_index++;
			}
		}

		index++;
	}

	if (sel_node != -1 && sel_link == -1)
	{
		in_drag = true;

		Node* node = nodes[sel_node];

		if (node->type == ScnCallback)
		{
			if (node->pos.x + nodeSize.x - 15.0f - Sprite::ed_cam_pos.x < ms.x && ms.x < node->pos.x + nodeSize.x - Sprite::ed_cam_pos.x &&
				node->pos.y + 30.0f - Sprite::ed_cam_pos.y < ms.y && ms.y < node->pos.y + 45.0f - Sprite::ed_cam_pos.y)
			{
				link_drag = true;
				in_drag = false;
			}
		}

		ms_pos = ms;
	}

	ShowProperties(true);
}

void SceneScript::OnLeftMouseUp()
{
	if (link_drag && target_link != -1)
	{
		if ((nodes[sel_node]->type == ScnCallback && nodes[target_link]->type == ScriptMethod))
		{
			int tmp = sel_node;
			sel_node = target_link;
			target_link = tmp;
		}

		Node* node = nodes[sel_node];
		Node* traget = nodes[target_link];

		if (node->type == ScriptMethod && traget->type == ScnCallback)
		{
			NodeScriptMethod* method_node = (NodeScriptMethod*)node;

			bool allow_add = true;

			for (auto& link : method_node->links)
			{
				if (link->node == target_link)
				{
					allow_add = false;
					break;
				}
			}

			if (allow_add)
			{
				LinkToMethod* link = new LinkToMethod();

				link->node = target_link;

				method_node->links.push_back(link);
			}
		}
	}

	in_drag = false;
	link_drag = false;
	target_link = -1;
}

void SceneScript::OnRightMouseDown(Vector2 ms)
{
	ms_pos = ms;

	ed_popup_menu->StartMenu(true);

	ed_popup_menu->StartSubMenu("Create Link to");
	ed_popup_menu->AddItem(5001, "Callback");
	ed_popup_menu->AddItem(5002, "Property");
	ed_popup_menu->AddItem(5003, "Method");
	ed_popup_menu->EndSubMenu();

	ed_popup_menu->AddItem(5004, "Duplicate", (sel_node != -1 && sel_link == -1));
	ed_popup_menu->AddItem(5005, "Delete", (sel_node != -1));

	ed_popup_menu->ShowAsPopup(ed_vieport, (int)ms.x, (int)ms.y);
}

void SceneScript::OnPopupMenuItem(int id)
{
	Node* node = nullptr;

	if (id == 5001)
	{
		NodeSceneObject* scene_node = new NodeSceneObject();
		scene_node->type = ScnCallback;
		scene_node->name = "Object";
		node = scene_node;
	}

	if (id == 5002)
	{
		NodeScriptProperty* prop_node = new NodeScriptProperty();
		prop_node->type = ScriptProperty;
		prop_node->name = "property";
		node = prop_node;
	}

	if (id == 5003)
	{
		NodeScriptMethod* method_node = new NodeScriptMethod();
		method_node->type = ScriptMethod;
		method_node->name = "method";
		node = method_node;
	}

	if (node)
	{
		node->pos = ms_pos + Sprite::ed_cam_pos;
		nodes.push_back(node);
	}

	if (id == 5004)
	{
	}

	if (id == 5005)
	{
		if (sel_link == -1)
		{
			Node* node = nodes[sel_node];

			if (node->type == ScriptMethod)
			{
				NodeScriptMethod* node_method = (NodeScriptMethod*)nodes[sel_node];

				for (auto& link : node_method->links)
				{
					delete link;
				}
			}
			else
			{
				for (auto& nd : nodes)
				{
					if (nd->type == ScriptMethod)
					{
						NodeScriptMethod* node_method = (NodeScriptMethod*)nd;

						int index = 0;
						for (auto& link : node_method->links)
						{
							if (link->node == sel_node)
							{
								delete link;
								node_method->links.erase(node_method->links.begin() + index);
							}
							else
							if (link->node > sel_node)
							{
								link->node--;
							}

							index++;
						}
					}
				}
			}

			int sel_nd = sel_node;
			ShowProperties(false);

			delete node;
			nodes.erase(nodes.begin() + sel_nd);
		}
		else
		{
			NodeScriptMethod* node_method = (NodeScriptMethod*)nodes[sel_node];

			int link = sel_link;
			ShowProperties(false);

			delete node_method->links[link];
			node_method->links.erase(node_method->links.begin() + link);
		}
	}
}

void SceneScript::OnDragObjectFromTreeView(bool is_scene_tree, SceneObject* object, Vector2 ms)
{
	if (!is_scene_tree)
	{
		return;
	}

	for (auto& node : nodes)
	{
		if (node->pos.x - Sprite::ed_cam_pos.x < ms.x && ms.x < node->pos.x + nodeSize.x - Sprite::ed_cam_pos.x &&
			node->pos.y - Sprite::ed_cam_pos.y < ms.y && ms.y < node->pos.y + nodeSize.y - Sprite::ed_cam_pos.y)
		{
			if (node->type == ScnCallback || node->type == ScriptProperty)
			{
				NodeSceneObject* scene_node = (NodeSceneObject*)node;

				scene_node->object_uid = object->GetParentUID();
				scene_node->object_child_uid = object->GetUID();

				if (scene_node->object_uid == 0)
				{
					scene_node->object_uid = scene_node->object_child_uid;
					scene_node->object_child_uid = 0;
				}
				if (scene_node->object_uid == scene_node->object_child_uid)
				{
					scene_node->object_child_uid = 0;
				}

				scene_node->object = object;
			}

			break;
		}
	}
}

void SceneScript::ShowProperties(bool show)
{
	if (show)
	{
		if (sel_node != -1 && sel_link != -1)
		{
			LinkToMethod* link = ((NodeScriptMethod*)nodes[sel_node])->links[sel_link];
			if (link->GetMetaData())
			{
				link->GetMetaData()->Prepare(link);
				link->GetMetaData()->PrepareWidgets(ed_obj_cat);
			}
		}
		else
		if (sel_node != -1)
		{
			Node* node = nodes[sel_node];
			node->GetMetaData()->Prepare(node);
			node->GetMetaData()->PrepareWidgets(ed_obj_cat);
		}
	}
	else
	{
		if (sel_node != -1 && sel_link != -1)
		{
			LinkToMethod* link = ((NodeScriptMethod*)nodes[sel_node])->links[sel_link];
			if (link->GetMetaData())
			{
				link->GetMetaData()->HideWidgets();
			}
		}
		else
		if (sel_node != -1)
		{
			nodes[sel_node]->GetMetaData()->HideWidgets();
		}

		sel_node = -1;
		sel_link = -1;
	}
}
#endif