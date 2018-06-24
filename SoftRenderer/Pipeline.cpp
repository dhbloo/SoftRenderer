#include "Pipeline.h"

Pipeline::Pipeline(IntBuffer & renderBuffer) : renderBuffer(renderBuffer),
renderState(WIREFRAME), clearState(CLEAR_COLOR_DEPTH),
smoothLine(true),
ZBuffer(renderBuffer.getWidth(), renderBuffer.getHeight()) {
	locks = new omp_lock_t[renderBuffer.getHeight()];
	for (int i = 0; i < renderBuffer.getHeight(); i++)
		omp_init_lock(locks + i);
}

Pipeline::~Pipeline() {
	for (int i = 0; i < renderBuffer.getHeight(); i++)
		omp_destroy_lock(locks + i);
	delete[] locks;
}

inline void Pipeline::drawPixel(int x, int y, const RGBColor & color) {
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

void Pipeline::rasterizeLine_antialiasing(float x0, float y0, float x1, float y1, RGBColor c0, RGBColor c1) {
	bool steep = std::abs(y1 - y0) > std::abs(x1 - x0);

	if (steep)
		std::swap(x0, y0), std::swap(x1, y1);
	if (x0 > x1)
		std::swap(x0, x1), std::swap(y0, y1), std::swap(c0, c1);

	float dx = x1 - x0, dy = y1 - y0;
	float gradient = (dx == 0.0f) ? 1.0f : dy / dx;

	// handle first endpoint
	float xend = std::round(x0);
	float yend = y0 + gradient * (xend - x0);
	float xgap = 1.0f - Math::fract(x0 + 0.5f);
	int xpxl1 = (int)xend;  // this will be used in the main loop
	int ypxl1 = (int)yend;

	if (steep) {
		drawPixel(ypxl1, xpxl1, (1.0f - Math::fract(yend)) * xgap * c0);
		drawPixel(ypxl1 + 1, xpxl1, Math::fract(yend) * xgap * c0);
	} else {
		drawPixel(xpxl1, ypxl1, (1.0f - Math::fract(yend)) * xgap * c0);
		drawPixel(xpxl1, ypxl1 + 1, Math::fract(yend) * xgap * c0);
	}

	// first y-intersection for the main loop
	float intery = yend + gradient;

	// handle second endpoint
	xend = std::round(x1);
	yend = y1 + gradient * (xend - x1);
	xgap = Math::fract(x1 + 0.5f);
	int xpxl2 = (int)xend; // this will be used in the main loop
	int ypxl2 = (int)yend;

	if (steep) {
		drawPixel(ypxl2, xpxl2, (1.0f - Math::fract(yend)) * xgap * c1);
		drawPixel(ypxl2 + 1, xpxl2, Math::fract(yend) * xgap * c1);
	} else {
		drawPixel(xpxl2, ypxl2, (1.0f - Math::fract(yend)) * xgap * c1);
		drawPixel(xpxl2, ypxl2 + 1, Math::fract(yend) * xgap * c1);
	}

	RGBColor ci = (c1 - c0) / (xpxl2 - xpxl1 - 2.f);
	RGBColor c = c0, co;
	if (steep) {
		for (int x = xpxl1 + 1; x < xpxl2; x++) {
			float fpart = Math::fract(intery);
			co.setRGBInt(renderBuffer.get((int)intery, x));
			drawPixel((int)intery, x, Math::lerp(co, c, 1.0f - fpart));
			co.setRGBInt(renderBuffer.get((int)intery + 1, x));
			drawPixel((int)intery + 1, x, Math::lerp(co, c, fpart));
			intery += gradient;
			c += ci;
		}
	} else {
		for (int x = xpxl1 + 1; x < xpxl2; x++) {
			float fpart = Math::fract(intery);
			co.setRGBInt(renderBuffer.get(x, (int)intery));
			drawPixel(x, (int)intery, Math::lerp(co, c, 1.0f - fpart));
			co.setRGBInt(renderBuffer.get(x, (int)intery + 1));
			drawPixel(x, (int)intery + 1, Math::lerp(co, c, fpart));
			intery += gradient;
			c += ci;
		}
	}
	
}

void Pipeline::rasterizeScanline(Scanline & scanline) {
	int * fbPtr = renderBuffer(0, scanline.y);
	float * zbPtr = ZBuffer(0, scanline.y);
	int x0 = MAX(scanline.x0, 0), x1 = MIN(scanline.x1, renderBuffer.getWidth() - 1);
	TVertex vi = scanline.v0, v;
	RGBColor c;
	int rs = currentTexture ? renderState : renderState & (~TEXTURE);
	rs = currentShadeFunc ? rs : rs & (~SHADING);
	float invW = 1.f / renderBuffer.getWidth(), invH = 1.f / renderBuffer.getHeight();
	Vector3 pos;
	omp_set_lock(locks + scanline.y);
	for (int x = x0; x <= x1; x++) {
		float rhw = vi.rhw;
		if (rhw >= zbPtr[x]) {  // ʹ��Z-buffer�ж�����Ƿ�����
			v = vi * (1.0f / rhw);
			if (rs & SHADING) {
				pos = vi.point, pos.x *= invW, pos.y *= invH;
				if (currentShadeFunc(c, pos, v.color, v.normal.NormalizedVector(), currentTexture, v.texCoord)) {
					fbPtr[x] = c.toRGBInt();
					zbPtr[x] = rhw;
				}
			} else {
				if (rs & TEXTURE) {
					c.setRGBInt(currentTexture->get(v.texCoord));
					if (rs & COLOR) c *= v.color;
				} else if (rs & COLOR) {
					c = v.color;
				}
				fbPtr[x] = c.toRGBInt();
				zbPtr[x] = rhw;
			}
		}
		vi += scanline.step;
	}
	omp_unset_lock(locks + scanline.y);
}

void Pipeline::rasterizeTriangle(const SplitedTriangle & st) {
	if (st.type & SplitedTriangle::FLAT_TOP) {
		int y0 = (int)st.bottom.point.y + 1;
		int y1 = (int)st.left.point.y;
		float yl = st.left.point.y - st.bottom.point.y;

		for (int y = y0; y <= y1; y++) {
			float factor = (y - st.bottom.point.y) / yl;
			TVertex left = Math::lerp(st.bottom, st.left, factor);
			TVertex right = Math::lerp(st.bottom, st.right, factor);
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
		int y0 = (int)st.left.point.y + 1;
		int y1 = (int)st.top.point.y;
		float yl = st.top.point.y - st.left.point.y;

		for (int y = y0; y <= y1; y++) {
			float factor = (y - st.left.point.y) / yl;
			TVertex left = Math::lerp(st.left, st.top, factor);
			TVertex right = Math::lerp(st.right, st.top, factor);
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

void Pipeline::triangleSpilt(SplitedTriangle & st, const TVertex * v0, const TVertex * v1, const TVertex * v2) {
	// �����ζ��㰴��Y��������v0 <= v1 <= v2��
	if (v0->point.y > v1->point.y) swap(v0, v1);
	if (v0->point.y > v2->point.y) swap(v0, v2);
	if (v1->point.y > v2->point.y) swap(v1, v2);

	// �ж������ι���
	if (Math::isZero(v0->point.y - v1->point.y) && Math::isZero(v1->point.y - v2->point.y) ||
		Math::isZero(v0->point.x - v1->point.x) && Math::isZero(v1->point.x - v2->point.x)) {
		st.type = SplitedTriangle::NONE;
		return;
	}

	if (Math::isZero(v0->point.y - v1->point.y)) { // �ױ�Y��ȣ�ƽ�������Σ�
		assert(v2->point.y > v0->point.y);
		if (v0->point.x > v1->point.x) swap(v0, v1);

		st.top = *v2;
		st.left = *v0;
		st.right = *v1;
		st.type = SplitedTriangle::FLAT_BOTTOM;
		return;
	} else if (Math::isZero(v1->point.y - v2->point.y)) { // ����Y��ȣ�ƽ�������Σ�
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
	TVertex splitV = Math::lerp(st.bottom, st.top, factor);

	if (splitV.point.x <= v1->point.x) {
		st.left = splitV;
		st.right = *v1;
	} else {
		st.left = *v1;
		st.right = splitV;
	}
}

int Pipeline::checkCVV(const Vector4 & v) {
	float w = v.w;
	int check = 0;
	if (v.z < 0.f) check |= 1;
	if (v.z > w)   check |= 2;
	if (v.x < -w)  check |= 4;
	if (v.x > w)   check |= 8;
	if (v.y < -w)  check |= 16;
	if (v.y > w)   check |= 32;
	return check;
}

void Pipeline::transformHomogenize(const Vector4 & src, Vector3 & dst) {
	dst = (Vector3)src;
	dst.x = (dst.x + 1.0f) * renderBuffer.getWidth() * 0.5f;
	dst.y = (1.0f - dst.y) * renderBuffer.getHeight() * 0.5f;
}

void Pipeline::renderLine(const Line & line, const Matrix44 & transform) {
	Vector4 c0, c1;
	Vector3 p0, p1;
	transform.apply(line.vertices[0].point, c0);
	transform.apply(line.vertices[1].point, c1);

	if (checkCVV(c0) && checkCVV(c1)) return;

	transformHomogenize(c0, p0);
	transformHomogenize(c1, p1);

	if (smoothLine) {
		rasterizeLine_antialiasing(p0.x, p0.y, p1.x, p1.y, line.vertices[0].color, line.vertices[1].color);
	} else {
		rasterizeLine((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, line.vertices[0].color, line.vertices[1].color);
	}
	
}

void Pipeline::renderMesh(const shared_ptr<Mesh> mesh, const Matrix44 & transform, const Matrix44 & normalMatrix) {
	vector<Vertex> & v = mesh->vertices;
	currentTexture = mesh->texture;
	currentShadeFunc = mesh->shadeFunc;

#pragma omp parallel for schedule(dynamic)
	for (int i = 0; (size_t)i < mesh->primitives.size(); i++) {
		Vertex * vo[3];
		Vector4 c0, c1, c2;
		Vector3 p0, p1, p2;
		Primitive & p = mesh->primitives[i];
		vo[0] = &v[p.vertexIndex[0]];
		vo[1] = &v[p.vertexIndex[1]];
		vo[2] = &v[p.vertexIndex[2]];
		// ���� Transform �仯
		transform.apply(vo[0]->point, c0);
		transform.apply(vo[1]->point, c1);
		transform.apply(vo[2]->point, c2);

		// �ü�����:��������Ϊ��һ����ϸ�ü�
		int cvv[3] = { checkCVV(c0), checkCVV(c1), checkCVV(c2) };
		// ȫ�����㶼����Ļ��Ͳ���Ⱦ
		if (cvv[0] && cvv[1] && cvv[2]) continue;

		// ��һ������Ļ�ռ�
		transformHomogenize(c0, p0);
		transformHomogenize(c1, p1);
		transformHomogenize(c2, p2);

		if ((renderState & (~WIREFRAME)) && (!(cvv[0] || cvv[1] || cvv[2]))) {
			// �����޳�
			if (cross(p1 - p0, p2 - p1).z <= 0)
				continue;

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

		if (renderState & WIREFRAME) {
			if (smoothLine) {
				rasterizeLine_antialiasing(p0.x, p0.y, p1.x, p1.y, vo[0]->color, vo[1]->color);
				rasterizeLine_antialiasing(p1.x, p1.y, p2.x, p2.y, vo[1]->color, vo[2]->color);
				rasterizeLine_antialiasing(p2.x, p2.y, p0.x, p0.y, vo[2]->color, vo[0]->color);
			} else {
				rasterizeLine((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, vo[0]->color, vo[1]->color);
				rasterizeLine((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, vo[1]->color, vo[2]->color);
				rasterizeLine((int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, vo[2]->color, vo[0]->color);
			}
			
		}
	}
}

void Pipeline::render(const Scene & scene) {
	// ���buffer
	if (clearState | CLEAR_COLOR)
		renderBuffer.fill(clearColor.toRGBInt());
	if (clearState | CLEAR_DEPTH)
		ZBuffer.fill(0.f);

	Matrix44 projectionViewTransform = scene.view * scene.projection;

	// ��ȾMesh
	for (size_t i = 0; i < scene.meshes.size(); i++) {
		renderMesh(scene.meshes[i], scene.modelMatrixs[i] * projectionViewTransform, scene.modelMatrixs[i] * scene.view);
	}

	// ��Ⱦ����
#pragma omp parallel for schedule(dynamic)
	for (int i = 0; (size_t)i < scene.lines.size(); i++) {
		renderLine(scene.lines[i], projectionViewTransform);
	}
}
