#include "Window.h"
#include <iostream>
#include <cmath>
#include <vector>
#include <cmath>
#include <algorithm>


#include "Gobals.h"
#include "Vector.h"
#include "Matrix.h"
#include "Model.h"
#include "math.h"
#include "Input.h"
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Shadow.h"
#include "Skybox.h"

using namespace std;	


class Color
{
public:
	float r, g, b, a;
	Color(float cr, float cg, float cb, float ca);
	Color() {};
	~Color();

	Color operator + (const Color& right) const;
	Color operator+(const float c)
	{
		return Color(r + c, g + c, b + c, a);
	}
	Color operator - (const Color& right) const;
	Color operator * (const Color& right) const;
	Color operator * (float value) const;
	Color operator / (float value) const;
	static Color Lerp(const Color& left, const Color& right, float t);
	static Color white;
};


//void DrawTriangle(Window* w, Triangle<Vector2> Tri);
void DDA(Window* w, Vector2 P1, Vector2 P2);
void Update(Window* w);
void DoRender(Window* w);
void ShowFPS(Window* w);
void UpdateInput();
void DrawDepthBuffer(Window* w, float(*depthBuffer)[windowWidth]);

//void DrawUpTriangle(Window* w, Vector2 arr[], const float& ymax, const float& ymin);
//void DrawDownTriangle(Window* w, Vector2 arr[], const float& ymax, const float& ymin);

int main()
{
	Window* w = new Window(windowWidth, windowHeight, "Test");
	Update(w);
	system("pause");
	return 0;
}

void Update(Window* w)
{
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		UpdateInput();

		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			DoRender(w);
			ShowFPS(w);
		}
	}
}
void ShowFPS(Window* w)
{
	static float  fps = 0;
	static int    frameCount = 0;
	static float  currentTime = 0.0f;
	static float  lastTime = 0.0f;

	frameCount++;
	currentTime = timeGetTime() * 0.001f;

	if (currentTime - lastTime > 1.0f)
	{
		fps = (float)frameCount / (currentTime - lastTime);
		lastTime = currentTime;
		frameCount = 0;
	}

	wchar_t strBuffer[20];
	swprintf_s(strBuffer, 20, L"%0.3f", fps);
	TextOut(w->hdc, 0, 0, strBuffer, 6);
}

//Camera* camera = new Camera(Vector3(0, 5, 0), Vector3(0, 1, 0));

void CreateCube(Model &m)
{
	// 8个顶点
	Vector4* vertex = new Vector4[8];
	vertex[0] = Vector4(-2, 2, 2,1);
	vertex[1] = Vector4(2, 2,  2,1);
	vertex[2] = Vector4(2, -2, 2,1);
	vertex[3] = Vector4(-2, -2,2,1);
								
	vertex[4] = Vector4(-2, 2,-2,1);
	vertex[5] = Vector4(2, 2, -2,1);
	vertex[6] = Vector4(2, -2,-2,1);
	vertex[7] = Vector4(-2,-2,-2,1);
	// 12个三角形
	//Triangle<Vector3> *tris = new Triangle<Vector3>[12];
	//tris[0] = Triangle<Vector3>(vertex[0], vertex[3], vertex[1]);
	//tris[1] = Triangle<Vector3>(vertex[1], vertex[3], vertex[2]);
	//tris[2] = Triangle<Vector3>(vertex[1], vertex[6], vertex[5]);
	//tris[3] = Triangle<Vector3>(vertex[1], vertex[2], vertex[6]);

	//tris[4] = Triangle<Vector3>(vertex[4], vertex[5], vertex[7]);
	//tris[5] = Triangle<Vector3>(vertex[5], vertex[6], vertex[7]);
	//tris[6] = Triangle<Vector3>(vertex[0], vertex[4], vertex[7]);
	//tris[7] = Triangle<Vector3>(vertex[0], vertex[7], vertex[3]);

	//tris[8]  = Triangle<Vector3>(vertex[0], vertex[1], vertex[4]);
	//tris[9]  = Triangle<Vector3>(vertex[1], vertex[5], vertex[4]);
	//tris[10] = Triangle<Vector3>(vertex[3], vertex[7], vertex[2]);
	//tris[11] = Triangle<Vector3>(vertex[2], vertex[7], vertex[6]);

	//m.triangles = tris;

	// 面数组 12个,对应顶点索引
	Vector3 *face = new Vector3[12];
	face[0] = Vector3(0, 3, 1);
	face[1] = Vector3(1, 3, 2);
	face[2] = Vector3(1, 6, 5);
	face[3] = Vector3(1, 2, 6);
			  
	face[4] = Vector3(4, 5, 7);
	face[5] = Vector3(5, 6, 7);
	face[6] = Vector3(0, 4, 7);
	face[7] = Vector3(0, 7, 3);
			  
	face[8] = Vector3(0, 1, 4);
	face[9] = Vector3(1, 5, 4);
	face[10] =Vector3(3, 7, 2);
	face[11] =Vector3(2, 7, 6);

	// 法线数组 12个 每个面对应三个
	Vector3** normal = new Vector3 *[12];
	normal[0] = new Vector3[3]{ Vector3(0,0,1),Vector3(0,0,1),Vector3(0,0,1) };
	normal[1] = new Vector3[3]{ Vector3(0,0,1),Vector3(0,0,1),Vector3(0,0,1) };

	//normal[0] = new Vector3[3]{ Vector3(0,0,-1),Vector3(0,0,-1),Vector3(0,0,-1) };
	//normal[1] = new Vector3[3]{ Vector3(0,0,-1),Vector3(0,0,-1),Vector3(0,0,-1) };
	
	normal[2] = new Vector3[3]{ Vector3(1,0,0),Vector3(1,0,0),Vector3(1,0,0) };
	normal[3] = new Vector3[3]{ Vector3(1,0,0),Vector3(1,0,0),Vector3(1,0,0) };

	normal[4] = new Vector3[3]{ Vector3(0,0,-1),Vector3(0,0,-1),Vector3(0,0,-1) };
	normal[5] = new Vector3[3]{ Vector3(0,0,-1),Vector3(0,0,-1),Vector3(0,0,-1) };

	normal[6] = new Vector3[3]{ Vector3(-1,0,0),Vector3(-1,0,0),Vector3(-1,0,0) };
	normal[7] = new Vector3[3]{ Vector3(-1,0,0),Vector3(-1,0,0),Vector3(-1,0,0) };

	normal[8] = new Vector3[3]{ Vector3(0,1,0),Vector3(0,1,0),Vector3(0,1,0) };
	normal[9] = new Vector3[3]{ Vector3(0,1,0),Vector3(0,1,0),Vector3(0,1,0) };

	normal[10] = new Vector3[3]{ Vector3(0,-1,0),Vector3(0,-1,0),Vector3(0,-1,0) };
	normal[11] = new Vector3[3]{ Vector3(0,-1,0),Vector3(0,-1,0),Vector3(0,-1,0) };
	//normal[10] = new Vector3[3]{ Vector3(0,1,0),Vector3(0,1,0),Vector3(0,1,0) };
	//normal[11] = new Vector3[3]{ Vector3(0,1,0),Vector3(0,1,0),Vector3(0,1,0) };

	// 法线数组 6个 与面无关
	//Vector3* normal = new Vector3[6];
	//normal[0] = Vector3(0, 0, 1);
	//normal[1] = Vector3(0, 0, 1);
	//normal[2] = Vector3(1, 0, 0);
	//normal[3] = Vector3(1, 0, 0);
	//normal[4] = Vector3(0, 0, -1);
	//normal[5] = Vector3(0, 0, -1);


	// uv数组,不一定顶点和对应
	Vector2 *uvs = new Vector2[4];
	uvs[0] = Vector2(0.0f, 1.0f);
	uvs[1] = Vector2(1.0f, 1.0f);
	uvs[2] = Vector2(1.0f, 0.0f);
	uvs[3] = Vector2(0.0f, 0.0f);

	//面-uv数组
	Vector3* face_uv = new Vector3[12];
	face_uv[0] = Vector3(0, 3, 1);
	face_uv[1] = Vector3(1, 3, 2);
	face_uv[2] = Vector3(1, 3, 0);
	face_uv[3] = Vector3(1, 2, 3);
		
	face_uv[4] = Vector3(0, 1, 3);
	face_uv[5] = Vector3(1, 2, 3);
	face_uv[6] = Vector3(1, 0, 3);
	face_uv[7] = Vector3(1, 3, 2);
		
	face_uv[8] = Vector3(0, 1, 3);
	face_uv[9] = Vector3(1, 2, 3);
	face_uv[10] = Vector3(0, 3, 1);
	face_uv[11] = Vector3(1, 3, 2);

	//uvs[0] = Vector2(0.5f, 1.0f);
	//uvs[1] = Vector2(1.0f , 1.0f);
	//uvs[2] = Vector2(1.0f, 0.0f);
	//uvs[3] = Vector2(0.5f , 0.0f);
	//uvs[4] = Vector2(0.0f, 1.0f);
	//uvs[5] = Vector2(0.5f , 1.0f);
	//uvs[6] = Vector2(0.5f, 0.0f);
	//uvs[7] = Vector2(0.0f , 0.0f);

	//m.face = face;
	//m.vertex = vertex;
	//m.normal = normal;
	//m.uvs = uvs;
	//m.face_uv = face_uv;
	//m.triangles = new Triangle<Vector3>[12]; // 屏幕xy+z
	//m.veretexNum = 8;
	//m.triangleNum = 12;
}
void CreatePlane(Model& m)
{
	// 3个顶点
	Vector4* vertex = new Vector4[4];
	// 下三角形
	vertex[0] = Vector4(-2, 2, 0, 1);
	vertex[1] = Vector4(2, 2, 0, 1);
	vertex[2] = Vector4(-2, -2, 0, 1);
	vertex[3] = Vector4(2, -2, 0, 1);

	// 面数组 1个
	Vector3* face = new Vector3[2];
	face[0] = Vector3(0, 1, 2);
	face[1] = Vector3(1, 3, 2);

	// 法线数组 1个
	Vector3** normal = new Vector3 * [2];
	normal[0] = new Vector3[3]{ Vector3(0,0,-1),Vector3(0,0,-1),Vector3(0,0,-1) };
	normal[1] = new Vector3[3]{ Vector3(0,0,-1),Vector3(0,0,-1),Vector3(0,0,-1) };

	// uv数组
	Vector2* uvs = new Vector2[4];
	uvs[0] = Vector2(0.0f, 0.0f);
	uvs[1] = Vector2(1.0f, 0.0f);
	uvs[2] = Vector2(0.0f, 1.0f);
	uvs[3] = Vector2(1.0f, 1.0f);

	// face uv索引数组
	Vector3* face_uv = new Vector3[2];
	face_uv[0] = Vector3(0, 1, 2);
	face_uv[1] = Vector3(1, 3, 2);

	//m.face = face;
	//m.vertex = vertex;
	//m.face_uv = face_uv;
	//m.triangles = new Triangle<Vector3>[2]; // 屏幕空间2D像素坐标 + z深度值
	//m.veretexNum = 4;
	//m.triangleNum = 2;
	//m.normal = normal;
	//m.uvs = uvs;
}
// 创建三角形，非索引类型
void CreateTriangle(Model& m)
{
	// 3个顶点
	Vector4* vertex = new Vector4[3];
	// 下三角形
	//vertex[0] = Vector4(-2, 2, 0, 1);
	//vertex[1] = Vector4(2, 2, 0, 1);
	//vertex[2] = Vector4(0, -2, 0, 1);
	// 上三角形
	vertex[0] = Vector4(0, 2, 0, 1);
	vertex[1] = Vector4(-2, -2, 0, 1);
	vertex[2] = Vector4(2, -2, 0, 1);
	//// 世界空间，测试
	//Vector4* vertex = new Vector4[3];
	//vertex[0] = Vector4(0, 7, 20, 1);
	//vertex[1] = Vector4(2, 3, 20, 1);
	//vertex[2] = Vector4(-2, 3, 20, 1);

	// 面数组 1个
	Vector3* face = new Vector3[1];
	face[0] = Vector3(0, 2, 1);

	// 法线数组 1个
	Vector3 **normal = new Vector3*[1];
	normal[0] = new Vector3[3]{Vector3(0,0,-1),Vector3(0,0,-1),Vector3(0,0,-1)};

	// uv数组
	Vector2* uvs = new Vector2[3];
	uvs[0] = Vector2(0.0f, 0.0f);
	uvs[1] = Vector2(0.5f, 0.5f);
	uvs[2] = Vector2(1.0f, 0.0f);

	//面-uv数组
	Vector3* face_uv = new Vector3[1];
	face_uv[0] = Vector3(0, 1, 2);

	//m.face = face;
	//m.vertex = vertex;
	//m.face_uv = face_uv;
	//m.triangles = new Triangle<Vector3>[1]; // 屏幕空间2D像素坐标 + z深度值
	//m.veretexNum = 3;
	//m.triangleNum = 1;
	//m.normal = normal;
	//m.uvs = uvs;
}


class IShader
{
public:
	IShader() {};
	~IShader() {};

	virtual Vector3 ShaderModel()
	{

	}
};

class PhongShader : IShader
{
public:
	PhongShader(): IShader() {};
	~PhongShader() {  };
	float gloss;
	float specular;
	Vector3 ShaderModel()
	{

	}
};

Model box;
Model plane("./models/floor.obj");
//Model obj("./models/african_head/african_head.obj");
Model obj("./models/cube/cube.obj");//("./models/floor.obj");
//Model obj("./models/skybox/skybox.obj");
//Model obj("./models/floor.obj");
TransparentShader objShader; //读取obj文件的模型
Shader boxShader;
Shader planeShader;
SkyboxShader skyboxShader;
//ShadowShader* boxShadow = new ShadowShader();//两个物体，两个阴影shader分开pass
//ShadowShader* planeShadow = new ShadowShader();

DirectionLight* dl = new DirectionLight(Vector3(1, -1, 1), Vector3(250, 250, 250));
//DirectionLight* dl = new DirectionLight(Vector3(-1, -1, 1), Vector3(200, 250, 200));
Camera* camera = new Camera(Vector3(0, 0, -camRadius), Vector3(0, 1, 0), Vector3(0, 0, 0),false/*,Vector3(0,0,1)*/); // Vector3(0, 5, 20)
//Camera* camera = new Camera(Vector3(4.33f, 0, -2.5f), Vector3(0, 1, 0), Vector3(0, 0, 0)); // Vector3(0, 5, 20)
//Camera* lightCamera = new Camera(-dl->dir * 20.0f, Vector3(0, 1, 0), Vector3(0, 0, 0), true); // 光源位置为相机位置
//Camera* lightCamera = new Camera(Vector3(0, 20, 0), Vector3(0, 0, 1), Vector3(0, 0, 0),true); // 光源位置为相机位置
void UpdateInput()
{
	static float alpha = 0;// 绕着Y轴逆时针
	static float beta = 0; // 绕着X轴顺时针
	if (objShader.camera == nullptr)
		return;
	static Vector3 t = camera->pos;
	static Vector3 dir = camera->dir;
	static Vector3 tag = camera->target;
	static float tagAlpha = 0;// tag绕着Y轴逆时针
	static float tagBeta = 0; // tag绕着X轴顺时针
	static float dirAlpha = 0;// dir绕着Y轴逆时针
	static float dirBeta = 0; // dir绕着X轴顺时针

	// 摄像机WSAD环绕目标点移动
	if (IS_KEY_DOWN('A'))
	{
		alpha += 2.0f; // 影响x和z
		t.x = camRadius * cos(beta * PIdivide180) * sin(alpha * PIdivide180);
		t.z = -camRadius * cos(beta * PIdivide180) * cos(alpha * PIdivide180);
	}
	if (IS_KEY_DOWN('D'))
	{
		alpha -= 2.0f;
		t.x = camRadius * cos(beta * PIdivide180) * sin(alpha * PIdivide180);
		t.z = -camRadius * cos(beta * PIdivide180) * cos(alpha * PIdivide180);
	}
	if (IS_KEY_DOWN('W'))
	{
		beta += 2.0f;//只影响Y坐标
		t.y = camRadius * sin(beta * PIdivide180);
		t.x = camRadius * cos(beta * PIdivide180) * sin(alpha * PIdivide180);
		t.z = -camRadius * cos(beta * PIdivide180) * cos(alpha * PIdivide180);
	}
	if (IS_KEY_DOWN('S'))
	{
		beta -= 2.0f;
		t.y = camRadius * sin(beta * PIdivide180);
		t.x = camRadius * cos(beta * PIdivide180) * sin(alpha * PIdivide180);
		t.z = -camRadius * cos(beta * PIdivide180) * cos(alpha * PIdivide180);
	}
	//cout << t;
	//objShader.camera->SetPosition(t);

	// 摄像机前后左右移动WSAD
/*	if (IS_KEY_DOWN('A'))
	{
		t.x -= 0.1f;
	}
	else if (IS_KEY_DOWN('D'))
	{
		t.x += 0.1f;
	}
	else if (IS_KEY_DOWN('W'))
	{
		t.z += 0.1f;
	}
	else if (IS_KEY_DOWN('S'))
	{
		t.z -= 0.1f;
	}
	else */
	if (IS_KEY_DOWN(VK_LEFT))
	{
		// 上箭头被按下
		dirAlpha -= 2.0f; // 影响x和z
		dir.x = camRadius * cos(dirBeta * PIdivide180) * sin(dirAlpha * PIdivide180);
		dir.z = camRadius * cos(dirBeta * PIdivide180) * cos(dirAlpha * PIdivide180);
	}
	else if (IS_KEY_DOWN(VK_RIGHT))
	{
		// 下箭头被按下
		dirAlpha += 2.0f; // 影响x和z
		dir.x = camRadius * cos(dirBeta * PIdivide180) * sin(dirAlpha * PIdivide180);
		dir.z = camRadius * cos(dirBeta * PIdivide180) * cos(dirAlpha * PIdivide180);
	}
	else if (IS_KEY_DOWN(VK_UP))
	{
		// 左箭头被按下
		dirBeta -= 2.0f;//只影响Y坐标
		dir.y = camRadius * sin(dirBeta * PIdivide180);
		dir.x = camRadius * cos(dirBeta * PIdivide180) * sin(dirAlpha * PIdivide180);
		dir.z = camRadius * cos(dirBeta * PIdivide180) * cos(dirAlpha * PIdivide180);
	}
	else if (IS_KEY_DOWN(VK_DOWN))
	{
		// 右箭头被按下
		dirBeta += 2.0f;
		dir.y = camRadius * sin(dirBeta * PIdivide180);
		dir.x = camRadius * cos(dirBeta * PIdivide180) * sin(dirAlpha * PIdivide180);
		dir.z = camRadius * cos(dirBeta * PIdivide180) * cos(dirAlpha * PIdivide180);
	}
	objShader.camera->SetPosition(t);
	//objShader.camera->SetDirection(dir);
	//if (IS_KEY_DOWN(VK_LEFT))
	//{
	//	// 上箭头被按下
	//	tagAlpha -= 2.0f; // 影响x和z
	//	tag.x = camRadius * cos(tagBeta * PIdivide180) * sin(tagAlpha * PIdivide180);
	//	tag.z = -camRadius * cos(tagBeta * PIdivide180) * cos(tagAlpha * PIdivide180);
	//}
	//else if (IS_KEY_DOWN(VK_RIGHT))
	//{
	//	// 下箭头被按下
	//	tagAlpha += 2.0f; // 影响x和z
	//	tag.x = camRadius * cos(tagBeta * PIdivide180) * sin(tagAlpha * PIdivide180);
	//	tag.z = -camRadius * cos(tagBeta * PIdivide180) * cos(tagAlpha * PIdivide180);
	//}
	//else if (IS_KEY_DOWN(VK_UP))
	//{
	//	// 左箭头被按下
	//	tagBeta += 2.0f;//只影响Y坐标
	//	tag.y = camRadius * sin(tagBeta * PIdivide180);
	//	tag.x = camRadius * cos(tagBeta * PIdivide180) * sin(tagAlpha * PIdivide180);
	//	tag.z = -camRadius * cos(tagBeta * PIdivide180) * cos(tagAlpha * PIdivide180);
	//}
	//else if (IS_KEY_DOWN(VK_DOWN))
	//{
	//	// 右箭头被按下
	//	tagBeta -= 2.0f;
	//	tag.y = camRadius * sin(tagBeta * PIdivide180);
	//	tag.x = camRadius * cos(tagBeta * PIdivide180) * sin(tagAlpha * PIdivide180);
	//	tag.z = -camRadius * cos(tagBeta * PIdivide180) * cos(tagAlpha * PIdivide180);
	//}
	////cout << tag;
	//objShader.camera->SetTagPosition(tag);
}

void DoRender(Window* w)
{
	//Triangle<Vector2> ScrTri(Vector2(375, 250), Vector2(325, 400), Vector2(400, 375));
	//Triangle<Vector4> NDCTri(Vector4(0.5,0,1,1), Vector4(0.3,0.6,1,1), Vector4(0.6,0.5,1,1)); // [-1,1]
	//Triangle<Vector4> viewTri(Vector4(0,150,20,1), Vector4(100,350,20,1), Vector4(200,150,20,1)); // [??,??]
	//Triangle<Vector4> NDCTri = OrthoToClipSpace(OrthoTri); // 应该还有一步是裁剪的
	//Triangle<Vector4> worldTri(Vector4(0, 100, 20, 1), Vector4(100, 300, 20, 1), Vector4(200, 100, 20, 1)); // [??,??]
	
	if (plane.modelMat == nullptr)
	{
		plane.SetModel(Vector3(0, -1, 0), Vector3(0, 0, 0), Vector3(1.5f, 0, 1.5f)); // 
		//CreatePlane(plane);
	}
	//if (box.veretexNum == 0)
	//{
	//	CreateCube(box);
	//	box.SetModel(Vector3(0, 1, 0), Vector3(0, 0, 0), Vector3(0.5f, 0.5f, 0.5f));
	//}

	if (obj.modelMat == nullptr)
	{
		//obj.SetModel(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(1.0f, 1.0f, 1.0f));
		//Vector3 a = Vector3(1.0f, 1.0f, 1.0f) * 10.0f;
		//cout << a;
		//obj.SetModel(Vector3(0, 0, 0), Vector3(0, 0, 0), Vector3(10,10,10)); // 天空盒大小
		obj.SetModel(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0, 0, 0), Vector3(0.5f, 0.5f, 0.5f));
	}
	// TODO : 输入网格m，输出一组2D屏幕三角形坐标数组，然后绘制

	// 设置shadow shader的光源相机，光源or物体移动时候才需要更新，先只渲染一帧阴影贴图
	//if (boxShadow->camera == nullptr)
	//{
	//	boxShadow->ClearFragment();// 清理阴影深度缓存
	//	boxShadow->SetParameter(lightCamera, box, dl, L"./gezi.bmp", Vector3(100, 100, 100), Vector3(100, 150, 100), 0.8f); // 不需要每帧都设置
	//	boxShadow->Pass();
	//}
	//if (planeShadow->camera == nullptr)
	//{
	//	planeShadow->SetParameter(lightCamera, plane, dl, L"./gezi.bmp", Vector3(100, 100, 100), Vector3(100, 150, 100), 0.8f); // 不需要每帧都设置
	//	planeShadow->Pass();
	//}
	//if (boxShader.camera == nullptr)
	//{			 
	//	// 设置shader的相机、对应模型、模型的高光、漫反射颜色信息、光泽度
	//	boxShader.SetParameter(camera, box, dl, L"./gezi.bmp", Vector3(100, 100, 100), Vector3(100, 150, 100), 0.8f,lightCamera->VP); // 不需要每帧都设置
	//}
	if (planeShader.camera == nullptr)
	{
		planeShader.SetParameter(camera, plane, dl, "./face.bmp", Vector3(100, 100, 100), Vector3(100, 150, 200), 0.8f,false,nullptr); // 不需要每帧都设置
	}
	if (objShader.camera == nullptr)
	{
		// L"./models/african_head/african_head_diffuse.bmp"
		objShader.SetParameter(camera, obj, dl, "./models/cube/wall.bmp", Vector3(200, 100, 100), Vector3(100, 150, 100), 0.8f,false, nullptr,nullptr,true); // 不需要每帧都设置,"./geziNormal.bmp""./models/african_head/african_head_nm_tangent.tga"
		//objShader.SetParameter(camera, obj, dl, "./models/african_head/african_head_diffuse.tga", Vector3(50, 50, 50), Vector3(200, 150, 200), 0.8f, nullptr,"./models/african_head/african_head_nm_tangent.tga"); // 不需要每帧都设置
	}
	//if (skyboxShader.camera == nullptr)
	//{
	//	skyboxShader.SetParameter(camera, obj, dl, "./models/skybox/skybox.bmp", Vector3(200, 100, 100), Vector3(200, 150, 200), 0.8f,false, nullptr); // 不需要每帧都设置,"./geziNormal.bmp""./models/african_head/african_head_nm_tangent.tga"
	//}
	Shader::ClearBuffer(camera);
	//Shader::ClearBuffer(lightCamera); // 清理深度缓存
	//boxShader.ClearFragment();
	planeShader.ClearFragment();
	objShader.ClearFragment();
	//skyboxShader.ClearFragment();
	//boxShader.Pass();
	planeShader.Pass();
	objShader.Pass();

	//skyboxShader.Pass();
	// f 4/4/1 3/3/1 1/1/1  
	// f 3/3/1 2/2/1 1/1/1


	//if (shader.camera == nullptr)
	//{
	//	shader.camera = camera;
	//	shader.camera->SetModel(m.pos,m.rotation,m.scale); // 旋转顺序：Y->X->Z ，有点奇怪
	//	//camera->SetPosition(0.7f * camRadius, 0, 0.7f * camRadius);
	//	//shader.c_d = Vector3(100, 100, 150); // 漫反射
	//	const wchar_t* path = L"./gezi.bmp";
	//	shader.tex = new Texture(path);
	//	shader.c_s = Vector3(100, 100, 100);
	//	shader.gloss = 0.8f;
	//	//PointLight* pl = new PointLight(Vector3(0, 5, 0), Vector3(100, 250, 200));
	//	Light* light = dl;
	//	shader.light = light;
	//}

	//for (int i = 0; i < shader.tex->width; i++)
	//{
	//	for (int j = 0; j < shader.tex->height; j++)
	//	{
	//		if (shader.tex->textureData[i][j].x > 200)
	//		{
	//			printf("%d %d %f %f %f\n",i,j, shader.tex->textureData[i][j].x, shader.tex->textureData[i][j].y, shader.tex->textureData[i][j].z);
	//		}
	//	}
	//}

	//shader.Pass(m);

	// 直接光栅化
	//m.vertex = Triangle<Vector4>(Vector2(100, 100), Vector2(300, 100), Vector2(200, 300));
	//m.triangles[0] = Triangle<Vector2>(Vector2(100, 100), Vector2(300, 100), Vector2(200, 300));


	// 裁剪空间，测试 [-1，1]
	//if (shader.clipPosition == nullptr)
	//{
	//	shader.clipPosition = new Vector4[3];
	//	//Vector4* vertex = new Vector4[3];
	//	shader.clipPosition[0] = Vector4(0, 0.5f, 0, 1);
	//	shader.clipPosition[1] = Vector4(0.5, -0.5f, 0, 1);
	//	shader.clipPosition[2] = Vector4(-0.5f, -0.5f, 0, 1);
	//}

	/*shader.VertexShader(m,8);
	shader.ClipToNDCToScreen(m, 12);
	shader.Rasterization(m.triangles, 12);
	shader.zTest(m.triangles,12);*/

	//DrawColorBuffer(w, cb); // 直接使用颜色缓存来绘制，而不是三角形
	//DrawColorBuffer(w, shader.fxaaBuffer); // 直接使用颜色缓存来绘制，而不是三角形
	DrawColorBuffer(w, colorBuffer); // 直接使用颜色缓存来绘制，而不是三角形
	//DrawDepthBuffer(w,depthBuffer); // 直接使用颜色缓存来绘制，而不是三角形
	camera->view_update = false;
	//DrawColorBuffer(w);
	/*原有：输入一个三角形，输出这个三角形的屏幕映射并且填充*/
	/*
	Triangle<Vector4> modelTri(Vector4(-3,-3,0,1), Vector4(0,3,0,1), Vector4(3,-3,0,1)); // [??,??]
	
	Triangle<Vector4> worldTri = ModelToWorldSpace(modelTri,Vector3(0,50,30),Vector3(1,1,1),Vector3(0,30,0));
	Triangle<Vector4> viewTri = WorldToViewSpace(worldTri);
	Triangle<Vector4> ClipTri = PerspectToClipSpace(viewTri); // 应该还有一步是裁剪的
	//Triangle<Vector4> ClipTri = OrthoToClipSpace(viewTri); // 应该还有一步是裁剪的
	Triangle<Vector2> ScrTri = ClipToScreenSpace(ClipTri);

	DrawTriangle(w, ScrTri);
	*/
	//SET_WHITE_PIXEL(w, ScrTri.x.x, ScrTri.x.y);
	//SET_WHITE_PIXEL(w, ScrTri.y.x, ScrTri.y.y);
	//SET_WHITE_PIXEL(w, ScrTri.z.x, ScrTri.z.y);
	BitBlt(w->hdc, 0, 0, windowWidth, windowHeight, w->screenHDC, 0, 0, SRCCOPY); //比特位转移，w->screenHDC是内存 DC，存储刚刚这一帧的颜色
}
void DrawColorBuffer(Window* w, Vector3 (*colorBuffer)[windowWidth])
{
	static COLORREF cachedColors[windowWidth * windowHeight] = { 0 }; // 颜色缓存
	int index = 0;
	//auto color = RGB(0,0,0);
	for (int x = 0 ; x < windowWidth ; x++)
	{
		for (int y = 0; y < windowHeight; y++)
		{
			COLORREF color = RGB(colorBuffer[x][y].x, colorBuffer[x][y].y, colorBuffer[x][y].z);
			if (cachedColors[index] != color) // 发生变化
			{
				cachedColors[index] = color;
				SetPixel((w)->screenHDC, x, y, color);
			}
			++index;
		}
	}
}

void DrawDepthBuffer(Window* w, float(*depthBuffer)[windowWidth])
{
	static COLORREF cachedDepth[windowWidth * windowHeight] = { 0 }; // 颜色缓存
	int index = 0;
	//auto color = RGB(0,0,0);
	for (int x = 0; x < windowWidth; x++)
	{
		for (int y = 0; y < windowHeight; y++)
		{
			if (depthBuffer[x][y] > -10.0f)
			{
				//printf("%d %d %.2f  #", x,y,depthBuffer[x][y]);
				COLORREF color = RGB(200, 0, 0);
				//COLORREF color = RGB(depthBuffer[x][y] * 255, depthBuffer[x][y] * 255, depthBuffer[x][y] * 255);
				if (cachedDepth[index] != color) // 发生变化
				{
					cachedDepth[index] = color;
					SetPixel((w)->screenHDC, x, y, color);
				}
			}
			++index;
		}
	}
}

struct VertexData
{
public:
	Vector3 screenXyz; // 屏幕+Z
	float viewPosZ; // view空间深度Z
	Vector2 uv; // uv坐标
	Vector3 worldPos; // 世界pos
	VertexData(Vector3 &s,Vector2 &uv,Vector3 &w,float &vz)
	{
		screenXyz = s;
		viewPosZ = vz;
		this->uv = uv;
		worldPos = w;
	}
	VertexData()
	{

	}
};

bool vertexCmp(const VertexData& v1, const VertexData& v2)
{
	if (!equals(v1.screenXyz.y, v2.screenXyz.y))
	{
		return v1.screenXyz.y < v2.screenXyz.y;// 屏幕空间中的上是小
	}
	else
	{
		return v1.screenXyz.x < v2.screenXyz.x;
	}
}

