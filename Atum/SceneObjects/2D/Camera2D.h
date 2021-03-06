
#pragma once

#include "Services/Scene/SceneObject.h"

class Camera2D : public SceneObject
{
public:
	META_DATA_DECL(Camera2D)

	Vector2 target_pos = 0.0f;
	Transform2D trans;
	float screen_border = 100.0f;
	float screen_vert_border = 100.0f;
	float left_border = -300.0f;
	float right_border =300.0f;
	
	bool use_lr_borders = false;
	Vector2 init_pos;

	Camera2D();
	virtual ~Camera2D();

	void BindClassToScript() override;

	void Init() override;
	void Update(float dt);

	bool Play() override;
	void Stop() override;

	void Reset();

#ifdef EDITOR
	void SetEditMode(bool ed);
#endif
};
