
#include "terrain.h"
#include "programs.h"
#include "Services/Render/Render.h"
#include "SceneObjects/RenderLevels.h"

CLASSREG(SceneObject, Terrain, "Terrain")

META_DATA_DESC(Terrain)
FLOAT_PROP(Terrain, scaleh, 0.5f, "Geometry", "ScaleH")
FLOAT_PROP(Terrain, scalev, 0.1f, "Geometry", "ScaleV")
FILENAME_PROP(Terrain, tex_name, "", "Geometry", "Texture")
FILENAME_PROP(Terrain, hgt_name, "", "Geometry", "Heightmap")
COLOR_PROP(Terrain, color, COLOR_WHITE, "Geometry", "COLOR")
META_DATA_DESC_END()

Terrain::Terrain() : SceneObject()
{

}

Terrain::~Terrain()
{

}

void Terrain::Init()
{
	VertexDecl::ElemDesc desc[] = { { VertexDecl::Float3, VertexDecl::Position, 0 },{ VertexDecl::Float2, VertexDecl::Texcoord, 0 },{ VertexDecl::Float3, VertexDecl::Texcoord, 1 } };
	vdecl = render.GetDevice()->CreateVertexDecl(3, desc);

	RenderTasks(false)->AddTask(RenderLevels::Geometry, this, (Object::Delegate)&Terrain::Render);

	owner->AddToGroup(this, "Terrain");
}

void Terrain::ApplyProperties()
{
	int stride = sizeof(VertexTri);

	LoadHMap(hgt_name.c_str());

	sz = (hwidth) * (hheight) * 2;

	RELEASE(buffer)
	buffer = render.GetDevice()->CreateBuffer(sz * 3, stride);

	VertexTri* v_tri = (VertexTri*)buffer->Lock();

	float start_x = -hwidth * 0.5f * scaleh;
	float start_z = -hheight * 0.5f * scaleh;

	float du = 1.0f / (hwidth - 1.0f);
	float dv = 1.0f / (hheight - 1.0f);

	bool shift = false;

	for (int j = 0; j < hheight - 1; j++)
	{
		for (int i = 0; i < hwidth - 1; i++)
		{
			if (shift)
			{
				if (i % 2 == 0)
				{
					v_tri[1].position = GetVecHeight(i,j);
					v_tri[1].texCoord = Vector2(0.0f, 0.0f);
					v_tri[0].position = GetVecHeight(i+1, j);
					v_tri[0].texCoord = Vector2(1.0f, 0.0f);
					v_tri[2].position = GetVecHeight(i+1, j+1);
					v_tri[2].texCoord = Vector2(1.0f, 1.0f);

					v_tri[4].position = GetVecHeight(i, j);
					v_tri[4].texCoord = Vector2(0.0f, 0.0f);
					v_tri[3].position = GetVecHeight(i + 1, j + 1);
					v_tri[3].texCoord = Vector2(1.0f, 1.0f);
					v_tri[5].position = GetVecHeight(i, j + 1);
					v_tri[5].texCoord = Vector2(0.0f, 1.0f);
				}
				else
				{
					v_tri[1].position = GetVecHeight(i+1,j);
					v_tri[1].texCoord = Vector2(1.0f, 0.0f);
					v_tri[0].position = GetVecHeight(i+1,j+1);
					v_tri[0].texCoord = Vector2(1.0f, 1.0f);
					v_tri[2].position = GetVecHeight(i, j+1);
					v_tri[2].texCoord = Vector2(0.0f, 1.0f);

					v_tri[4].position = GetVecHeight(i, j);
					v_tri[4].texCoord = Vector2(0.0f, 0.0f);
					v_tri[3].position = GetVecHeight(i+1, j);
					v_tri[3].texCoord = Vector2(1.0f, 0.0f);
					v_tri[5].position = GetVecHeight(i, j + 1);
					v_tri[5].texCoord = Vector2(0.0f, 1.0f);
				}
			}
			else
			{
				if (i % 2 == 0)
				{
					v_tri[1].position = GetVecHeight(i, j);
					v_tri[1].texCoord = Vector2(0.0f, 0.0f);
					v_tri[0].position = GetVecHeight(i + 1, j);
					v_tri[0].texCoord = Vector2(1.0f, 0.0f);
					v_tri[2].position = GetVecHeight(i, j + 1);
					v_tri[2].texCoord = Vector2(0.0f, 1.0f);

					v_tri[4].position = GetVecHeight(i + 1, j);
					v_tri[4].texCoord = Vector2(1.0f, 0.0f);
					v_tri[3].position = GetVecHeight(i + 1, j + 1);
					v_tri[3].texCoord = Vector2(1.0f, 1.0f);
					v_tri[5].position = GetVecHeight(i, j + 1);
					v_tri[5].texCoord = Vector2(0.0f, 1.0f);
				}
				else
				{
					v_tri[1].position = GetVecHeight(i, j);
					v_tri[1].texCoord = Vector2(0.0f, 0.0f);
					v_tri[0].position = GetVecHeight(i + 1, j + 1);
					v_tri[0].texCoord = Vector2(1.0f, 1.0f);
					v_tri[2].position = GetVecHeight(i, j + 1);
					v_tri[2].texCoord = Vector2(0.0f, 1.0f);

					v_tri[4].position = GetVecHeight(i, j);
					v_tri[4].texCoord = Vector2(0.0f, 0.0f);
					v_tri[3].position = GetVecHeight(i + 1, j);
					v_tri[3].texCoord = Vector2(1.0f, 0.0f);
					v_tri[5].position = GetVecHeight(i + 1, j + 1);
					v_tri[5].texCoord = Vector2(1.0f, 1.0f);
				}
			}

			float cur_du = du * i;
			float cur_dv = dv * j;

			for (int k = 0; k < 6; k++)
			{
				v_tri[k].position.x = start_x + v_tri[k].position.x * scaleh;
				v_tri[k].position.z = start_z - v_tri[k].position.z * scaleh;
				v_tri[k].texCoord = Vector2(cur_du + v_tri[k].texCoord.x * du, cur_dv + v_tri[k].texCoord.y * dv);
			}

			for (int k = 0; k < 2; k++)
			{
				Vector v1 = v_tri[k * 3 + 1].position - v_tri[k * 3 + 0].position;
				Vector v2 = v_tri[k * 3 + 2].position - v_tri[k * 3 + 0].position;
				
				v1.Normalize();
				v2.Normalize();

				Vector normal;
				
				normal = v1.Cross(v2);
				normal.Normalize();

				for (int l = 0; l < 3; l++)
				{
					v_tri[k * 3 + l].normal = normal;
				}
			}

			v_tri += 6;
		}

		shift = !shift;
	}

	buffer->Unlock();

	RELEASE(texture)
	texture = render.LoadTexture(tex_name.c_str());
}

float Terrain::GetHeight(int i, int j)
{
	return hmap ? hmap[((j)* hwidth + i)] * scalev : 1.0f;
}

Vector Terrain::GetVecHeight(int i, int j)
{
	return Vector((float)i, GetHeight(i, j), -(float)j);
}

void Terrain::LoadHMap(const char* hgt_name)
{
	Buffer hbuffer;

	FREE_PTR(hmap)

	if (!hbuffer.Load(hgt_name))
	{
		hwidth = 512;
		hheight = 512;

		return;
	}
	uint8_t* ptr = hbuffer.GetData();

	ptr += 2;

	uint8_t imageTypeCode = *((uint8_t*)ptr);

	if (imageTypeCode != 2 && imageTypeCode != 3)
	{
		return;
	}

	ptr += 10;

	short int imageWidth = *((short int*)ptr);
	ptr += 2;

	short int imageHeight = *((short int*)ptr);
	ptr += 2;

	uint8_t bitCount = *((uint8_t*)ptr);
	ptr += 2;

	int colorMode = bitCount / 8;

	hwidth = imageWidth;
	hheight = imageHeight;

	hmap = (uint8_t*)malloc(hwidth * hheight);

	for (int i = 0; i < hheight; i++)
	{
		for (int j = 0; j < hwidth; j++)
		{
			hmap[i * hheight + j] = ptr[((j)* imageWidth + i) * colorMode];
		}
	}

#ifdef PLATFORM_PC
	string cooked_name = hgt_name + string("hm");

	if (!Buffer::IsFileExist(cooked_name.c_str()))
	{
		PhysHeightmap::Desc hdesc;
		hdesc.width = hwidth;
		hdesc.height = hheight;
		hdesc.scale = Vector2(scaleh, scalev);
		hdesc.hmap = hmap;

		physics.CookHeightmap(hdesc, cooked_name.c_str());
	}
#endif
}

void Terrain::Render(float dt)
{
	Render(Programs::GetTranglPrg());
}

void Terrain::ShRender(float dt)
{
	Render(Programs::GetShdTranglPrg());
}

void Terrain::Render(Program* prg)
{
	render.GetDevice()->SetVertexDecl(vdecl);
	render.GetDevice()->SetVertexBuffer(0, buffer);

	render.GetDevice()->SetProgram(prg);

	render.SetTransform(Render::World, Matrix());

	Matrix view_proj;
	render.GetTransform(Render::WrldViewProj, view_proj);

	Matrix mat;
	Matrix world;

	prg->SetMatrix(Program::Vertex, "trans", &world, 1);
	prg->SetMatrix(Program::Vertex, "view_proj", &view_proj, 1);
	prg->SetVector(Program::Pixel, "color", (Vector4*)&color, 1);
	prg->SetTexture(Program::Pixel, "diffuseMap", texture);

	render.GetDevice()->Draw(Device::TrianglesList, 0, sz);
}

void Terrain::Play()
{
	PhysHeightmap::Desc hdesc;
	hdesc.width = hwidth;
	hdesc.height = hheight;
	hdesc.scale = Vector2(scaleh, scalev);

	hm = PScene()->CreateHeightmap(hdesc, (hgt_name + string("hm")).c_str());
}

void Terrain::Stop()
{
	RELEASE(hm);
}