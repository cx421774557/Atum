
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

	virtual SceneObject* CreateInstance();

#ifdef EDITOR
	virtual bool OnAssetTreeViewItemDragged(bool item_from_assets, SceneAsset* item, int prev_child_index, SceneObject* target, int child_index);
	virtual void OnAssetTreeSelChange(SceneAsset* item);
	virtual void OnAssetTreePopupItem(int id);
	virtual void OnAssetTreeRightClick(int x, int y, SceneAsset* item, int child_index);
#endif
};

CLASSFACTORYDEF(SceneAsset)
