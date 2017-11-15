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
	// ֱ�ߵĹ�դ��
	void rasterizeLine(int x0, int y0, int x1, int y1, RGBAColor c0, RGBAColor c1);
	// ��һ��ɨ���ߵĹ�դ��
	void rasterizeScanline(Scanline & scanline);
	// �����������ηָ�Ϊ����ƽ�ף�����������
	void triangleSpilt(SplitedTriangle & st, const TVertex * v0, const TVertex * v1, const TVertex * v2);
	// ����yֵ��ƽ�ף�����������ת��Ϊɨ��������
	void rasterizeTriangle(const SplitedTriangle & st);

	// ����������ͬ cvv �ı߽�������׶�ü�
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