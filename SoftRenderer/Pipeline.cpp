#include "Pipeline.h"

void Pipeline::drawPixel(int x, int y, RGBAColor color) {
	if (x >= 0 && x < renderBuffer.getWidth() && y >= 0 && y < renderBuffer.getHeight())
		renderBuffer.set(x, y, (color + RGBColor().setRGBInt(renderBuffer.get(x, y))).toRGBInt());
}

void Pipeline::rasterizeLine(int x0, int y0, int x1, int y1, RGBAColor c) {
	int x, y, rem = 0;
	if (x0 == x1 && y0 == y1) {
		drawPixel(x0, y0, c);
	} else if (x0 == x1) {
		if (y1 < y0) swap(y0, y1);
		for (y = y0; y <= y1; y++) drawPixel(x0, y, c);
	} else if (y0 == y1) {
		if (x1 < x0) swap(x0, x1);
		for (x = x0; x <= x1; x++) drawPixel(x, y0, c);
	} else {
		int dx = (x0 < x1) ? x1 - x0 : x0 - x1;
		int dy = (y0 < y1) ? y1 - y0 : y0 - y1;
		if (dx >= dy) {
			if (x1 < x0) x = x0, y = y0, x0 = x1, y0 = y1, x1 = x, y1 = y;
			for (x = x0, y = y0; x <= x1; x++) {
				drawPixel(x, y, c);
				rem += dy;
				if (rem >= dx) {
					rem -= dx;
					y += (y1 >= y0) ? 1 : -1;
					drawPixel(x, y, c);
				}
			}
			drawPixel(x1, y1, c);
		} else {
			if (y1 < y0) x = x0, y = y0, x0 = x1, y0 = y1, x1 = x, y1 = y;
			for (x = x0, y = y0; y <= y1; y++) {
				drawPixel(x, y, c);
				rem += dx;
				if (rem >= dy) {
					rem -= dy;
					x += (x1 >= x0) ? 1 : -1;
					drawPixel(x, y, c);
				}
			}
			drawPixel(x1, y1, c);
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

void Pipeline::renderMesh(shared_ptr<Mesh> mesh, const Matrix44 & transform) {
	vector<Vector3> & vertices = mesh->vertices;
	vector<RGBAColor> & colors = mesh->colors;

	Vector4 c0, c1, c2;
	Vector3 p0, p1, p2;
	for each (auto p in mesh->primitives) {
		// 按照 Transform 变化
		transform.apply(Vector4(vertices[p.vectorIndex[0]]), c0);
		transform.apply(Vector4(vertices[p.vectorIndex[1]]), c1);
		transform.apply(Vector4(vertices[p.vectorIndex[2]]), c2);

		// 裁剪测试：所有顶点都出去了就不渲染
		// 可以完善为具体判断几个点在 cvv内以及同cvv相交平面的坐标比例
		// 进行进一步精细裁剪，将一个分解为几个完全处在 cvv内的三角形
		if (transformCheckCVV(c0) && transformCheckCVV(c1) && transformCheckCVV(c2)) continue;

		// 归一化
		transformHomogenize(c0, p0);
		transformHomogenize(c1, p1);
		transformHomogenize(c2, p2);

		if (renderState & Wireframe) {
			rasterizeLine((int)p0.x, (int)p0.y, (int)p1.x, (int)p1.y, colors[p.colorIndex[0]]);
			rasterizeLine((int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y, colors[p.colorIndex[1]]);
			rasterizeLine((int)p2.x, (int)p2.y, (int)p0.x, (int)p0.y, colors[p.colorIndex[2]]);
		}
		
	}
}

void Pipeline::render(Scene & scene) {
	renderBuffer.fill(clearColor.toRGBInt());

	Matrix44 projectionViewTransform = scene.view * scene.projection;

	for (size_t i = 0; i < scene.meshes.size(); i++) {
		renderMesh(scene.meshes[i], scene.modelMatrixs[i] * projectionViewTransform);
	}

	for each (auto line in scene.lines) {
		Vector4 c0, c1;
		Vector3 p0, p1;
		projectionViewTransform.apply(Vector4(line->vertices[0]), c0);
		projectionViewTransform.apply(Vector4(line->vertices[1]), c1);


		if (transformCheckCVV(c0) && transformCheckCVV(c1)) continue;

		transformHomogenize(c0, p0);
		transformHomogenize(c1, p1);

		rasterizeLine(line->vertices[0][0], line->vertices[0][1], line->vertices[1][0], line->vertices[1][1], line->color);
	}
}
