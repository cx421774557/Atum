
#include <jni.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Services/Core/Core.h"
#include "Services/Scene/Scene.h"
#include "Services/Scene/ExecuteLevels.h"
#include "SceneObjects/2D/Sprite.h"

TaskExecutor::SingleTaskPool* renderTaskPool;

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

JNIEnv* cur_env = nullptr;
jobject cur_obj;
jclass  cur_cls;
std::vector<jvalue> cur_args;

extern "C"
{
	JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_Init(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_Resize(JNIEnv* env, jobject obj, jint width, jint height);
	JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_Update(JNIEnv* env, jobject obj);
	JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_SetAssetManager(JNIEnv* env, jobject obj, jobject assetManager);
	JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_TouchStart(JNIEnv* env, jobject obj, int index, int x, int y);
	JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_TouchUpdate(JNIEnv* env, jobject obj, int index, int x, int y);
	JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_TouchEnd(JNIEnv* env, jobject obj, int index);
};

void UpdateJavaEnv(JNIEnv* env, jobject obj)
{
	cur_env = env;
	cur_obj = obj;
	cur_cls = cur_env->GetObjectClass(cur_obj);
}

void CallJavaMethod(const char* function, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	std::string prototype = "(";
	int index = 0;

	bool fail = false;
	int len = strlen(format);

	cur_args.resize(0);

	while (format[index] != 0)
	{
		if (format[index] != '%' || index + 1 >= len)
		{
			fail = true;
			break;
		}
		
		jvalue cur_value;

		switch (format[index + 1])
		{
			case 'i':
			{
				prototype += "Ljava/lang/Int;";
				cur_value.i = va_arg(args, int);
				break;
			}
			case 'f':
			{
				prototype += "Ljava/lang/Float;";
				cur_value.f = (float)va_arg(args, double);
				break;
			}
			case 'd':
			{
				prototype += "Ljava/lang/Double;";
				cur_value.d = va_arg(args, double);
				break;
			}
			case 's':
			{
				prototype += "Ljava/lang/String;";
				cur_value.l = cur_env->NewStringUTF(va_arg(args, const char*));
				break;
			}
			default:
			fail = true;
		}

		cur_args.push_back(cur_value);

		if (fail)
		{
			break;
		}

		index += 2;
	}

	va_end(args);

	if (fail)
	{
		core.Log("app", "CallJavaMethod: %s - invalid format at %i", function, index);
	}
	else
	{
		prototype += ")V";

		jmethodID mid = cur_env->GetMethodID(cur_cls, function, prototype.c_str());

		if (mid == nullptr)
		{
			core.Log("app", "CallJavaMethod: Method %s not found", function);
		}
		else
		{
			cur_env->CallVoidMethodA(cur_obj, mid, &cur_args[0]);
		}
	}
}

extern Scene* hack_scene;

class Renderer : public Object
{
public:

	Scene scene;
	bool inited = false;

	void Init()
	{
		if (inited)
		{
			return;
		}

		hack_scene = &scene;

		scene.Init();
		scene.Load("Projects/SunnyLand/SunnyLand.scn");
		scene.Play();

		inited = true;
	}

	void Draw(float dt)
	{
		render.DebugPrintText(10.0f, COLOR_GREEN, "%i", core.GetFPS());

		render.GetDevice()->Clear(true, COLOR_GRAY, true, 1.0f);

		render.ExecutePool(ExecuteLevels::Camera, dt);
		render.ExecutePool(ExecuteLevels::Prepare, dt);
		render.ExecutePool(ExecuteLevels::Geometry, dt);
		render.ExecutePool(ExecuteLevels::DebugGeometry, dt);
		render.ExecutePool(ExecuteLevels::Sprites, dt);
		render.ExecutePool(ExecuteLevels::PostProcess, dt);
		render.ExecutePool(ExecuteLevels::GUI, dt);
		render.ExecutePool(ExecuteLevels::Debug, dt);
	}
};

Renderer renderer;

JNIEXPORT void JNICALL
Java_com_atum_engine_AtumLib_Init(JNIEnv* env, jobject obj)
{
	UpdateJavaEnv(env, obj);

	core.Init(nullptr);

	Sprite::Init();

	scripts.Start();

	render.AddExecutedLevelPool(1);

	renderTaskPool = render.AddTaskPool();
	renderTaskPool->AddTask(1, &renderer, (Object::Delegate)&Renderer::Draw);
}

JNIEXPORT void JNICALL
Java_com_atum_engine_AtumLib_Resize(JNIEnv* env, jobject obj, jint width, jint height)
{
	UpdateJavaEnv(env, obj);

	render.GetDevice()->SetVideoMode(width, height, nullptr);

	renderer.Init();
}

JNIEXPORT void JNICALL
Java_com_atum_engine_AtumLib_Update(JNIEnv* env, jobject obj)
{
	UpdateJavaEnv(env, obj);

	core.CountDeltaTime();

	renderer.scene.Execute(core.GetDeltaTime());

	core.Update();
}

JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_SetAssetManager(JNIEnv* env, jobject obj, jobject assetManager)
{
	AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
	files.android_asset_manager = mgr;
}

JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_TouchStart(JNIEnv* env, jobject obj, int index, int x, int y)
{
	core.Log("Touch", "Touch %i at (%i, %i) started", index, x, y);
	controls.TouchStart(index, x, y);
}

JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_TouchUpdate(JNIEnv* env, jobject obj, int index, int x, int y)
{
	core.Log("Touch", "Touch %i updated to (%i, %i)", index, x, y);
	controls.TouchUpdate(index, x, y);
}

JNIEXPORT void JNICALL Java_com_atum_engine_AtumLib_TouchEnd(JNIEnv* env, jobject obj, int index)
{
	core.Log("Touch", "Touch %i ended", index);
	controls.TouchEnd(index);
}