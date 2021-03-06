
#pragma once

#include "SceneObject.h"

#ifdef EDITOR
#include "EUI.h"
#endif

class SceneAsset : public SceneObject
{
	friend class Scene;

public:


	virtual ~SceneAsset() = default;

	bool UsingCamera2DPos() override;

	const char* inst_class_name = nullptr;

#ifdef EDITOR
	vector<SceneObject*> instances;

	bool IsAsset() override;
	virtual SceneObject* CreateInstance();
	virtual void DeleteAsset(SceneObject* obj);

	virtual void PreapreAssetTree();
	virtual bool OnAssetTreeViewItemDragged(bool item_from_assets, SceneAsset* item, int prev_child_index, SceneObject* target, int child_index);
	virtual void OnAssetTreeSelChange(SceneAsset* item);
	virtual void OnAssetTreePopupItem(int id);
	virtual void OnAssetTreeRightClick(int x, int y, SceneAsset* item, int child_index);
#endif
};

CLASSFACTORYDEF(SceneAsset)
CLASSFACTORYDEF_END()
