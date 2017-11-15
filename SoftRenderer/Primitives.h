#pragma once

#ifndef _PIRMITIVES_H_
#define _PIRMITIVES_H_

#include "Vector.h"
#include "Color.h"

struct Texture {
	float u, v;
};

struct Vertex {
	Vector3 point;
	RGBAColor color;
	Texture tex;
	Vector3 normal;
};

// 直线图元
struct Line {
	Vertex vertices[2];
};

// 三角图元
struct Primitive {
	UInt vectorIndex[3];
};

// 三角Mesh
struct Mesh {
	vector<Vertex> vertices;

	vector<Primitive> primitives;
};

struct TVertex {
	Vector4 point;
	RGBAColor color;
	Texture tex;
	float rhw;

	TVertex() {}
	TVertex(const Vertex & vertex) : point(vertex.point), color(vertex.color), tex(tex) {}
	TVertex(Vector4 point, RGBAColor color, float u, float v, float rhw) : point(point), color(color),
		tex(tex), rhw(rhw) {}

	void init_rhw() {
		rhw = 1.0f / point.w;
		color *= rhw;
	}
	TVertex operator+ (const TVertex & vertex) const {
		return TVertex {
			Vector4(
				point.x + vertex.point.x,
				point.y + vertex.point.y,
				point.z + vertex.point.z,
				point.w + vertex.point.w
			),
			RGBAColor(
				color.rgb.r + vertex.color.rgb.r,
				color.rgb.g + vertex.color.rgb.g,
				color.rgb.b + vertex.color.rgb.b,
				color.alpha + vertex.color.alpha
				),
			tex.u + vertex.tex.u,
			tex.v + vertex.tex.v,
			rhw + vertex.rhw 
		};
	}
	TVertex & operator+= (const TVertex & vertex) {
		point.x += vertex.point.x;
		point.y += vertex.point.y;
		point.z += vertex.point.z;
		point.w += vertex.point.w;
		color.rgb.r += vertex.color.rgb.r;
		color.rgb.g += vertex.color.rgb.g;
		color.rgb.b += vertex.color.rgb.b;
		color.alpha += vertex.color.alpha;
		tex.u += vertex.tex.u;
		tex.v += vertex.tex.v;
		rhw += vertex.rhw;
		return *this;
	}
	TVertex operator- (const TVertex & vertex) const {
		return TVertex{
			Vector4(
				point.x - vertex.point.x,
				point.y - vertex.point.y,
				point.z - vertex.point.z,
				point.w - vertex.point.w
			),
			RGBAColor(
				color.rgb.r - vertex.color.rgb.r,
				color.rgb.g - vertex.color.rgb.g,
				color.rgb.b - vertex.color.rgb.b,
				color.alpha - vertex.color.alpha
			),
			tex.u - vertex.tex.u,
			tex.v - vertex.tex.v,
			rhw - vertex.rhw
		};
	}
	TVertex & operator-= (const TVertex & vertex) {
		point.x -= vertex.point.x;
		point.y -= vertex.point.y;
		point.z -= vertex.point.z;
		point.w -= vertex.point.w;
		color.rgb.r -= vertex.color.rgb.r;
		color.rgb.g -= vertex.color.rgb.g;
		color.rgb.b -= vertex.color.rgb.b;
		color.alpha -= vertex.color.alpha;
		tex.u -= vertex.tex.u;
		tex.v -= vertex.tex.v;
		rhw -= vertex.rhw;
		return *this;
	}
	TVertex operator* (float k) const {
		return TVertex { 
			Vector4(
				point.x * k,
				point.y * k,
				point.z * k,
				point.w * k
			),
			RGBAColor(
				color.rgb * k,
				color.alpha * k
			),
			tex.u * k,
			tex.v * k,
			rhw * k 
		};
	}
	TVertex & operator*= (float k) {
		point.x *= k;
		point.y *= k;
		point.z *= k;
		point.w *= k;
		color.rgb *= k;
		color.alpha *= k;
		tex.u *= k;
		tex.v *= k;
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
	TriangleType type;   // 切割的三角形编号(00:无 01:平顶 10:平底 11:平顶+平底)
};

#endif