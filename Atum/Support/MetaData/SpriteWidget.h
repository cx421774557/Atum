
#pragma once

#include "ProperyWidget.h"
#include "SceneAssets/Sprite.h"

class SpriteWidget : public ProperyWidget, public EUIWidget::Listener
{
public:

	typedef void(*Callback)(void* owner);

	EUIButton* openBtn;

	Sprite::Data* sprite;
	
	void Init(EUICategories* parent, const char* catName, const char* labelName) override;
	void SetData(void* set_data) override;
	void OnLeftMouseUp(EUIWidget* sender, int mx, int my) override;
};