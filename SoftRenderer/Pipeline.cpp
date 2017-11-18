#include "Pipeline.h"

void Pipeline::drawPixel(int x, int y, const RGBColor & color) {
	if (x >= 0 && x < renderBuffer.getWidth() && y >= 0 && y < renderBuffer.getHeight())
		renderBuffer.set(x, y, color.toRGBInt());
}

void Pipeline::rasterizeLine(int x0, int y0, int x1, int y1, RGBColor c0, RGBColor c1) {
	if (x0 == x1 && y0 == y1) {
		drawPixel(x0, y0, (c0 + c1) * 0.5f);
	} else if (x0 == x1) {
		if (y1 < y0) swap(y0, y1), swap(c0, c1);
		RGBColor ci = (c1 - c0) / (float)(y1 - y0);
		for (int y = y0; y <= y1; y++, c0 += ci) drawPixel(x0, y, c0);
	} else if (y0 == y1) {
		if (x1 < x0) swap(x0, x1), swap(c0, c1);
		RGBColor ci = (c1 - c0) / (float)(x1 - x0);
		for (int x = x0; x <= x1; x++, c0 += ci) drawPixel(x, y0, c0);
	} else {
		int rem = 0;
		int dx = (x0 < x1) ? x1 - x0 : x0 - x1;
		int dy = (y0 < y1) ? y1 - y0 : y0 - y1;
		if (dx >= dy) {
			if (x1 < x0) swap(x0, x1), swap(y0, y1), swap(c0, c1);
			RGBColor ci = (c1 - c0) / (float)(x1 - x0);
			for (int x = x0, y = y0; x <= x1; x++, c0 += ci) {
				drawPixel(x, y, c0);
				rem += dy;
				if (rem >= dx) {
					rem -= dx;
					y += (y1 >= y0) ? 1 : -1;
					drawPixel(x, y, c0);
				}
			}
			drawPixel(x1, y1, c0);
		} else {
			if (y1 < y0) swap(x0, x1), swap(y0, y1), swap(c0, c1);
			RGBColor ci = (c1 - c0) / (float)(y1 - y0);
			for (int x = x0, y = y0; y <= y1; y++, c0 += ci) {
				drawPixel(x, y, c0);
				rem += dx;
				if (rem >= dy) {
					rem -= dy;
					x += (x1 >= x0) ? 1 : -1;
					drawPixel(x, y, c0);
				}
			}
			drawPixel(x1, y1, c0);
		}
	}
}

void Pipeline::rasterizeScanline(Scanline & scanline) {
	int * fbPtr = renderBuffer(0, scanline.y);
	float * zbPtr = ZBuffer(0, scanline.y);
	int x0 = MAX(scanline.x0, 0), x1 = MIN(scanline.x1, renderBuffer.getWidth() - 1);
	TVertex vi = scanline.v0, v;
	RGBColor c;
	int rs = currentTexture ? renderState : renderState & (~Texture);
	rs = currentShadeFunc ? rs : rs & (~Shading);
	float invW = 1.f / renderBuffer.getWidth(), invH = 1.f / renderBuffer.getHeight();
	Vector3 pos;
	for (int x = x0; x <= x1; x++) {
		float rhw = vi.rhw;
		if (rhw >= zbPtr[x]) {  // 使用Z-buffer判断深度是否满足
			v = vi * (1.0f / rhw);
			if (rs & Shading) {
				pos = vi.point, pos.x *= invW, pos.y *= invH;
				if (currentShadeFunc(c, pos, v.color, v.normal.NormalizedVector(), v.tex)) {
					fbPtr[x] = c.toRGBInt();
					zbPtr[x] = rhw;
				}
			} else {
				if (rs & Texture) {
					c.setRGBInt(currentTexture->get(v.tex.u, v.tex.v));
					if (rs & Color) c *= v.color;
				} else if (rs & Color) {
					c = v.color;
				}
				fbPtr[x] = c.toRGBInt();
				zbPtr[x] = rhw;
			}
		}
		vi += scanline.step;
	}
}

void Pipeline::triangleSpilt(SplitedTriangle & st, const TVertex * v0, const TVertex * v1, const TVertex * v2) {
	// 三角形顶点按照Y坐标排序（v0 <= v1 <= v2）
	if (v0->point.y > v1->point.y) swap(v0, v1);
	if (v0->point.y > v2->point.y) swap(v0, v2);
	if (v1->point.y > v2->point.y) swap(v1, v2);

	// 判断三角形共线
	if (MathUtils::isZero(v0->point.y - v1->point.y) && MathUtils::isZero(v1->point.y - v2->point.y) ||
		MathUtils::isZero(v0->point.x - v1->point.x) && MathUtils::isZero(v1->point.x - v2->point.x)) {
		st.type = SplitedTriangle::NONE;
		return;
	}
	
	if (MathUtils::isZero(v0->point.y - v1->point.y)) { // 底边Y相等（平底三角形）
		assert(v2->point.y > v0->point.y);
		if (v0->point.x > v1->point.x) swap(v0, v1);

		st.top = *v2;
		st.left = *v0;
		st.right = *v1;
		st.type = SplitedTriangle::FLAT_BOTTOM;
		return;
	} else if (MathUtils::isZero(v1->point.y - v2->point.y)) { // 顶边Y相等（平顶三角形）
		assert(v2->point.y > v0->point.y);
		if (v1->point.x > v2->point.x) swap(v1, v2);

		st.bottom = *v0;
		st.left = *v1;
		st.right = *v2;
		st.type = SplitedTriangle::FLAT_TOP;
		return;
	}

	st.top = *v2;
	st.bottom = *v0;
	st.type = SplitedTriangle::FLAT_TOP_BOTTOM;

	float factor = (v1->point.y - v0->point.y) / (v2->point.y - v0->point.y);
	TVertex splitV = MathUtils::lerp(st.bottom, st.top, factor);

	if (splitV.point.x <= v1->point.x) {
		//splitV.point.y = v1->point.y;
		st.left = splitV;
		st.right = *v1;
	} else {
		st.left = *v1;
		st.right = splitV;
	}
}

void Pipeline::rasterizeTriangle(const SplitedTriangle & st) {
	if (st.type & SplitedTriangle::FLAT_TOP) {
		int y0 = (int)st.bottom.point.y + 1, y1 = (int)st.left.point.y;
	#pragma omp parallel for schedule(dynamic)
		for (int y = y0; y <= y1; y++) {
			float factor = (y - st.bottom.point.y) / (st.left.point.y - st.bottom.point.y);
			TVertex left = MathUtils::lerp(st.bottom, st.left, factor);
			TVertex right = MathUtils::lerp(st.bottom, st.right, factor);
			Scanline scanline;
			scanline.x0 = (int)left.point.x;
			scanline.x1 = (int)right.point.x;
			scanline.y = y;
			scanline.v0 = left;
			scanline.step = (right - left) * (1.0f / (right.point.x - left.point.x));
			rasterizeScanline(scanline);
		}
	}
	if (st.type & SplitedTriangle::FLAT_BOTTOM) {
		int y0 = (int)st.left.point.y + 1, y1 = (int)st.top.point.y;
	#pragma omp parallel for schedule(dynamic)
		for (int y = y0; y <= y1; y++) {
			float factor = (y - st.left.point.y) / (st.top.point.y - st.left.point.y);
			TVertex left = MathUtils::lerp(st.left, st.top, factor);
			TVertex right = MathUtils::lerp(st.right, st.top, factor);
			Scanline scanline;
			scanline.x0 = (int)left.point.x;
			scanline.x1 = (int)right.point.x;
			scanline.y = y;
			scanline.v0 = left;
			scanline.step = (right - left) * (1.0f / (right.point.x - left.point.x));
			rasterizeScanline(scanline);
		}
	}
}

int Pipeline::transformCheckCVV(const Vector4 & v) {
	float w = v.w;
	int check = 0;
	if (v.z < 0.f) check |= 1;
	if (v.z >  w) check |= 2;
	if (v.x < -w) check |= 4;
	if (v.x >  w) check |= 8;
	if (v.y < -w) check |= 16;
	if (v.y >  w) check |= 32;
	return check;
}

void Pipeline::transformHomogenize(const Vector4 & src, Vector3 & dst) {
	float rhw = 1.0f / src.w;
	dst.x = (src.x * rhw + 1.0f) * renderBuffer.getWidth() * 0.5f;
	dst.y = (1.0f - src.y * rhw) * renderBuffer.getHeight() * 0.5f;
	dst.z = src.z * rhw;
}

void Pipeline::renderMesh(const shared_ptr<Mesh> mesh, const Matrix44 & transform, const Matrix44 & normalMatrix) {
	vector<Vertex> & v = mesh->vertices;
	currentTexture = mesh->texture;
	currentShadeFunc = mesh->shadeFunc;

	Vertex * vo[3];
	Vector4 c0, c1, c2;
	Vector3 p0, p1, p2;
	for each (auto p in mesh->primitives) {
		vo[0] = &v[p.vectorIndex[0]];
		vo[1] = &v[p.vectorIndex[1]];
		vo[2] = &v[p.vectorIndex[2]];
		// 按照 Transform 变化
		transform.apply(vo[0]->point, c0);
		transform.apply(vo[1]->point, c1);
		transform.apply(vo[2]->point, c2);

		// 裁剪测试:可以完善为进一步精细裁剪
		int cvv[3] = { transformCheckCVV(c0), transformCheckCVV(c1), transformCheckCVV(c2) };
		// 全部顶点都在屏幕外就不渲染
		if (cvv[0] && cvv[1] && cvv[2]) continue;

		// 归一化
		transformHomogenize(c0, p0);
		transformHomogenize(c1, p1);
		transformHomogenize(c2, p2);

		if ((renderState & (~Wireframe)) && (!(cvv[0] || cvv[1] || cvv[2]))) {
			TVertex v0(*vo[0]), v1(*vo[1]), v2(*vo[2]);
			SplitedTriangle st;
			v0.point = p0;
			v1.point = p1;
			v2.point = p2;

			if (p.extraNormal.isZero()) {
				normalMatrix.applyDir(vo[0]->normal, v0.normal);
				normalMatrix.applyDir(vo[1]->normal, v1.normal);
				normalMatrix.applyDir(vo[2]->normal, v2.normal);
			} else {
				normalMatrix.applyDir(p.extraNormal, v0.normal);
				v1.normal = v2.normal = v0.normal;
			}

			v0.init_rhw(c0.w);
			v1.init_rhw(c1.w);
			v2.init_rhw(c2.w);

			triangleSpilt(st, &v0, &v1, &v2);
			rasterizeTriangle(st);
		}

		if (renderState & Wireframe) {
			rasterizeLine((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, vo[0]->color, vo[1]->color);
			rasterizeLine((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, vo[1]->color, vo[2]->color);
			rasterizeLine((int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, vo[2]->color, vo[0]->color);
		}
	}
}

void Pipeline::renderLine(const Line & line, const Matrix44 & transform) {
	Vector4 c0, c1;
	Vector3 p0, p1;
	transform.apply(line.vertices[0].point, c0);
	transform.apply(line.vertices[1].point, c1);

	if (transformCheckCVV(c0) && transformCheckCVV(c1)) return;

	transformHomogenize(c0, p0);
	transformHomogenize(c1, p1);

	rasterizeLine((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, line.vertices[0].color, line.vertices[1].color);
}

void Pipeline::render(const Scene & scene) {
	renderBuffer.fill(clearColor.toRGBInt());
	ZBuffer.fill(0.f);

	Matrix44 projectionViewTransform = scene.view * scene.projection;

	for (size_t i = 0; i < scene.meshes.size(); i++) {
		renderMesh(scene.meshes[i], scene.modelMatrixs[i] * projectionViewTransform, scene.modelMatrixs[i] * scene.view);
	}

	for each (auto line in scene.lines) {
		renderLine(line, projectionViewTransform);
	}
}
