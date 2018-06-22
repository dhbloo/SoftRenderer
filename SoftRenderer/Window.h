#pragma once

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <unordered_map>
#include <windows.h>
#include <tchar.h>

#ifdef _MSC_VER
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "user32.lib")
#endif

class Window {
private:
	bool screen_running = false;
	int screen_w, screen_h;
	int screen_mx = 0, screen_my = 0, screen_mb = 0;
	int screen_keys[512];	            // ��ǰ���̰���״̬
	HWND screen_handle = nullptr;		// ������ HWND
	HDC screen_dc = nullptr;			// ���׵� HDC
	HBITMAP screen_hb = nullptr;		// DIB
	HBITMAP screen_ob = nullptr;		// �ϵ� BITMAP
	unsigned char *screen_fb = nullptr;	// frame buffer
	long screen_pitch = 0;
	int current_fps;

	static std::unordered_map<HWND, Window *> screen_refs;

	// win32 event handler
	friend LRESULT CALLBACK screen_events(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	void update_fps();

public:
	Window(int w, int h, const TCHAR *title);  // ��Ļ��ʼ��
	~Window();  // �ر���Ļ

	bool is_run() { return screen_running; }
	bool is_key(unsigned int code) { return code >= 512 ? false : screen_keys[code]; }
	int get_fps() { return current_fps; }
	void dispatch();  // ������Ϣ
	void update();    // ��ʾ FrameBuffer
	void destory();   // ����
	bool setTitle(const TCHAR *title);

	// ����FrameBufferĳ��Pixel��Int32��һ����ɫָ��(ÿ��pixel���ֽڶ���)
	int * operator()(unsigned int index = 0);
	int * operator()(int x, int y);
};

#endif