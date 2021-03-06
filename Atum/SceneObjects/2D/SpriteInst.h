
#pragma once

#include "Services/Scene/SceneObject.h"
#include "SpriteAsset.h"
#include "Services/Script/Libs/scriptarray.h"
#include "SpriteGraphAsset.h"

class SpriteInst : public SceneObjectInst
{
public:
	META_DATA_DECL(SpriteInst)

	Transform2D trans;
	Transform2D multi_trans;
	float axis_scale = 1.0f;
	int draw_level = 0;

	struct Instance
	{
		META_DATA_DECL(Instance)

		int index = 0;
		Sprite::FrameState frame_state;

		void SetObject(asIScriptObject* object, vector<int>* mapping);
		void SetPos(Vector2 pos);
		Vector2 GetPos();
		void SetFlipped(int horz_flipped);
		int  GetFlipped();
		void SetVisible(int visible);
		bool IsVisible();

		float GetAlpha();
		void  SetAlpha(float set_alpha);

		float GetSizeX();
		void  SetSizeX(float set_size);

		float GetAngle();
		void  SetAngle(float set_angle);

		Color color;

		bool auto_delete = false;
		bool hack_marker = false;

		SpriteGraphAsset::Instance graph_instance;
		Vector2 dir = 0.0f;

	private:
		vector<int>* mapping = nullptr;
		asIScriptObject* object = nullptr;
		Vector2 pos;
		float   size_x = -1.0f;
		int     visible = 1;
		float   alpha = 1.0f;
		float   angle = 0.0f;
	};

	struct InstanceHolder
	{
		int index = -1;
		Instance* inst = nullptr;
	};

	vector<int> mapping;
	CScriptArray* array = nullptr;

	int sel_inst = -1;
	vector<Instance> instances;

	bool rect_select = false;
	Vector2 rect_p1;
	Vector2 rect_p2;

	vector<int> sel_instances;

	virtual ~SpriteInst() = default;

	void BindClassToScript() override;
	bool InjectIntoScript(const char* type, void* property) override;
	void MakeMapping(asIScriptObject* object);

	void Init() override;
	void ApplyProperties() override;
	virtual void Draw(float dt);

	bool Play() override;

	PhysObject* HackGetBody(int index);
	int AddInstance(float x, float y, bool auto_delete);
	void RemoveInstance(int index);
	void ClearInstances();
	void ApplyLinearImpulse(int index, float x, float y);
	void MoveTo(int index, float x, float y);
	void SetActiveTrack(int index, bool set);

#ifdef EDITOR
	void ClearRect();
	void FillRect();
	void OnRectSelect(Vector2 p1, Vector2 p2) override;
	bool CheckSelection(Vector2 ms) override;
	void SetEditMode(bool ed) override;
	void SetGizmo();
	int  GetInstCount();
#endif
};
