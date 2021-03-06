
#pragma once

#include "Services/Scene/SceneObject.h"
#include "SpriteInst.h"

class SpriteGraphInst : public SpriteInst
{
public:
	META_DATA_DECL(SpriteGraphInst)

	float hack_height = 0.0f;

	virtual ~SpriteGraphInst() = default;

	void BindClassToScript() override;

	void Init() override;
	void ApplyProperties() override;
	bool Play() override;
	void Draw(float dt);
	bool ActivateLink(int index, string& link);
	void GotoNode(int index, string& node);
	PhysController* HackGetController(int index);
	bool CheckColission(int index, bool under);
	void MoveController(int index, float dx, float dy);
	void MoveControllerTo(int index, float x, float y);
};
