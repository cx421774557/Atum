
#pragma once

#include "Support/Support.h"
#include "Shader.h"
#include "GeometryBuffer.h"
#include "Program.h"
#include "VertexDecl.h"
#include "Texture.h"

class Device
{
	friend class Render;
	friend class Program;
	friend class DeviceDX11;
	friend class DeviceGLES;

	int  scr_w = 0;
	int  scr_h = 0;
	int  cur_rt_w = 0;
	int  cur_rt_h = 0;
	float cur_aspect = 0.0f;

	virtual bool Init(void* external_device) = 0;
	virtual void PrepareProgram(Program* program) = 0;
	virtual void Release() = 0;

	Program* cur_program = nullptr;

public:

	struct Viewport
	{
		int x, y;
		int width, height;
		float minZ, maxZ;
	};

	struct Rect
	{
		int left, right;
		int top, bottom;
	};

	enum Primitive
	{
		LineStrip = 0,
		LinesList,
		TriangleStrip,
		TrianglesList
	};

	enum CullMode
	{
		CullNone = 0,
		CullCW,
		CullCCW
	};

	enum BlendOp
	{
		BlendAdd = 0,
		BlendSub,
		BlendRevSub,
		BlendMin,
		BlendMax
	};

	enum CompareFunc
	{
		CmpNever = 0,
		CmpLess,
		CmpEqual,
		CmpLessEqual,
		CmpGreater,
		CmpNotequal,
		CmpGreaterqual,
		CmpAlways,
	};

	enum BlendArg
	{
		ArgZero = 0,
		ArgOne,
		ArgSrcColor,
		ArgInvSrcColor,
		ArgSrcAlpha,
		ArgInvSrcAlpha,
		ArgDestAlpha,
		ArgInvDestAlpha,
		ArgDestColor,
		ArgInvDestColor
	};

	virtual void  SetVideoMode(int wgt, int hgt, void* data) = 0;
	virtual void* GetBackBuffer() { return nullptr; };
	virtual int   GetWidth() = 0;
	virtual int   GetHeight() = 0;
	virtual float GetAspect() = 0;

	virtual void Clear(bool renderTarget, Color color, bool zbuffer, float zValue) = 0;
	virtual void Present() = 0;

	virtual void SetProgram(Program* program) = 0;

	virtual VertexDecl* CreateVertexDecl(int count, VertexDecl::ElemDesc* elems) = 0;
	virtual void SetVertexDecl(VertexDecl* vdecl) = 0;

	virtual GeometryBuffer* CreateBuffer(int count, int stride) = 0;
	virtual void SetVertexBuffer(int slot, GeometryBuffer* buffer) = 0;
	virtual void SetIndexBuffer(GeometryBuffer* buffer) = 0;

	virtual Texture* CreateTexture(int w, int h, Texture::Format f, int l, bool rt, Texture::Type tp) = 0;

	virtual void Draw(Primitive prim, int startVertex, int primCount) = 0;
	virtual void DrawIndexed(Primitive prim, int startVertex, int startIndex, int primCount) = 0;

	virtual void SetAlphaBlend(bool enable) = 0;
	virtual void SetBlendFunc(BlendArg src, BlendArg dest) = 0;
	virtual void SetBlendOperation(BlendOp op) = 0;
	virtual void SetDepthTest(bool enable) = 0;
	virtual void SetDepthWriting(bool enable) = 0;
	virtual void SetDepthFunc(CompareFunc func) = 0;
	virtual void SetCulling(CullMode mode) = 0;
	virtual void SetupSlopeZBias(bool enable, float slopeZBias, float depthOffset) = 0;

	virtual void SetViewport(const Viewport& viewport) = 0;
	virtual void GetViewport(Viewport& viewport) = 0;

	virtual void SetRenderTarget(int slot, Texture* rt) = 0;
	virtual void SetDepth(Texture* depth) = 0;
	virtual void RestoreRenderTarget() = 0;

protected:
	virtual Shader* CreateShader(Shader::Type type, const char* name) = 0;
};