#include "Window.h"
#include "Pipeline.h"

using namespace std;

static int sceneNum = 2;
static float aspect;
static float translateZ = 1.2f;
static float rotateX = 0, rotateY = 0;

void createScene(Scene & scene, int index) {
	shared_ptr<Mesh> m = make_shared<Mesh>();
	switch (index) {
	case 0:
		m->vertices = { Vector3(0,0,0),Vector3(1,0,0),Vector3(0,1,0) };
		m->colors = { Colors::Red, Colors::Blue, Colors::Green };
		m->primitives = { Primitive{ 0,1,2,0,1,2 } };
		scene.addMesh(m);

		scene.setPerspective(90, aspect, 0.1f, 1000);
		//scene.setViewMatrix(Matrix44().setLookAt(Vector3(5 * sin(t), 5 * cos(t), -3), Vector3(0, 0, 0)));
		scene.setViewMatrix(Matrix44().translate(-0.5f, -0.5f, 0).rotate(0, 1, 0, rotateY).rotate(1, 0, 0, rotateX).translate(0, 0, translateZ));
		break;
	case 1:
		m->vertices = {
			Vector3(-0.5f, -0.5f,  0.5f),
			Vector3(0.5f, -0.5f,  0.5f),
			Vector3(0.5f,  0.5f,  0.5f),
			Vector3(-0.5f,  0.5f,  0.5f),
			Vector3(-0.5f, -0.5f, -0.5f),
			Vector3(-0.5f,  0.5f, -0.5f),
			Vector3(0.5f,  0.5f, -0.5f),
			Vector3(0.5f, -0.5f, -0.5f)
		};
		m->colors = { Colors::White };
		m->primitives = {
			Primitive{ 0,1,2 }, Primitive{ 0,2,3 },
			Primitive{ 4,5,6 }, Primitive{ 4,6,7 },
			Primitive{ 5,3,2 }, Primitive{ 5,2,6 },
			Primitive{ 4,7,1 }, Primitive{ 4,1,0 },
			Primitive{ 7,6,2 }, Primitive{ 7,2,1 },
			Primitive{ 4,0,3 }, Primitive{ 4,3,5 },
		};
		scene.addMesh(m);

		scene.setPerspective(70, aspect, 0.1f, 1000);
		scene.setViewMatrix(Matrix44().rotate(0, 1, 0, rotateY).rotate(1, 0, 0, rotateX).translate(0, 0, translateZ));
		break;
	default:
		break;
	}
}

int main() {
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	IntBuffer image(700, 500);
	Pipeline pipeline(image);

	Window window(image.getWidth(), image.getHeight(), _T("SoftRenderer"));
	aspect = image.aspect();

	bool kbhit = false;
	int toggle = 0;
	
	while (window.is_run()) {
		Scene scene;
		createScene(scene, toggle);
		pipeline.render(scene);

		memcpy(window(), image(), image.getSize() * sizeof(int));
		window.update();
		ostringstream s;
		s << "SoftRenderer Fps:" << window.get_fps();
		window.setTitle(_T(s.str().c_str()));
		if (window.is_key(VK_ESCAPE)) window.destory();
		else if (window.is_key(VK_LEFT)) rotateY -= 2.5f;
		else if (window.is_key(VK_RIGHT)) rotateY += 2.5f;
		else if (window.is_key(VK_DOWN)) rotateX -= 2.5f;
		else if (window.is_key(VK_UP)) rotateX += 2.5f;
		else if (window.is_key('W')) translateZ -= .05f;
		else if (window.is_key('S')) translateZ += .05f;
		else if (window.is_key(VK_SPACE)) {
			if (!kbhit) toggle = ++toggle % sceneNum;
			kbhit = true;
		} else kbhit = false;
		Sleep(1);
	}
}