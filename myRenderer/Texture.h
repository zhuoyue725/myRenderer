#pragma once
#include<iostream>
#include<vector>
#include "Vector.h"
#include"TgaTexture.h"
#include <fstream>
#pragma comment("winmm.lib")

enum TexType
{
	BMP,TGA,PNG,JPG
};
using namespace std;
class Texture
{
public:
	Texture() {};
	Texture(const char* path,TexType tp = BMP) 
	{
		type = tp;
		switch (tp)
		{
		case BMP:
			LoadTextureBMP(path);
			break;
		case TGA:
			LoadTextureTGA(path);
			break;
		case PNG:
			break;
		case JPG:
			break;
		default:
			break;
		}
	};
	~Texture() {};
	uint32_t width = 2000;
	uint32_t height = 2000;
	vector<vector<Vector3>> textureData;
	TexType type;
	TGAImage* tga = nullptr;

	wchar_t* ConvertToWideChar(const char* str) 
	{
		// 获取所需缓冲区大小
		int bufferSize = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);

		// 分配缓冲区
		wchar_t* wideStr = new wchar_t[bufferSize];

		// 多字节编码转宽字符编码
		MultiByteToWideChar(CP_UTF8, 0, str, -1, wideStr, bufferSize);

		return wideStr;
	}

	char* ConvertToChar(const wchar_t* wstr)
	{
		// 获取所需缓冲区大小
		int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);

		// 分配缓冲区
		char* str = new char[bufferSize];
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, bufferSize, NULL, NULL);
		return str;
	}
	void LoadTextureTGA(const char* path)
	{
		if(tga == nullptr)
			tga = new TGAImage();
		tga->read_tga_file(path);
		width = tga->get_width();
		height = tga->get_height();
	}
	void LoadTextureBMP(const char* path)
	{
		//char buffer[MAX_PATH];
		//GetCurrentDirectoryA(MAX_PATH, buffer);
		//std::cout << "当前目录路径为：" << buffer << std::endl;

		//FILE* file;
		//// 获取所需缓冲区大小
		//int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, NULL, 0, NULL, NULL);

		//// 分配缓冲区
		//char* path = new char[bufferSize];
		//WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path, bufferSize, NULL, NULL);
		//if (fopen_s(&file, path, "r") != 0)
		//{
		//	printf("不存在");
		//	return;
		//}
		//LPCWSTR wpath = L"./gezi.bmp";

		wchar_t* wpath = ConvertToWideChar(path);
		HBITMAP bitmap = (HBITMAP)LoadImage(NULL, wpath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

		if (bitmap == NULL)
		{
			cout << "加载贴图图片识别" << endl;
			return;
		}
		BITMAP bm;
		GetObject(bitmap, sizeof(bm), &bm);

		width = bm.bmWidth;
		height = bm.bmHeight;

		//cout << "width:" << width << endl;
		//cout << "height:"<< height << endl;

		HDC hdc = CreateCompatibleDC(NULL);
		SelectObject(hdc, bitmap);

		textureData.resize(width);
		for (int i = 0; i < width; i++)
		{
			for (int j = height-1; j >= 0; j--)
			{
				COLORREF color = GetPixel(hdc, i, j);
				int r = color % 256;
				int g = (color >> 8) % 256;
				int b = (color >> 16) % 256;
				//Color c((float)r / 256, (float)g / 256, (float)b / 256, 1);
				textureData[i].push_back(Vector3(r, g, b));
			}
		}
		DeleteObject(bitmap);	
	}
	Vector3 Sample(float u, float v)
	{
		u = Clamp(0, 1.0f, u);
		v = Clamp(0, 1.0f, v);
		int intu = width * u;
		int intv = height * v;
		return textureData[intu][intv];
	}
	Vector3 SimpleSample(Vector2& uv)
	{
		if (type == TGA)
		{
			return tga->getV3(uv.x * width, (1 - uv.y) * height);
		}
		else
		{
			uv.x = Clamp(0, 1.0f, uv.x);
			uv.y = Clamp(0, 1.0f, uv.y);
			int intu = width * uv.x;
			int intv = height * uv.y;
			return textureData[intu % width][intv % height];
		}

	}
	// 双线性过滤插值（当纹理分辨率比较低的时候）
	Vector3 SampleLinearFilter(Vector2& uv)
	{
		if (type == TGA)
		{
			return tga->getV3(uv.x * width, (1 - uv.y) * height);
		}
		else
		{
			// 可以不限制，如循环纹理
			uv.x = Clamp(0, 1.0f, uv.x);
			uv.y = Clamp(0, 1.0f, uv.y);
			float u_t = uv.x * width;
			float v_t = uv.y * height;

			int u_0 = (int)u_t; // 左
			int v_0 = (int)v_t; // 下

			float a = u_t - u_0;
			float b = v_t - v_0;

			float uv00 = (1 - a) * (1 - b);
			float uv10 = a * (1 - b);
			float uv01 = (1 - a) * b;
			float uv11 = a * b;
			Vector3 result = textureData[u_0 % width][v_0 % height] * uv00 +
				textureData[(u_0 + 1) % width][v_0 % height] * uv10 +
				textureData[u_0 % width][(v_0 + 1) % height] * uv01 +
				textureData[(u_0 + 1) % width][(v_0 + 1) % height] * uv11;
			return result;
		}
	}
	float Clamp(float min, float max, float value)
	{
		if (value > max)
			value = max;
		if (value < min)
			value = min;
		return value;
	}
};