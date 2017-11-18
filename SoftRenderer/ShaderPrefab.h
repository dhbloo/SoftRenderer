#pragma once

#ifndef _SHADERPREFAB_H_
#define _SHADERPREFAB_H_

#include "Primitives.h"

namespace FragmentShader {
	ShadeFunc depth(float zNear = 0.0f, float zFar = 1.0f);
	ShadeFunc normal();
	ShadeFunc lambert_direction_light(Vector3 lightDir, RGBColor lightColor = Colors::White);
	ShadeFunc phong(Vector3 lightDir, RGBColor ambient, RGBColor diffuse, RGBColor specular, float specularPower);
	ShadeFunc blinn_phong(Vector3 lightDir, RGBColor ambient, RGBColor diffuse, RGBColor specular, float specularPower);
}

#endif