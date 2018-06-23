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
	enum RenderState { 
		Wireframe = 1, 
		Color = 2, 
		Texture = 4, 
		ColoredTexture = Color | Texture,
		Shading = 8 
	};

private:
	IntBuffer & renderBuffer;   // 渲染缓冲区
	FloatBuffer ZBuffer;        // Z Buffer
	omp_lock_t * locks;         // 多线程锁

	RGBColor clearColor;
	RenderState renderState;

	shared_ptr<IntBuffer> currentTexture;
	ShadeFunc currentShadeFunc;

	// 画像素点(会检查越界)
	void drawPixel(int x, int y, const RGBColor & color);
	// 光栅化直线
	void rasterizeLine(int x0, int y0, int x1, int y1, RGBColor c0, RGBColor c1);
	// 光栅化扫描线
	void rasterizeScanline(Scanline & scanline);
	// 切割三角形(任意三角形切为平顶三角形和平底三角形)
	void triangleSpilt(SplitedTriangle & st, const TVertex * v0, const TVertex * v1, const TVertex * v2);
	// 根据y值将平底（顶）三角形转变为扫描线数据
	void rasterizeTriangle(const SplitedTriangle & st);

	// 判断点是否在CVV里面,返回标识位置的码,用于视锥裁剪
	int checkCVV(const Vector4 & v);
	// 坐标归一化,并转换到屏幕空间
	void transformHomogenize(const Vector4 & src, Vector3 & dst);

	// 渲染一条直线
	void renderLine(const Line & line, const Matrix44 & transform);
	// 渲染一个mesh
	void renderMesh(const shared_ptr<Mesh> mesh, const Matrix44 & transform, const Matrix44 & normalMatrix);

public:
	Pipeline(IntBuffer & renderBuffer);
	~Pipeline();

	// 设置渲染状态
	void setRenderState(RenderState state) { this->renderState = state; }
	// 设置清除颜色
	void setClearColor(RGBColor clearColor) { this->clearColor = clearColor; }
	
	// 渲染一帧
	void render(const Scene & scene);
};

#endif