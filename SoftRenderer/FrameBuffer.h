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
	void set(UInt index, const T & data) { assert(index < size); buffer[index] = data; }
	void add(int x, int y, const T & data) { buffer[y * width + x] += data; }
	void add(UInt index, const T & data) { assert(index < size); buffer[index] += data; }

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
		x = Math::fastFract(x);
		y = Math::fastFract(y);
		return get((int)(x * width) % width, (int)(y * height) % height);
	}
	void get(T & ref, float x, float y) const {
		x = Math::fastFract(x);
		y = Math::fastFract(y);
		get(ref, (int)(x * width) % width, (int)(y * height) % height);
	}
};

typedef FrameBuffer<RGBColor> ColorBuffer;
typedef FrameBuffer<float> FloatBuffer;
typedef FrameBuffer<int> IntBuffer;

shared_ptr<IntBuffer> CreateTexture(const char * filename);

#endif