#pragma once
#include "Gobals.h"
#include "Vector.h"
#include "Camera.h"
#include "Light.h"

class ShadowShader:public Shader
{
public:
	DirectionLight* dl;
	void CalculateShadowMap()// 计算阴影贴图
	{
		// 光源位置作为相机位置
		// 对场景中所有物体进行正交投影 MVP，得到clip
		// Clip to NDC

		// 对片元逐个进行计算深度
		// 深度测试，得到深度图
	}
	virtual void ClearFragment()
	{
		fragments.clear();
		std::fill(reinterpret_cast<float*>(depthBuffer), reinterpret_cast<float*>(depthBuffer) + windowHeight * windowWidth, -100.0f);
		//memset(depthBuffer, -100, windowHeight * windowWidth * sizeof(float));
	}
	//virtual void Pass()
	//{
	//	printf("XX");
	//}

	// 光栅化阶段,GPU,计算每个片元的深度即可，不需要片元着色器，也不需要除了屏幕坐标、深度的插值
	virtual void FragmentShader()
	{
		//printf("x");
	}
	// 变换到光源的Clip空间，因为w=1，所以不需要也行
	Matrix3x3 GetLightVPMat()
	{
		return (*camera->viewMat * *camera->projectMat).toMatrix3x3();
	}
	virtual void zTest()
	{
		// 逐个片元测试，这里只进行深度测试，但是先不测试，有就直接上颜色
		int fragNum = fragments.size();
		Vector3 lightPos = light->pos;
		//float distance = 0;
		//auto zBuffer = camera->zBuffer;
		for (int j = 0; j < fragNum; ++j)
		{
			int x = fragments[j].m_x, y = fragments[j].m_y;
			//if (fragments[j].color.x > this->colorBuffer[x][y].x)
			//{
			//	this->colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // 写入颜色缓存
			//}

			//// 先不测试更新颜色缓存
			//printf("%d %d\n",x,y);
			if (0 <= x && x < windowWidth && 0 <= y && y < windowHeight)
			{
				//distance = (fragments[j].m_worldPos - lightPos).length();//到光源的距离？or到相机成像平面的距离？都可以？
				//colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // 写入颜色缓存
				if (fragments[j].m_depth > depthBuffer[x][y]) // 通过测试，缓存中的比较小，有时候共面的三角形深度相似，会误判
				{
					//printf("%f\n", fragments[j].m_depth);
					//if(camera->zBuffer[x][y] > -99)
					//	printf("%f 大 %f\n",fragments[j].m_depth, depthBuffer[x][y]);
					depthBuffer[x][y] = fragments[j].m_depth; // 写入深度值
					//colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // 写入颜色缓存
				}
			}
		}
	}

};

// 透明shader,深度测试不深度写入
class TransparentShader :public Shader
{
public:
	float alpha = 0.5f;
	virtual void Pass() override
	{
		// 正面剔除
		initialCulling = true;
		frontcull = true;
		backcull = false;

		VertexShader();
		ClipToNDCToScreen();
		Rasterization(); // 35ms
		FragmentShader(); // 51ms
		zTest(); // 6ms

		// 背面剔除
		initialCulling = true;
		frontcull = false;
		backcull = true;

		VertexShader();
		ClipToNDCToScreen();
		Rasterization(); // 35ms
		FragmentShader(); // 51ms
		zTest(); // 6ms
	}
	virtual void zTest() override
	{
		// 逐个片元测试，这里只进行深度测试，但是先不测试，有就直接上颜色
		int fragNum = fragments.size();
		//auto zBuffer = camera->zBuffer;
		for (int j = 0; j < fragNum; ++j)
		{
			int x = fragments[j].m_x, y = fragments[j].m_y;

			//// 先不测试更新颜色缓存
			if (0 <= x && x < windowWidth && 0 <= y && y < windowHeight)
			{
				//colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // 写入颜色缓存
				if (fragments[j].m_depth > camera->zBuffer[x][y]) // 通过测试，缓存中的比较小，有时候共面的三角形深度相似，会误判
				{
					//camera->zBuffer[x][y] = fragments[j].m_depth; // 写入深度值
					colorBuffer[x][y] = fragments[j].color * alpha + colorBuffer[x][y] * (1 - alpha); // 透明度混合后写入
				}
			}
		}
	}
};