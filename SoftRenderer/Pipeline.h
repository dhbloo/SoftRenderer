#pragma once

#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include "Matrix44.h"
#include "FrameBuffer.h"
#include "Primitives.h"
#include "Scene.h"

#include <omp.h>

class Pipeline {
public:
	enum RenderState { Wireframe = 1, Color = 2, Texture = 4, ColoredTexture = 6, Shading = 8 };
private:
	IntBuffer & renderBuffer;
	FloatBuffer ZBuffer;
	omp_lock_t * locks;

	RGBColor clearColor;
	RenderState renderState;

	shared_ptr<IntBuffer> currentTexture;
	ShadeFunc currentShadeFunc;

	// 绘制点
	void drawPixel(int x, int y, const RGBColor & color);
	// 直线的光栅化
	void rasterizeLine(int x0, int y0, int x1, int y1, RGBColor c0, RGBColor c1);
	// 扫描线的光栅化
	void rasterizeScanline(Scanline & scanline);
	// 将任意三角形分割为两个平底（顶）三角形
	void triangleSpilt(SplitedTriangle & st, const TVertex * v0, const TVertex * v1, const TVertex * v2);
	// 根据y值将平底（顶）三角形转变为扫描线数据
	void rasterizeTriangle(const SplitedTriangle & st);

	// 检查齐次坐标同 cvv 的边界用于视锥裁剪
	int transformCheckCVV(const Vector4 & v);
	// 归一化，得到屏幕坐标
	void transformHomogenize(const Vector4 & src, Vector3 & dst);

	void renderMesh(const shared_ptr<Mesh> mesh, const Matrix44 & transform, const Matrix44 & normalMatrix);
	void renderLine(const Line & line, const Matrix44 & transform);

public:
	Pipeline(IntBuffer & renderBuffer) : renderBuffer(renderBuffer),
		ZBuffer(renderBuffer.getWidth(), renderBuffer.getHeight()),
		renderState(Wireframe) {
		locks = new omp_lock_t[renderBuffer.getHeight()];
		for (int i = 0; i < renderBuffer.getHeight(); i++)
			omp_init_lock(locks + i);
	}
	~Pipeline() {
		for (int i = 0; i < renderBuffer.getHeight(); i++)
			omp_destroy_lock(locks + i);
		delete[] locks;
	}

	void setRenderState(RenderState state) { this->renderState = state; }
	void setClearColor(RGBColor clearColor) { this->clearColor = clearColor; }
	
	void render(const Scene & scene);
};

#endif