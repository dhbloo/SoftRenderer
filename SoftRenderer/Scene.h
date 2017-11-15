#ifndef _SCENE_H_
#define _SCENE_H_

#pragma once
#include "Matrix44.h"
#include "Primitives.h"

class Scene {
	friend class Pipeline;
private:
	vector<Line> lines;
	vector<shared_ptr<Mesh>> meshes;

	vector<Matrix44> modelMatrixs;

	// ����任
	Matrix44 currentModel;  // ��ǰ��ģ�;���
	Matrix44 view;          // ������任
	Matrix44 projection;    // ͶӰ�任
public:
	Scene() {}
	~Scene() {}

	void setViewMatrix(Matrix44 view) { this->view = view; }
	void setProjectionMatrix(Matrix44 projection) { this->projection = projection; }
	void setPerspective(float fov, float aspect, float zNear, float zFar) { projection.setPerspective(fov, aspect, zNear, zFar); }

	void translate(float x, float y, float z) { currentModel.translate(x, y, z); }
	void scale(float x, float y, float z) { currentModel.scale(x, y, z); }
	void rotate(float x, float y, float z, float theta) { currentModel.rotate(x, y, z, theta); }

	void clear() { 
		lines.clear();
		meshes.clear();
		modelMatrixs.clear();
	}
	void addLine(Line line) { lines.push_back(line); }
	void addMesh(shared_ptr<Mesh> mesh) {
		meshes.push_back(mesh);
		modelMatrixs.push_back(currentModel);
	}
	void addMesh(shared_ptr<Mesh> mesh, const Matrix44 & modelMatrix) {
		meshes.push_back(mesh);
		modelMatrixs.push_back(modelMatrix);
	}
};

#endif