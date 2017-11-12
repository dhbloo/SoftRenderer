#pragma once

#ifndef _PIRMITIVES_H_
#define _PIRMITIVES_H_

#include "Vector.h"
#include "Color.h"

// 直线图元
struct Line {
	Vector3 vertices[2];
	RGBAColor color;
	Line(Vector3 v1, Vector3 v2, RGBAColor c) {
		vertices[0] = v1;
		vertices[1] = v2;
		color = c;
	}
};

// 三角图元
struct Primitive {
	UInt vectorIndex[3];
	UInt colorIndex[3] = { 0, 0, 0 };
};

// 三角Mesh
struct Mesh {
	vector<Vector3> vertices;
	vector<RGBAColor> colors;

	vector<Primitive> primitives;
};

#endif