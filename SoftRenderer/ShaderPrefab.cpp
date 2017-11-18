#include "ShaderPrefab.h"

ShadeFunc FragmentShader::depth(float zNear, float zFar) {
	float zLength = zFar - zNear;
	return [=](RGBColor & out, const Vector3 & pos, const RGBColor & color, const Vector3 & normal, const TexCoord & tex) -> bool {
		float f = (pos.z - zNear) / zLength;
		out = Colors::White * f;
		return true;
	};
}

ShadeFunc FragmentShader::normal() {
	return [=](RGBColor & out, const Vector3 & pos, const RGBColor & color, const Vector3 & normal, const TexCoord & tex) -> bool {
		out = RGBColor(normal.x, normal.y, -normal.z);
		return true;
	};
}

ShadeFunc FragmentShader::lambert_direction_light(Vector3 lightDir, RGBColor lightColor) {
	lightDir.normalize();
	return [=](RGBColor & out, const Vector3 & pos, const RGBColor & color, const Vector3 & normal, const TexCoord & tex) -> bool {
		out = lightColor * MathUtils::clamp(normal * lightDir);
		return true;
	};
}

ShadeFunc FragmentShader::phong(Vector3 lightDir, RGBColor ambient, RGBColor diffuse, RGBColor specular, float specularPower) {
	lightDir.normalize();
	return [=](RGBColor & out, const Vector3 & pos, const RGBColor & color, const Vector3 & normal, const TexCoord & tex) -> bool {
		Vector3 v(pos);
		v.x = 1.0f - 2 * v.x;
		v.y = 2 * v.y - 1.0f;
		v.z = -v.z;
		v.normalize();
		float diff = normal * lightDir;
		Vector3 reflectionVec = 2 * diff * normal - lightDir;
		reflectionVec.normalize();
		float spec = pow(MAX(0, reflectionVec * v), specularPower);

		out = ambient + diffuse * diff + specular * spec;
		return true;
	};
}

ShadeFunc FragmentShader::blinn_phong(Vector3 lightDir, RGBColor ambient, RGBColor diffuse, RGBColor specular, float specularPower) {
	lightDir.normalize();
	return [=](RGBColor & out, const Vector3 & pos, const RGBColor & color, const Vector3 & normal, const TexCoord & tex) -> bool {
		Vector3 v(pos);
		v.x = 1.0f - 2 * v.x;
		v.y = 2 * v.y - 1.0f;
		v.z = -v.z;
		v.normalize();
		float diff = normal * lightDir;
		Vector3 halfVec = lightDir + v;
		halfVec.normalize();
		float spec = pow(MAX(0, halfVec * normal), specularPower);

		out = ambient + diffuse * diff + specular * spec;
		return true;
	};
}
