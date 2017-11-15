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
	// 直线的光栅化
	void rasterizeLine(int x0, int y0, int x1, int y1, RGBAColor c0, RGBAColor c1);
	// 对一条扫描线的光栅化
	void rasterizeScanline(Scanline & scanline);
	// 将任意三角形分割为两个平底（顶）三角形
	void triangleSpilt(SplitedTriangle & st, const TVertex * v0, const TVertex * v1, const TVertex * v2);
	// 根据y值将平底（顶）三角形转变为扫描线数据
	void rasterizeTriangle(const SplitedTriangle & st);

	// 检查齐次坐标同 cvv 的边界用于视锥裁剪
	int transformCheckCVV(const Vector4 & v);
	void transformHomogenize(const Vector4 & src, Vector3 & dst);

	void renderMesh(const shared_ptr<Mesh> mesh, const Matrix44 & transform);
	void renderLine(const Line & line, const Matrix44 & transform);

public:
	Pipeline(IntBuffer & renderBuffer) : renderBuffer(renderBuffer),
		ZBuffer(renderBuffer.getWidth(), renderBuffer.getHeight()),
		renderState(Wireframe) {}
	~Pipeline() {}

	void setRenderState(RenderState state) { this->renderState = state; }
	void setClearColor(RGBColor clearColor) { this->clearColor = clearColor; }
	
	void render(const Scene & scene);
};

#endif