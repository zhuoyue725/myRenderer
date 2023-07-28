#pragma once

#define SET_WHITE_PIXEL(w,x,y) SetPixel((w)->screenHDC, x, y, RGB(255, 255, 255))
#define M_PI 3.1415926535
#define IS_KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define IS_KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)
//#define windowHeight 500
//#define windowWidth 500

extern constexpr float PIdivide180 = M_PI / 180.0f;
extern constexpr float oneDivide255 = 1.0f / 255.0f;
extern constexpr float twoDivide255 = 2.0f / 255.0f;
extern constexpr float threeDivideSqrt3 = 1.73205f / 3.0f;
extern constexpr float camRadius = 8;
extern constexpr float camTagRadius = 5;
extern constexpr uint32_t windowHeight = 500;
extern constexpr uint32_t windowWidth = 500;