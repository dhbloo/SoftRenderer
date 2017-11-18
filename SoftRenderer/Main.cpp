#include "Window.h"
#include "Pipeline.h"
#include "ShaderPrefab.h"

using namespace std;

const Pipeline::RenderState states[] = { Pipeline::Wireframe, Pipeline::Color, Pipeline::Texture,
Pipeline::ColoredTexture, Pipeline::Shading
};
const ShadeFunc shaders[] = {
	FragmentShader::depth(1.5f, 0),
	FragmentShader::normal(),
	FragmentShader::lambert_direction_light(Vector3(.5f, .5f, -1)),
	FragmentShader::phong_direction_light(Vector3(1, 1, -1), Colors::Black, Colors::White * .25f, Colors::White * .75f, 10.f),
	FragmentShader::blinn_phong_direction_light(Vector3(1, 1, -1), Colors::Black, Colors::White * .25f, Colors::White * .75f, 10.f)
};
const int shaderNum = 5;
const int sceneNum = 3;

static float aspect;
static float translateZ = 1.5f;
static float rotateX = 0, rotateY = 0;
static shared_ptr<IntBuffer> texture;
static ShadeFunc currentShader;

void createScene(Scene & scene, int index) {
	scene.clear();
	shared_ptr<Mesh> m = make_shared<Mesh>();
	m->texture = texture;
	m->shadeFunc = currentShader;
	switch (index) {
	case 0:
		m->vertices = { 
			{ Vector3(0,0,0), Colors::Red, TexCoord{ 0, 0 } },
			{ Vector3(1,0,0), Colors::Blue, TexCoord{ 1, 0 } },
			{ Vector3(0,1,0), Colors::Green, TexCoord{ 0, 1 } },
		};
		m->primitives = { Primitive{ 0,1,2, Vector3(0,0,1) } };
		scene.addMesh(m, Matrix44().translate(-.5f, -.5f, 0));
		break;
	case 1:
		m->vertices = {
			{ Vector3(-0.5f, -0.5f,  0.5f), Colors::Red, TexCoord {0, 0} },
			{ Vector3(0.5f, -0.5f,  0.5f), Colors::Green, TexCoord{ 1, 0 } },
			{ Vector3(0.5f,  0.5f,  0.5f), Colors::Blue, TexCoord{ 1, 1 } },
			{ Vector3(-0.5f,  0.5f,  0.5f), Colors::White, TexCoord{ 0, 1 } },
			{ Vector3(-0.5f, -0.5f, -0.5f), Colors::Blue, TexCoord{ 1, 0 } },
			{ Vector3(-0.5f,  0.5f, -0.5f), Colors::Red, TexCoord{ 1, 1 } },
			{ Vector3(0.5f,  0.5f, -0.5f), Colors::Green, TexCoord{ 0, 1 } },
			{ Vector3(0.5f, -0.5f, -0.5f), Colors::White, TexCoord{ 0, 0 } }
		};
		m->primitives = {
			Primitive{ 0,1,2, Vector3(0,0,1) }, Primitive{ 0,2,3, Vector3(0,0,1) },
			Primitive{ 4,5,6, Vector3(0,0,-1) }, Primitive{ 4,6,7, Vector3(0,0,-1) },
			Primitive{ 5,3,2, Vector3(0,1,0) }, Primitive{ 5,2,6, Vector3(0,1,0) },
			Primitive{ 4,7,1, Vector3(0,-1,0) }, Primitive{ 4,1,0, Vector3(0,-1,0) },
			Primitive{ 7,6,2, Vector3(1,0,0) }, Primitive{ 7,2,1, Vector3(1,0,0) },
			Primitive{ 4,0,3, Vector3(-1,0,0) }, Primitive{ 4,3,5, Vector3(-1,0,0) },
		};
		scene.addMesh(m);
		break;
	case 2:
		for (float i = 0; i < 360; i += 5) {
			scene.addLine(Line{
			    Vertex { Vector3(0,0,0), Colors::White },
			    Vertex { Vector3(0.7f * cos(i),0.7f * sin(i),1), Colors::Random() }
			});
		}
		break;
	}
}

int main() {
	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	IntBuffer image(700, 500);
	Pipeline pipeline(image);

	Scene scene;
	Window window(image.getWidth(), image.getHeight(), _T("SoftRenderer"));
	aspect = image.aspect();

	bool kbhit[3] = { false };
	int sceneI = 0, modeI = 0, shaderI = 0;
	texture = CreateTexture("C:\\Users\\dhb\\Pictures\\pika.jpg");
	currentShader = shaders[shaderI];

	createScene(scene, sceneI);
	
	while (window.is_run()) {
		//scene.setViewMatrix(Matrix44().setLookAt(Vector3(5 * sin(t), 5 * cos(t), -3), Vector3(0, 0, 0)));
		scene.setPerspective(70, aspect, 0.5f, 1000);
		scene.setViewMatrix(Matrix44().rotate(0, 1, 0, rotateY).rotate(1, 0, 0, rotateX).translate(0, 0, translateZ));

		pipeline.render(scene);

		memcpy(window(), image(), image.getSize() * sizeof(int));
		window.update();
		ostringstream s;
		s << "SoftRenderer(Space switch scene, Ctrl switch mode, Shift switch shader) Fps:" << window.get_fps();
		window.setTitle(_T(s.str().c_str()));
		if (window.is_key(VK_ESCAPE)) window.destory();
		if (window.is_key(VK_LEFT)) rotateY -= 2.5f;
		if (window.is_key(VK_RIGHT)) rotateY += 2.5f;
		if (window.is_key(VK_DOWN)) rotateX -= 2.5f;
		if (window.is_key(VK_UP)) rotateX += 2.5f;
		if (window.is_key('W')) translateZ -= .05f;
		if (window.is_key('S')) translateZ += .05f;

		if (window.is_key(VK_CONTROL)) {
			if (!kbhit[0]) {
				modeI = ++modeI % 5;
				pipeline.setRenderState(Pipeline::RenderState(states[modeI]));
			}
			kbhit[0] = true;
		} else kbhit[0] = false;
		if (window.is_key(VK_SPACE)) {
			if (!kbhit[1]) {
				sceneI = ++sceneI % sceneNum;
				createScene(scene, sceneI);
			}
			kbhit[1] = true;
		} else kbhit[1] = false;
		if (window.is_key(VK_SHIFT)) {
			if (!kbhit[2]) {
				shaderI = ++shaderI % shaderNum;
				currentShader = shaders[shaderI];
				createScene(scene, sceneI);
			}
			kbhit[2] = true;
		} else kbhit[2] = false;
		Sleep(1);
	}
}