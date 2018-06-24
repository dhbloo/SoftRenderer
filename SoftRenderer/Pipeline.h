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
	// ��Ⱦ״̬(ָʾ��ǰ����Ⱦģʽ)
	enum RenderState { 
		WIREFRAME = 1, 
		COLOR = 2, 
		TEXTURE = 4, 
		COLOR_TEXTURE = COLOR | TEXTURE,
		SHADING = 8 
	};

	// ���״̬(ָʾ��Ⱦǰ�����Щbuffer)
	enum ClearState {
		CLEAR_NONE = 0,
		CLEAR_COLOR = 1,
		CLEAR_DEPTH = 2,
		CLEAR_COLOR_DEPTH = CLEAR_COLOR | CLEAR_DEPTH
	};

private:
	////          ������Buffer          ////
	IntBuffer & renderBuffer;   // ��Ⱦ������
	FloatBuffer ZBuffer;        // Z Buffer
	omp_lock_t * locks;         // ���߳���

	////          ��ǰ��Ⱦ����          ////

	RGBColor clearColor;        // �����ɫ
	RenderState renderState;    // ��ǰ����Ⱦ״̬
	ClearState clearState;      // ��ǰ�����״̬

	bool smoothLine;            // �Ƿ������������

	////       ��ǰ��Ⱦ��״̬����       ////

	shared_ptr<IntBuffer> currentTexture;   // ��ǰMeshʹ�õ�����
	ShadeFunc currentShadeFunc;             // ��ǰMeshʹ�õ���ɫ����

	// �����ص�(����Խ��)
	void drawPixel(int x, int y, const RGBColor & color);
	// ��դ��ֱ��(Bresenham's algorithm)
	void rasterizeLine(int x0, int y0, int x1, int y1, RGBColor c0, RGBColor c1);
	// ��դ��������ֱ��(Xiaolin Wu's line algorithm)
	void rasterizeLine_antialiasing(float x0, float y0, float x1, float y1, RGBColor c0, RGBColor c1);
	// ��դ��ɨ����
	void rasterizeScanline(Scanline & scanline);
	// �и�������(������������Ϊƽ�������κ�ƽ��������)
	void triangleSpilt(SplitedTriangle & st, const TVertex * v0, const TVertex * v1, const TVertex * v2);
	// ����yֵ��ƽ�ף�����������ת��Ϊɨ��������
	void rasterizeTriangle(const SplitedTriangle & st);

	// �жϵ��Ƿ���CVV����,���ر�ʶλ�õ���,������׶�ü�
	int checkCVV(const Vector4 & v);
	// �����һ��,��ת������Ļ�ռ�
	void transformHomogenize(const Vector4 & src, Vector3 & dst);

	// ��Ⱦһ��ֱ��
	void renderLine(const Line & line, const Matrix44 & transform);
	// ��Ⱦһ��mesh
	void renderMesh(const shared_ptr<Mesh> mesh, const Matrix44 & transform, const Matrix44 & normalMatrix);

public:
	Pipeline(IntBuffer & renderBuffer);
	~Pipeline();

	// ������Ⱦ״̬
	void setRenderState(RenderState state) { this->renderState = state; }
	// ���������ɫ
	void setClearColor(RGBColor clearColor) { this->clearColor = clearColor; }
	
	// ��Ⱦһ֡
	void render(const Scene & scene);
};

#endif