#pragma once
#include "Gobals.h"
#include "Vector.h"
#include "Camera.h"
#include "Light.h"

class ShadowShader:public Shader
{
public:
	DirectionLight* dl;
	void CalculateShadowMap()// ������Ӱ��ͼ
	{
		// ��Դλ����Ϊ���λ��
		// �Գ��������������������ͶӰ MVP���õ�clip
		// Clip to NDC

		// ��ƬԪ������м������
		// ��Ȳ��ԣ��õ����ͼ
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

	// ��դ���׶�,GPU,����ÿ��ƬԪ����ȼ��ɣ�����ҪƬԪ��ɫ����Ҳ����Ҫ������Ļ���ꡢ��ȵĲ�ֵ
	virtual void FragmentShader()
	{
		//printf("x");
	}
	// �任����Դ��Clip�ռ䣬��Ϊw=1�����Բ���ҪҲ��
	Matrix3x3 GetLightVPMat()
	{
		return (*camera->viewMat * *camera->projectMat).toMatrix3x3();
	}
	virtual void zTest()
	{
		// ���ƬԪ���ԣ�����ֻ������Ȳ��ԣ������Ȳ����ԣ��о�ֱ������ɫ
		int fragNum = fragments.size();
		Vector3 lightPos = light->pos;
		//float distance = 0;
		//auto zBuffer = camera->zBuffer;
		for (int j = 0; j < fragNum; ++j)
		{
			int x = fragments[j].m_x, y = fragments[j].m_y;
			//if (fragments[j].color.x > this->colorBuffer[x][y].x)
			//{
			//	this->colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // д����ɫ����
			//}

			//// �Ȳ����Ը�����ɫ����
			//printf("%d %d\n",x,y);
			if (0 <= x && x < windowWidth && 0 <= y && y < windowHeight)
			{
				//distance = (fragments[j].m_worldPos - lightPos).length();//����Դ�ľ��룿or���������ƽ��ľ��룿�����ԣ�
				//colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // д����ɫ����
				if (fragments[j].m_depth > depthBuffer[x][y]) // ͨ�����ԣ������еıȽ�С����ʱ�����������������ƣ�������
				{
					//printf("%f\n", fragments[j].m_depth);
					//if(camera->zBuffer[x][y] > -99)
					//	printf("%f �� %f\n",fragments[j].m_depth, depthBuffer[x][y]);
					depthBuffer[x][y] = fragments[j].m_depth; // д�����ֵ
					//colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // д����ɫ����
				}
			}
		}
	}

};

// ͸��shader,��Ȳ��Բ����д��
class TransparentShader :public Shader
{
public:
	float alpha = 0.5f;
	virtual void Pass() override
	{
		// �����޳�
		initialCulling = true;
		frontcull = true;
		backcull = false;

		VertexShader();
		ClipToNDCToScreen();
		Rasterization(); // 35ms
		FragmentShader(); // 51ms
		zTest(); // 6ms

		// �����޳�
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
		// ���ƬԪ���ԣ�����ֻ������Ȳ��ԣ������Ȳ����ԣ��о�ֱ������ɫ
		int fragNum = fragments.size();
		//auto zBuffer = camera->zBuffer;
		for (int j = 0; j < fragNum; ++j)
		{
			int x = fragments[j].m_x, y = fragments[j].m_y;

			//// �Ȳ����Ը�����ɫ����
			if (0 <= x && x < windowWidth && 0 <= y && y < windowHeight)
			{
				//colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // д����ɫ����
				if (fragments[j].m_depth > camera->zBuffer[x][y]) // ͨ�����ԣ������еıȽ�С����ʱ�����������������ƣ�������
				{
					//camera->zBuffer[x][y] = fragments[j].m_depth; // д�����ֵ
					colorBuffer[x][y] = fragments[j].color * alpha + colorBuffer[x][y] * (1 - alpha); // ͸���Ȼ�Ϻ�д��
				}
			}
		}
	}
};