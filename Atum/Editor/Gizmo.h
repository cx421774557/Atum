
#pragma once

#include "Support/Support.h"
#include "EUI.h"

class Gizmo
{
public:

	enum Trans2D
	{
		trans_2d_move    = 1,
		trans_2d_scale   = 2,
		trans_2d_rotate  = 4,
		trans_2d_anchorn = 8
	};

	static Gizmo* inst;

	int     mode = 1;
	bool    useLocalSpace = false;
	float   scale = 1.0f;
	Vector2 align2d = 0.0f;

	EUIEditBox* ebox_x = nullptr;
	EUIEditBox* ebox_y = nullptr;
	EUIEditBox* ebox_width = nullptr;
	EUIEditBox* ebox_height = nullptr;
	EUIEditBox* ebox_offset_x = nullptr;
	EUIEditBox* ebox_offset_y = nullptr;

private:

	Matrix  transform;
	bool    mousedPressed = false;

	Transform2D* trans2D = nullptr;
	int     trans2D_actions = 0;
	bool    ignore_2d_camera = true;
	Vector2 pos2d = 0.0f;
	Vector2 origin;
	Vector2 moved_origin;
	Vector2 ancorns[8];

	bool    enabled = false;

	int     selAxis = -1;
	Vector2 ms_dir = 0.0f;
	Vector2 prev_ms = 0.0f;

	Color CheckColor(int axis);
	void DrawAxis(int axis);
	void DrawCircle(int axis);
	bool CheckInersection(Vector pos, Vector pos2, Vector2 ms,
	                      Vector trans, bool check_trans,
	                      Matrix view, Matrix view_proj);

	bool IsInsideTriangle(Vector2 s, Vector2 a, Vector2 b, Vector2 c);

	void CheckSelectionTrans2D(Vector2 ms);
	bool CheckSelectionTrans3D(int axis, Vector2 ms);
	void CheckSelectionTrans3D(Vector2 ms);

	void MoveTrans2D(Vector2 ms);
	void MoveTrans3D(Vector2 ms);

	void RenderTrans2D();
	void RenderTrans3D();

public:

	Vector2 delta_move = 0.0;

	void Init();
	void SetTrans2D(Transform2D* trans2D, int actions = 0xffff, bool ignore_2d_camera = false);
	void SetTrans2DWidgets(EUIEditBox* ebox_x = nullptr, EUIEditBox* ebox_y = nullptr, EUIEditBox* ebox_width = nullptr, EUIEditBox* ebox_height = nullptr, EUIEditBox* ebox_offset_x = nullptr, EUIEditBox* ebox_offset_y = nullptr);
	void SetTrans3D(Matrix transform);
	Matrix& GetTrans3D();
	bool IsTrans2D();
	bool IsEnabled();
	void Disable();

	void OnMouseMove(Vector2 ms);
	void OnLeftMouseDown();
	void OnLeftMouseUp();

	Vector2 MakeAligned(Vector2 pos);
	void Render();
};