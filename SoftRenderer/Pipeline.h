#pragma once

#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "Matrix44.h"
#include "FrameBuffer.h"
#include "Primitives.h"
#include "Scene.h"

class Pipeline {
public:
	enum RenderState { Wireframe = 1, Color = 2, Texture = 4 };
private:
	IntBuffer & renderBuffer;
	FloatBuffer ZBuffer;

	RGBColor clearColor;
	RenderState renderState;

	void drawPixel(int x, int y, RGBAColor color);
	void rasterizeLine(int x0, int y0, int x1, int y1, RGBAColor color);
	void rasterizePrimitive(int v0, int v1, int v2, RGBAColor c1, RGBAColor c2, RGBAColor c3);

	// 检查齐次坐标同 cvv 的边界用于视锥裁剪
	int transformCheckCVV(const Vector4 & v);
	void transformHomogenize(const Vector4 & src, Vector3 & dst);

	void renderMesh(shared_ptr<Mesh> mesh, const Matrix44 & transform);

public:
	Pipeline(IntBuffer & renderBuffer) : renderBuffer(renderBuffer),
		ZBuffer(renderBuffer.getWidth(), renderBuffer.getHeight()),
		renderState(Wireframe) {}
	~Pipeline() {}

	void setClearColor(RGBColor clearColor) { this->clearColor = clearColor; }
	
	void render(Scene & scene);
};

#endif