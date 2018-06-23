#pragma once

#ifndef _PIRMITIVES_H_
#define _PIRMITIVES_H_

#include "Vector.h"
#include "Color.h"
#include "FrameBuffer.h"

typedef Vector2 TexCoord;

struct Vertex {
	Vector3 point;
	RGBColor color;
	TexCoord texCoord;
	Vector3 normal;
};

// ֱ��ͼԪ
struct Line {
	Vertex vertices[2];
};

// ����ͼԪ
struct Primitive {
	size_t vertexIndex[3];
	Vector3 extraNormal = Vector3::Zero();
};

// ��ɫ����
typedef function<
	bool(RGBColor & out, const Vector3 & pos, const RGBColor & color, const Vector3 & normal,
		const shared_ptr<IntBuffer> & texture, const TexCoord & texCoord)
> ShadeFunc;

// ����Mesh
struct Mesh {
	vector<Vertex> vertices;
	vector<Primitive> primitives;
	shared_ptr<IntBuffer> texture;
	ShadeFunc shadeFunc;
};

// ��͸�ӽ����Ĳ�ֵ����
struct TVertex {
	Vector3 point;
	RGBColor color;
	TexCoord texCoord;
	Vector3 normal;
	float rhw;

	TVertex() {}
	TVertex(const Vertex & vertex) : color(vertex.color), texCoord(vertex.texCoord) {}
	TVertex(const Vector3 & point, const RGBColor & color, TexCoord texCoord, const Vector3 & normal, float rhw) :
		point(point), color(color), normal(normal), rhw(rhw), texCoord(texCoord) {}

	void init_rhw(float w) {
		rhw = 1.0f / w;
		color *= rhw;
		texCoord *= rhw;
		normal *= rhw;
	}
	TVertex operator+ (const TVertex & vertex) const {
		return TVertex {
			point + vertex.point,
			color + vertex.color,
			texCoord + vertex.texCoord,
			normal + vertex.normal,
			rhw + vertex.rhw
		};
	}
	TVertex & operator+= (const TVertex & vertex) {
		point += vertex.point;
		color += vertex.color;
		texCoord += vertex.texCoord;
		normal += vertex.normal;
		rhw += vertex.rhw;
		return *this;
	}
	TVertex operator- (const TVertex & vertex) const {
		return TVertex{
			point - vertex.point,
			color - vertex.color,
			texCoord - vertex.texCoord,
			normal - vertex.normal,
			rhw - vertex.rhw,
		};
	}
	TVertex & operator-= (const TVertex & vertex) {
		point -= vertex.point;
		color -= vertex.color;
		texCoord -= vertex.texCoord;
		normal -= vertex.normal;
		rhw -= vertex.rhw;
		return *this;
	}
	TVertex operator* (float k) const {
		return TVertex{
			point * k,
			color * k,
			texCoord * k,
			normal * k,
			rhw * k,
		};
	}
	TVertex & operator*= (float k) {
		point *= k;
		color *= k;
		texCoord *= k;
		normal *= k;
		rhw *= k;
		return *this;
	}
	
};

// ����ɨ����(����)
struct Scanline {
	TVertex v0, step;
	int x0, x1, y;
};

// �и�����������
struct SplitedTriangle {
	// �и�������������
	// 00:�� 01:ƽ�� 10:ƽ�� 11:ƽ��+ƽ��
	enum TriangleType { 
		NONE, 
		FLAT_TOP, 
		FLAT_BOTTOM, 
		FLAT_TOP_BOTTOM 
	};

	TVertex top;
	TVertex left, right;
	TVertex bottom;
	TriangleType type;   
};

#endif