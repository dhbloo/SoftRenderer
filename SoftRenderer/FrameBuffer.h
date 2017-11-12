#pragma once

#ifndef _FRAMEBUFFER_H_
#define _FRAMEBUFFER_H_

#include "Color.h"

template <class T>
class FrameBuffer {
protected:
	int width, height;
	int size;
	T * buffer;

public:
	FrameBuffer(int width, int height) : width(width), height(height), size(width * height) {
		buffer = new T[width * height];
	}
	~FrameBuffer() {
		delete[] buffer;
	}

	inline int getWidth() const { return width; }
	inline int getHeight() const { return height; }
	inline int getSize() const { return size; }
	inline float aspect() const { return (float)getWidth() / getHeight(); }

	void set(int x, int y, const T & data) { buffer[y * width + x] = data; }
	void set(UInt index) { assert(index < size); buffer[index] = data; }
	void add(int x, int y, const T & data) { buffer[y * width + x] += data; }
	void add(UInt index) { assert(index < size); buffer[index] += data; }

	void clear(int x, int y) { buffer[y * width + x] = T(); }
	void fill(const T & data) { 
		for (int i = 0; i < size; i++) buffer[i] = data;
	}
	T get(int x, int y) const { return buffer[y * width + x]; }
	T get(UInt index) const { assert(index < size); return buffer[index]; }
	void get(T & ref, int x, int y) const { ref = buffer[y * width + x]; }
	void get(T & ref, UInt index) const { assert(index < size); ref = buffer[index]; }

	T * operator()(UInt index = 0) { return buffer + index; }
	T * operator()(int x, int y) { return buffer + (y * width + x); }

	// x, y ÔÚ[0, 1)·¶Î§ÄÚ
	T get(float x, float y) const {
		return get(int(MathUtils::clamp(x) * width), int(MathUtils::clamp(y) * height));
	}
	void get(T & ref, float x, float y) const {
		get(ref, int(MathUtils::clamp(x) * width), int(MathUtils::clamp(y) * height));
	}
};

typedef FrameBuffer<RGBColor> ColorBuffer;
typedef FrameBuffer<RGBAColor> RGBAColorBuffer;
typedef FrameBuffer<float> FloatBuffer;
typedef FrameBuffer<int> IntBuffer;

#endif