#include "Physics.h"
#include "Services/Render/Render.h"
#include "SceneAssets/Sprite.h"

Physics physics;

#ifdef PLATFORM_ANDROID
extern "C"
{
	int android_getCpuCount()
	{
		return 1;
	}
}
#endif

class FooDraw : public b2Draw
{
public:
	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		for (int i = 0; i < vertexCount - 1; i++)
		{
			render.DebugLine2D({ vertices[i].x * 50.0f, vertices[i].y  * 50.0f }, COLOR_BLUE, { vertices[i + 1].x * 50.0f, vertices[i + 1].y  * 50.0f }, COLOR_BLUE);
		}
	}

	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		float scale = render.GetDevice()->GetHeight() / 1024.0f;

		Vector2 cam_pos;
		cam_pos.x = Sprite::cam_pos.x * scale - render.GetDevice()->GetWidth() * 0.5f;
		cam_pos.y = Sprite::cam_pos.y * scale - 512 * scale;

		scale *= 50.0f;

		for (int i = 0; i < vertexCount; i++)
		{
			int index = (i == vertexCount - 1) ? 0 : i + 1;
			render.DebugLine2D({ vertices[i].x * scale - cam_pos.x, vertices[i].y * scale - cam_pos.y }, COLOR_BLUE, { vertices[index].x * scale - cam_pos.x, vertices[index].y * scale - cam_pos.y }, COLOR_BLUE);
		}
	}

	void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {}
	void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {}
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {}
	void DrawTransform(const b2Transform& xf) {}
	void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) {};
};

FooDraw foo_draw;

void Physics::Init()
{
	PxTolerancesScale tolerancesScale;

	foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, defaultAllocatorCallback, defaultErrorCallback);
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, tolerancesScale, true);

#ifdef PLATFORM_PC
	cooking = PxCreateCooking(PX_PHYSICS_VERSION, *foundation, PxCookingParams(tolerancesScale));
#endif

	defMaterial = physics->createMaterial(0.5f, 0.5f, 0.95f);
}

#ifdef PLATFORM_PC
void Physics::CookHeightmap(PhysHeightmap::Desc& desc, const char* name)
{
	PxHeightFieldSample* samples = new PxHeightFieldSample[desc.width * desc.height];

	for (int x = 0; x < desc.width; x++)
		for (int y = 0; y < desc.height; y++)
		{
			samples[x + y * desc.width].height = PxI16(desc.hmap[((x)* desc.width + y)]);
			samples[x + y * desc.width].setTessFlag();
			samples[x + y * desc.width].materialIndex0 = 1;
			samples[x + y * desc.width].materialIndex1 = 1;
		}

	PxHeightFieldDesc heightFieldDesc;

	heightFieldDesc.format = PxHeightFieldFormat::eS16_TM;
	heightFieldDesc.nbColumns = desc.width;
	heightFieldDesc.nbRows = desc.height;
	heightFieldDesc.samples.data = samples;
	heightFieldDesc.samples.stride = sizeof(PxHeightFieldSample);

	StreamWriter writer;
	if (writer.Prepere(name))
	{
		cooking->cookHeightField(heightFieldDesc, writer);
	}
}
#endif

PhysScene* Physics::CreateScene()
{
	PhysScene* scene = new PhysScene();

	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.8f, 0.0f);

	if (!sceneDesc.cpuDispatcher)
	{
		sceneDesc.cpuDispatcher = PxDefaultCpuDispatcherCreate(1);
	}

	if (!sceneDesc.filterShader)
	{
		sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	}

	sceneDesc.isValid();

	scene->scene = physics->createScene(sceneDesc);
	scene->manager = PxCreateControllerManager(*scene->scene);

	scenes.push_back(scene);

	return scene;
}

b2World* Physics::CreateScene2D()
{
	b2Vec2 gravity(0.0f, 20.0f);
	b2World* world2D = new b2World(gravity);
	world2D->SetDebugDraw(&foo_draw);

	foo_draw.SetFlags(b2Draw::e_shapeBit);

	scenes2D.push_back(world2D);

	return world2D;
}

void Physics::DestroyScene(PhysScene* scene)
{
	for (int i=0; i<scenes.size(); i++)
	{
		if (scenes[i] == scene)
		{
			scenes.erase(scenes.begin() + i);
			scene->Release();
		}
	}
}

void Physics::DestroyScene2D(b2World* scene)
{
	for (int i = 0; i<scenes2D.size(); i++)
	{
		if (scenes2D[i] == scene)
		{
			scenes2D.erase(scenes2D.begin() + i);
			delete scene;
		}
	}
}


void Physics::Update(float dt)
{
	accum_dt += dt;

	if (accum_dt > 0.5f)
	{
		accum_dt = 0.5f;
	}

	if (accum_dt > physStep)
	{
		for (auto scene : scenes)
		{
			if (!scene->needFetch)
			{
			}

			scene->Simulate(physStep);
			scene->needFetch = true;
		}

		for (auto scene : scenes2D)
		{
			int32 velocityIterations = 8;
			int32 positionIterations = 3;

			scene->Step(physStep, velocityIterations, positionIterations);
		}

		accum_dt -= physStep;
	}

	for (auto scene : scenes2D)
	{
		//scene->DrawDebugData();
	}
}

void Physics::Fetch()
{
	for (int i = 0; i < scenes.size(); i++)
	{
		PhysScene* scene = scenes[i];

		if (!scene->needFetch)
		{
			continue;
		}

		scene->FetchResults();
		scene->needFetch = false;
	}
}