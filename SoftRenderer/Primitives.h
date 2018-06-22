#pragma once

#ifndef _PIRMITIVES_H_
#define _PIRMITIVES_H_

#include "Vector.h"
#include "Color.h"
#include "FrameBuffer.h"

struct TexCoord {
	float u, v;
};

struct Vertex {
	Vector3 point;
	RGBColor color;
	TexCoord tex;
	Vector3 normal;
};

// ֱ��ͼԪ
struct Line {
	Vertex vertices[2];
};

// ����ͼԪ
struct Primitive {
	size_t vectorIndex[3];
	Vector3 extraNormal = Vector3::Zero();
};

typedef function<bool(RGBColor & out, const Vector3 & pos, const RGBColor & color, const Vector3 & normal, const shared_ptr<IntBuffer> & texture, const TexCoord & tex)> ShadeFunc;

// ����Mesh
struct Mesh {
	vector<Vertex> vertices;
	vector<Primitive> primitives;
	shared_ptr<IntBuffer> texture;
	ShadeFunc shadeFunc;
};

struct TVertex {
	Vector3 point;
	RGBColor color;
	TexCoord tex;
	Vector3 normal;
	float rhw;

	TVertex() {}
	TVertex(const Vertex & vertex) : color(vertex.color), tex(vertex.tex) {}
	TVertex(const Vector3 & point, const RGBColor & color, float u, float v, const Vector3 & normal, float rhw) :
		point(point), color(color), normal(normal), rhw(rhw) {
		tex.u = u; tex.v = v;
	}

	void init_rhw(float w) {
		rhw = 1.0f / w;
		color *= rhw;
		tex.u *= rhw;
		tex.v *= rhw;
		normal *= rhw;
	}
	TVertex operator+ (const TVertex & vertex) const {
		return TVertex {
			Vector3(
				point.x + vertex.point.x,
				point.y + vertex.point.y,
				point.z + vertex.point.z
			),
			color + vertex.color,
			tex.u + vertex.tex.u,
			tex.v + vertex.tex.v,
			normal + vertex.normal,
			rhw + vertex.rhw
		};
	}
	TVertex & operator+= (const TVertex & vertex) {
		point.x += vertex.point.x;
		point.y += vertex.point.y;
		point.z += vertex.point.z;
		color += vertex.color;
		tex.u += vertex.tex.u;
		tex.v += vertex.tex.v;
		normal += vertex.normal;
		rhw += vertex.rhw;
		return *this;
	}
	TVertex operator- (const TVertex & vertex) const {
		return TVertex{
			Vector3(
				point.x - vertex.point.x,
				point.y - vertex.point.y,
				point.z - vertex.point.z
			),
			color - vertex.color,
			tex.u - vertex.tex.u,
			tex.v - vertex.tex.v,
			normal - vertex.normal,
			rhw - vertex.rhw,
		};
	}
	TVertex & operator-= (const TVertex & vertex) {
		point.x -= vertex.point.x;
		point.y -= vertex.point.y;
		point.z -= vertex.point.z;
		color -= vertex.color;
		tex.u -= vertex.tex.u;
		tex.v -= vertex.tex.v;
		normal -= vertex.normal;
		rhw -= vertex.rhw;
		return *this;
	}
	TVertex operator* (float k) const {
		return TVertex{
			Vector3(
				point.x * k,
				point.y * k,
				point.z * k
			),
			color * k,
			tex.u * k,
			tex.v * k,
			normal * k,
			rhw * k ,
		};
	}
	TVertex & operator*= (float k) {
		point.x *= k;
		point.y *= k;
		point.z *= k;
		color *= k;
		tex.u *= k;
		tex.v *= k;
		normal *= k;
		rhw *= k;
		return *this;
	}
	
};

struct Scanline {
	TVertex v0, step;
	int x0, x1, y;
};

struct SplitedTriangle {
	enum TriangleType { NONE, FLAT_TOP, FLAT_BOTTOM, FLAT_TOP_BOTTOM };
	TVertex top;
	TVertex left, right;
	TVertex bottom;
	TriangleType type;   // �и�������α��(00:�� 01:ƽ�� 10:ƽ�� 11:ƽ��+ƽ��)
};

#endif