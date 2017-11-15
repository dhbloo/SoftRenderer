#pragma once

#ifndef _COLOR_H_
#define _COLOR_H_

#include "Define.h"

class RGBColor {
public:
	float r, g, b;

	RGBColor() : r(0.f), g(0.f), b(0.f) {}
	RGBColor(float r, float g, float b) : r(r), g(g), b(b) {}
	~RGBColor() {}

	float operator[](UInt i) { return (&r)[i]; }

	bool operator ==(const RGBColor & color) {
		return color.r == r && color.g == g && color.b == b;
	}

	RGBColor operator - () const {
		return RGBColor(-r, -g, -b);
	}

	RGBColor operator + (const RGBColor & color) const {
		return RGBColor(r + color.r, g + color.g, b + color.b);
	}

	RGBColor operator - (const RGBColor & color) const {
		return RGBColor(r - color.r, g - color.g, b - color.b);
	}

	RGBColor operator * (const RGBColor & color) const {
		return RGBColor(r * color.r, g * color.g, b * color.b);
	}

	RGBColor operator / (const RGBColor & color) const {
		return RGBColor(r / color.r, g / color.g, b / color.b);
	}

	RGBColor operator * (float k) const {
		return RGBColor(r * k, g * k, b * k);
	}

	RGBColor operator / (float k) const {
		return RGBColor(r / k, g / k, b / k);
	}

	RGBColor & operator+= (const RGBColor & color) {
		r += color.r; g += color.g; b += color.b;
		return *this;
	}

	RGBColor & operator-= (const RGBColor & color) {
		r -= color.r; g -= color.g; b -= color.b;
		return *this;
	}

	RGBColor & operator*= (const RGBColor & color) {
		r *= color.r; g *= color.g; b *= color.b;
		return *this;
	}

	RGBColor & operator/= (const RGBColor & color) {
		r /= color.r; g /= color.g; b /= color.b;
		return *this;
	}

	RGBColor & operator*= (float k) {
		r *= k; g *= k; b *= k;
		return *this;
	}

	RGBColor & operator/= (float k) {
		r /= k; g /= k; b /= k;
		return *this;
	}

	inline RGBColor & clamp() {
		r = MathUtils::clamp(r);
		g = MathUtils::clamp(g);
		b = MathUtils::clamp(b);
		return *this;
	}

	inline RGBColor & clampNegative() {
		r = r < 0 ? 0 : r;
		g = g < 0 ? 0 : g;
		b = b < 0 ? 0 : b;
		return *this;
	}

	inline RGBColor & gammaCorrect(float gamma) {
		float oneOverGamma = 1 / gamma;
		r = pow(r, oneOverGamma);
		g = pow(g, oneOverGamma);
		b = pow(b, oneOverGamma);
		return *this;
	}

	inline RGBColor & gammaCorrect_inv(float oneOverGamma) {
		r = pow(r, oneOverGamma);
		g = pow(g, oneOverGamma);
		b = pow(b, oneOverGamma);
		return *this;
	}

	inline int toRGBInt() const {
		int ir = int(r * 255 + 0.5);
		int ig = int(g * 255 + 0.5);
		int ib = int(b * 255 + 0.5);
		ir = MathUtils::clamp(ir, 0, 255);
		ig = MathUtils::clamp(ig, 0, 255);
		ib = MathUtils::clamp(ib, 0, 255);
		return (ir << 16) | (ig << 8) | ib;
	}

	inline RGBColor & setRGBInt(int rgb) {
		r = ((rgb >> 16) & 0xFF) / 255.0f;
		g = ((rgb >> 8) & 0xFF) / 255.0f;
		b = (rgb & 0xFF) / 255.0f;
		return *this;
	}

	inline void toByte(UInt8 & rb, UInt8 & gb, UInt8 & bb) const {
		rb = UInt8(MathUtils::clamp(int(r * 255 + 0.5), 0, 255));
		gb = UInt8(MathUtils::clamp(int(g * 255 + 0.5), 0, 255));
		bb = UInt8(MathUtils::clamp(int(b * 255 + 0.5), 0, 255));
	}

	inline void toByte(UInt8 * rb, UInt8 * gb, UInt8 * bb) const {
		*rb = UInt8(MathUtils::clamp(r * 255 + 0.5, 0, 255));
		*gb = UInt8(MathUtils::clamp(g * 255 + 0.5, 0, 255));
		*bb = UInt8(MathUtils::clamp(b * 255 + 0.5, 0, 255));
	}

	friend std::ostream& operator << (std::ostream &s, const RGBColor &c) {
		return s << "color(" << c.r << "," << c.g << "," << c.b << ")";
	}
};

inline RGBColor ColorByte(UInt8 r, UInt8 g, UInt8 b) {
	const float s = 1.0f / 255;
	return RGBColor(r, g, b) *= s;
}

namespace Colors {
	const RGBColor Black = RGBColor(0.0, 0.0, 0.0);
	const RGBColor White = RGBColor(1.0, 1.0, 1.0);
	const RGBColor Red = RGBColor(1.0, 0.0, 0.0);
	const RGBColor Green = RGBColor(0.0, 1.0, 0.0);
	const RGBColor Blue = RGBColor(0.0, 0.0, 1.0);
	inline RGBColor Random() {
		return RGBColor((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX);
	}
}


class RGBAColor {
public:
	RGBColor rgb;
	float alpha;

	RGBAColor() : rgb(), alpha(1.f) {}
	RGBAColor(const RGBColor & color, float a = 1.f) : rgb(color), alpha(a) {}
	RGBAColor(float r, float g, float b, float a = 1.f) : rgb(r, g, b), alpha(a) {}
	~RGBAColor() {}

	RGBAColor operator + (const RGBAColor & c) const {
		return RGBAColor(rgb * alpha + c.rgb * c.alpha, 1 - (1 - alpha) * (1 - c.alpha));
	}

	RGBAColor & operator += (const RGBAColor & c) {
		rgb = rgb * alpha + c.rgb * c.alpha;
		alpha = 1 - (1 - alpha) * (1 - c.alpha);
		return *this;
	}

	RGBColor operator + (const RGBColor & c) const {
		return rgb * alpha + c * (1 - alpha);
	}

	RGBAColor operator * (float k) const {
		return RGBAColor(rgb * k, alpha);
	}
	RGBAColor & operator *= (float k) {
		rgb *= k;
		return *this;
	}
};

#endif