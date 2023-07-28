#include "Vector.h"
#include "Model.h"
#include "Camera.h"
#include "Light.h"
#include "Texture.h"
#include "math.h"
//#include "Shadow.h"
#include <vector>
using namespace std;

//#define Luminance(rgb) (0.2126f * rgb.x, 0.715f * rgb.y, 0.072f * rgb.z)
float Luminance(Vector3 rgb)
{
	return 0.2126f * rgb.x + 0.715f * rgb.y + 0.072f * rgb.z;
}


//void DrawTriangle(Triangle<Vector3>& Tri, vector<Fragment>& fms, Vector3 worldPos[3], Vector3 faceNormal[3], Vector3& viewPosZ, Vector2 uvs[3]);
//void DrawUpTriangle(Vector3 arr[3], const float& ymax, const float& ymin, vector<Fragment>& fms, Vector3 worldPos[3], Vector3& viewPosZ, Vector3 faceNormal[3], Triangle<Vector3>& Tri, Vector2 uvs[3]);// 只有第一个数组需要排序，其他数据初始顺序就可以
//void DrawDownTriangle(Vector3 arr[3], const float& ymax, const float& ymin, vector<Fragment>& fms, Vector3 worldPos[3], Vector3& viewPosZ, Vector3 faceNormal[3], Triangle<Vector3>& Tri, Vector2 uvs[3]);// 只有第一个数组需要排序，其他数据初始顺序就可以

void DrawColorBuffer(Window* w, Vector3(*colorBuffer)[windowWidth]);
//
//Vector3 CenterOfGravity(Vector3& v1, Vector3& v2, Vector3& v3, Vector2 p);
//Vector3 GravityToProperty3D(Vector3& a, Vector3& b, Vector3& c, Vector3& g, Vector3& zAbc, float& zt);
//Vector2 GravityToProperty2D(Vector2& a, Vector2& b, Vector2& c, Vector3& g, Vector3& zAbc, float& zt);

Vector3(*colorBuffer)[windowWidth] = new Vector3[windowWidth][windowHeight];
float(*depthBuffer)[windowWidth] = new float[windowWidth][windowHeight];

class Shader
{
public:
	Shader() 
	{
		rectPos[0] = Vector2(0, 0);//左上角
		rectPos[1] = Vector2(windowWidth, 0);//右上角
		rectPos[2] = Vector2(0, windowHeight);//左下角
		rectPos[3] = Vector2(windowWidth, windowHeight);//右下角
	};
	//Shader() {};//设置各种矩阵
	~Shader() {};

	Vector4* clipPosition = nullptr; // 裁切
	Vector2* screenPosition = nullptr; // 屏幕
	vector<Fragment> fragments; // 片元集合
	bool* backCulling = nullptr;// 背面剔除的三角形
	bool* frontCulling = nullptr;// 背面剔除的三角形
	bool* frustumCulling = nullptr;// 背面剔除的三角形
	bool isNDC = false;
	bool isNormalMap = false;
	//bool isTransparent = false;
	bool initialCulling = true;

	// 应该在v2f的：
	Vector3* worldPosition = nullptr; // 世界顶点坐标
	Vector3(*worldNormal)[3] = nullptr; // 世界法线方向，每个面有三个
	Vector3* viewPosition = nullptr;
	Vector2 rectPos[4];//屏幕点坐标

	Camera* camera = nullptr;
	Model* m = nullptr;
	Light* light = nullptr;

	Matrix4x4* VP = nullptr;
	// 静态的二维数组，有问题
	//static Vector3(*colorBuffer)[500] = new Vector3[500][500];
	//static Vector3 colorBuffer[windowWidth][windowWidth];
	//Vector3(*colorBuffer)[windowWidth] = new Vector3[windowWidth][windowHeight];

	Vector3(*fxaaBuffer)[windowWidth] = new Vector3[windowWidth][windowHeight];
	//Matrix4x4 modelMat;
	//Matrix4x4 viewMat;
	//Matrix4x4 projectMat;

	// 应该创建一个phong shader中的参数
	Vector3 shadowColor = Vector3(50,50,50);
	Vector3 c_d = Vector3(200, 250, 100);
	Texture* tex = nullptr;
	Vector3 c_s;
	float gloss;
	Texture* normalTex = nullptr;// 法线贴图

	float _RelativeThreshold = 0.6f;
	float _ContrastThreshold = 0.6f;

	bool backcull = true;
	bool frontcull = false;
	struct a2v
	{
		Vector4* Position = nullptr; // 模型空间坐标
		unsigned int num = 0; // 顶点数量
	};
	struct v2f
	{
		// 世界空间坐标
	};
	string GetFileExtension(const std::string& filePath) 
	{
		size_t dotPos = filePath.find_last_of(".");
		if (dotPos != std::string::npos && dotPos < filePath.length() - 1) {
			return filePath.substr(dotPos + 1);
		}
		return "";
	}
	//// 设置为透明(混合)，则深度测试时关闭深度写入
	//void SetTransparent()
	//{
	//	isTransparent = true;
	//}
	void SetParameter(Camera* c, Model& m, Light* l, const char* path, Vector3 c_s, Vector3 c_d, float gloss, bool bc = false/*背面剔除*/, Matrix4x4* vp = nullptr/*相机正交投影矩阵*/, const char* normalPath = nullptr,bool fc = false)
	{
		light = l;
		this->m = &m;
		camera = c;
		//c->SetModel(m.pos,m.rotate,m.scale);
		string fileExtension = GetFileExtension(path);
		TexType tp;
		if (fileExtension == "tga")
		{
			tp = TGA;
		}
		else
		{
			tp = BMP;
		}
		tex = new Texture(path,tp);
		this->c_s = c_s;
		this->c_d = c_d;
		this->gloss = gloss;
		this->VP = vp;
		this->backcull = bc;
		this->frontcull = fc;
		if (normalPath != nullptr)
		{
			isNormalMap = true;
			fileExtension = GetFileExtension(normalPath);
			if (fileExtension == "tga")
			{
				tp = TGA;
			}
			else
			{
				tp = BMP;
			}
			this->normalTex = new Texture(normalPath, tp);
		}
	}
	void SetModel(Model& m)
	{
		this->m = &m;
	}
	//Vector3(*Pass(Model& m))[windowWidth]
	virtual void Pass()
	{
		//ClearArray();
		VertexShader();
		ClipToNDCToScreen();
		Rasterization(); // 35ms
		FragmentShader(); // 51ms
		zTest(); // 6ms

		//FXAA();
		//return colorBuffer;
	}
	static void ClearBuffer(Camera *c)
	{
		// 使用 std::fill() 函数将所有元素赋值为 -100.0f
		std::fill(reinterpret_cast<float*>(c->zBuffer), reinterpret_cast<float*>(c->zBuffer) + windowHeight * windowWidth, -100.0f);
		//memset(c->zBuffer, -100.0f, windowHeight * windowWidth * sizeof(float));
		//std::fill(colorBuffer, colorBuffer + windowWidth * windowHeight, Vector3(0,0,0));
		// 将二维数组转换为一维数组并填充
		std::fill(reinterpret_cast<Vector3*>(colorBuffer), reinterpret_cast<Vector3*>(colorBuffer) + windowHeight * windowWidth, Vector3(0, 0, 0));
	}
	virtual void ClearFragment()
	{
		fragments.clear();
	}
	virtual void VertexShader()// 模型空间 返回 裁剪空间的齐次坐标
	{
		int vexNum = m->veretexNum;// 顶点数量
		int triNum = m->triangleNum;// 面数量

		// MVP变换到裁剪空间
		if (clipPosition == nullptr)
		{
			//int size = sizeof(input.Position) / sizeof(Vector4);
			//unsigned int size = input.num;
			worldPosition = new Vector3[vexNum]; // 用于片元插值得到世界坐标
			clipPosition = new Vector4[vexNum];  // 用于计算屏幕像素位置+深度缓存
			viewPosition = new Vector3[vexNum];  // 用于透视投影插值矫正
			for (int i = 0; i < vexNum; ++i)
			{
				//Vector4 model = *camera->modelMat * m.vertex[i];
				Vector4 model = *m->modelMat * m->vertexs[i];
				Vector4 view = *camera->viewMat * model;
				clipPosition[i] = *camera->projectMat * view;//可以是透视投影
				//clipPosition[i] = *camera->perspectMat * view;

				worldPosition[i] = model.ToVector3();

				viewPosition[i] = view.ToVector3();
				//*camera->modelMat * *camera->viewMat * *camera->MVP

				// 光源位置的NDC空间
				//lightSpace2dz[i] = *camera->VP * worldPosition[i];
				//lightSpace2dz[i] = lightSpace2dz[i] * 0.5f + Vector3(0.5f,0.5f,0.5f);
			}
		}
		// 初始化剔除数组
		if (initialCulling)
		{
			// 背面和正面剔除
			backCulling = new bool[triNum];
			frontCulling = new bool[triNum];
			// 视锥体剔除三角形
			frustumCulling = new bool[triNum];
			memset(frustumCulling, false, triNum * sizeof(bool));//全部关闭，每帧更新
			memset(backCulling, false, triNum * sizeof(bool));
			memset(frontCulling, false, triNum * sizeof(bool));


			//第一次计算背面和正面剔除
			// 背面剔除计算
			if (backcull)
			{
				//backCulling = new bool[triNum];
				memset(backCulling, false, triNum * sizeof(bool)); //全部不剔除
				if (backcull)
				{
					for (uint32_t i = 0; i < triNum; ++i)
					{
						Vector3 n = cross((worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x])).normalized();
						Vector3 viewDir = (worldPosition[(uint32_t)m->face[i].x] - camera->pos).normalized(); // 任意一个点
						if (dot(n, viewDir) + 0.00001f >= 0) // >= 0不可见
						{
							backCulling[i] = true;
						}
					}
				}
			}
			// 正面剔除计算
			else if (frontcull)
			{
				//frontCulling = new bool[triNum];
				memset(frontCulling, false, triNum * sizeof(bool)); //全部不剔除
				if (frontcull)
				{
					for (uint32_t i = 0; i < triNum; ++i)
					{
						Vector3 n = cross((worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x])).normalized();
						Vector3 viewDir = (worldPosition[(uint32_t)m->face[i].x] - camera->pos).normalized(); // 任意一个点
						if (dot(n, viewDir) + 0.00001f < 0) // 可见剔除
						{
							frontCulling[i] = true;
						}
					}
				}
			}
			initialCulling = false;
		}
		if(m->TBN == nullptr)
		{
			m->TBN = new Matrix3x3[triNum][3]; // 每个三角面片有三个TBN矩阵
			worldNormal = new Vector3[triNum][3];
			if (normalTex == nullptr)//没有法线贴图，默认TBN矩阵为单位阵，因为法线不需要变换
			{
				// 求世界空间法线方向，每个面对应三个顶点，一个顶点可能有多个法线
				for (int j = 0; j < triNum; ++j)
					for (int k = 0; k < 3; ++k)
					{
						//worldNormal[j][k] = m->TBN[j][k] * (normalTex->SampleLinearFilter(m->triUvs[j][k]) * twoDivide255 - 1.0f); // 没有叠加的，直接用扰动的值
						worldNormal[j][k] = (*m->normalToWorldMat * m->triNormals[j][k]).normalized();
						//printf("%d %d : %f %f %f\n",j,k, worldNormal[j][k].x, worldNormal[j][k].y, worldNormal[j][k].z);
					}
			}
			else
			{
				// 计算每个面对应的TBN矩阵(3x3)
				Vector3 E1, E2, n, t, b;
				Vector2 deltaE1, deltaE2;
				float deno = 0.0f;
				for (uint32_t i = 0; i < triNum; i++)
				{
					//m->TBN[i][0] = Matrix3x3(Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
					//m->TBN[i][1] = Matrix3x3(Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
					//m->TBN[i][2] = Matrix3x3(Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));

					// 2的TBN矩阵
					E1 = worldPosition[(uint32_t)m->face[i].x] - worldPosition[(uint32_t)m->face[i].z];
					E2 = worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].z];
					deltaE1 = m->triUvs[i][0] - m->triUvs[i][2];
					deltaE2 = m->triUvs[i][1] - m->triUvs[i][2];
					deno = 1.0f / (deltaE1.x * deltaE2.y - deltaE2.x * deltaE1.y);
					//n = cross(E2, E1).normalized();
					n = (*m->normalToWorldMat * m->triNormals[i][2]).normalized();
					t = (Vector3(deltaE2.y * E1.x - deltaE1.y * E2.x, deltaE2.y * E1.y - deltaE1.y * E2.y, deltaE2.y * E1.z - deltaE1.y * E2.z) * deno).normalized();
					t = (t - n * dot(t,n)).normalized();
					b = cross(t,n).normalized();
					// 可以进行施密特正交化，暂时先不做
					m->TBN[i][2] = Matrix3x3(Vector3(t.x,b.x,n.x), Vector3(t.y, b.y, n.y), Vector3(t.z, b.z, n.z));
					//m->TBN[i][0] = Matrix3x3(t, n, b);

					// 1的TBN矩阵
					E1 = worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].y];
					E2 = worldPosition[(uint32_t)m->face[i].x] - worldPosition[(uint32_t)m->face[i].y];
					deltaE1 = m->triUvs[i][2] - m->triUvs[i][1];
					deltaE2 = m->triUvs[i][0] - m->triUvs[i][1];
					deno = 1.0f / (deltaE1.x * deltaE2.y - deltaE2.x * deltaE1.y);
					//n = cross(E2, E1).normalized();
					n = (*m->normalToWorldMat * m->triNormals[i][1]).normalized();;
					t = (Vector3(deltaE2.y * E1.x - deltaE1.y * E2.x, deltaE2.y * E1.y - deltaE1.y * E2.y, deltaE2.y * E1.z - deltaE1.y * E2.z) * deno).normalized();
					t = (t - n * dot(t, n)).normalized();
					b = cross(t,n).normalized();
					// 可以进行施密特正交化，暂时先不做
					//m->TBN[i][1] = Matrix3x3(t, n, b);
					m->TBN[i][1] = Matrix3x3(Vector3(t.x, b.x, n.x), Vector3(t.y, b.y, n.y), Vector3(t.z, b.z, n.z));

					// 0的TBN矩阵
					E1 = worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x];
					E2 = worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x];
					deltaE1 = m->triUvs[i][1] - m->triUvs[i][0];
					deltaE2 = m->triUvs[i][2] - m->triUvs[i][0];
					deno = 1.0f / (deltaE1.x * deltaE2.y - deltaE2.x * deltaE1.y);
					//n = cross(E2, E1).normalized();
					n = (*m->normalToWorldMat * m->triNormals[i][0]).normalized();
					t = (Vector3(deltaE2.y * E1.x - deltaE1.y * E2.x, deltaE2.y * E1.y - deltaE1.y * E2.y, deltaE2.y * E1.z - deltaE1.y * E2.z) * deno).normalized();
					t = (t - n * dot(t, n)).normalized();
					b = cross(t,n).normalized();
					// 可以进行施密特正交化，暂时先不做
					//m->TBN[i][2] = Matrix3x3(t, n, b);
					m->TBN[i][0] = Matrix3x3(Vector3(t.x, b.x, n.x), Vector3(t.y, b.y, n.y), Vector3(t.z, b.z, n.z));

					// 进行法线贴图采样
					for (int k = 0; k < 3; ++k)
					{
						//Vector3 sp = normalTex->SampleLinearFilter(m->triUvs[i][k]);
						//sp = sp * twoDivide255 - 1.0f;
						//worldNormal[i][k] = (m->TBN[i][k] * sp).normalized(); // 没有叠加的，直接用扰动的值
						//n = (*m->normalToWorldMat * m->triNormals[i][k]).normalized();
						//if (!equals(worldNormal[i][k],n))
						//{
						//	cout << worldNormal[i][k];
						//	cout << n;
						//	printf("xxxxxxx\n");
						//}
						//worldNormal[i][k] = (*m->normalToWorldMat * m->triNormals[i][k] + m->TBN[i][k] * (normalTex->SampleLinearFilter(m->triUvs[i][k]) * twoDivide255 - 1.0f)).normalized(); // 没有叠加的，直接用扰动的值
						worldNormal[i][k] = (*m->normalToWorldMat * m->triNormals[i][k]).normalized();
						//printf("%d %d : %f %f %f\n",j,k, worldNormal[j][k].x, worldNormal[j][k].y, worldNormal[j][k].z);
					}
				}
			}
		}
		if (camera->view_update)  // 相机矩阵已更新
		{
			// 更新view坐标
			for (int i = 0; i < vexNum; ++i)
			{
				//Vector4 model = *m->modelMat * m->vertexs[i];
				Vector4 view = *camera->viewMat * ToVector4(worldPosition[i]);

				//worldPosition[i] = model.ToVector3();
				//clipPosition[i] = *camera->perspectMat * view;
				clipPosition[i] = *camera->projectMat * view;

				viewPosition[i] = view.ToVector3();
				//*camera->modelMat * *camera->viewMat * *camera->MVP
			}
			//memset();
			if (backcull)
			{
				// 重新计算一次是否剔除
				memset(backCulling, false, triNum * sizeof(bool));
				for (uint32_t i = 0; i < triNum; ++i)
				{
					Vector3 n = cross((worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x])).normalized();
					//Vector3 n = cross((worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x])).normalized();
					Vector3 viewDir = (worldPosition[(uint32_t)m->face[i].x] - camera->pos).normalized(); // 任意一个点
					if (dot(n, viewDir) + 0.00001f >= 0) //不可见
					{
						backCulling[i] = true;
					}
				}
			}
			// 正面剔除
			if (frontcull)
			{
				// 重新计算一次是否剔除
				memset(frontCulling, false, triNum * sizeof(bool));
				for (uint32_t i = 0; i < triNum; ++i)
				{
					Vector3 n = cross((worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x])).normalized();
					//Vector3 n = cross((worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x])).normalized();
					Vector3 viewDir = (worldPosition[(uint32_t)m->face[i].x] - camera->pos).normalized(); // 任意一个点
					if (dot(n, viewDir) + 0.00001f < 0) // 可见
					{
						frontCulling[i] = true;
					}
				}
			}
			// 视锥体剔除，默认开启，这里全部false，在clip2NDC阶段才能判断
			memset(frustumCulling, false, triNum * sizeof(bool));
		}
	}

	// 裁剪空间到NDC空间，再到屏幕空间 自动调用
	void ClipToNDCToScreen()
	{
		int triNum = m->triangleNum;
		if (!isNDC)
		{
			uint32_t idx[3];
			int vaild = 0, tooNear = 0;
			float w;
			m->triangles = new Triangle<Vector3>[triNum];
			for (int i = 0; i < triNum; ++i) // 12个三角形，进行逐个变换，存储片元到2D三角形中
			{
				if (backCulling[i] || frustumCulling[i] || frontCulling[i]) // 剔除
					continue;
				idx[0] = (uint32_t)m->face[i][0];
				idx[1] = (uint32_t)m->face[i][1];
				idx[2] = (uint32_t)m->face[i][2];
				// 一个三角形的三个顶点
				vaild = 0;
				tooNear = 0;
				for (int j = 0; j < 3; ++j)
				{
					w = clipPosition[idx[j]].w + 0.0001f;
					m->triangles[i][j].x = ((clipPosition[idx[j]].x / w) * 0.5f + 0.5f) * windowWidth; // 第i个三角形，对应的顶点的索引
					m->triangles[i][j].y = ((clipPosition[idx[j]].y / w) * 0.5f + 0.5f) * windowHeight;
					// 深度缓存
					// camera->zBuffer[(int)screenPosition[i].x][(int)screenPosition[i].y] = clipPosition[i].z / clipPosition[i].w * 0.5f + 0.5f; // 最深就是1，最近就是0
					// m.triangles[i].vertexDepth[j] = clipPosition[(uint16_t)m.face[i][j]].z / clipPosition[(uint16_t)m.face[i][j]].w * 0.5f + 0.5f; // 最深就是1，最近就是0
					m->triangles[i][j].z = clipPosition[idx[j]].z / w; // 最深就是1，最近就是0
					//printf("%d x = %.f  | y = %.f 对应顶点 %d\n", j, m->triangles[i][j].x, m->triangles[i][j].y, (uint16_t)m->face[i][j]);
					//printf("%d z = %.5f 对应顶点 %d\n", j, m->triangles[i][j].z, (uint16_t)m->face[i][j]);

					//printf("%d %d z=%f clipz = %f clipw = %f\n",i,j, m->triangles[i][j].z, clipPosition[idx[j]].z, clipPosition[idx[j]].w);
					//printf("XXXXX tri = %d %f,%f,%f XXXXX\n", i, m->triangles[i][j].x, m->triangles[i][j].y, m->triangles[i][j].z);
					if ((m->triangles[i][j].x < 0 || m->triangles[i][j].x > windowWidth) || (m->triangles[i][j].y < 0 || m->triangles[i][j].y > windowHeight))
					{
						//printf("XXXXX %f,%f,%f XXXXX\n", m->triangles[i][j].x, m->triangles[i][j].y, m->triangles[i][j].z);
						vaild++;
					}
					if (m->triangles[i][j].z > 1.055f || m->triangles[i][j].z < 0.2f)
					{
						tooNear++;
					}
				}
				//如果一个三角形的三个顶点都在外面，也就是不在[0,1]范围内，则剔除
				//if (vaild == 3)
				//{
				//	printf("裁剪%d\n", i);
				//	frustumCulling[i] = true;
				//}
				//如果一个三角形的三个顶点都在外面，也就是不在[0,1]范围内，则剔除
				if (vaild == 3)
				{
					int i = 0;
					//二次判断，四个边界顶点需要至少在这个三角形内部
					for (int i = 0 ; i < 4; i++)
					{
						// 有一个在内部
						if (PointInTriangle(rectPos[i],m->triangles[i][0].toVector2(),m->triangles[i][1].toVector2(), m->triangles[i][2].toVector2()))
						{
							break;
						}
					}
					if (i == 4)
					{
						printf("裁剪%d\n", i);
						frustumCulling[i] = true;
					}
				}
				// 三个太近了
				if (tooNear == 3)
				{
					frustumCulling[i] = true;
				}
			}
			isNDC = true;
		}
		if (camera->view_update)
		{
			uint32_t idx[3];
			int vaild = 0,tooNear = 0;
			float w;
			for (int i = 0; i < triNum; ++i) // 12个三角形，进行逐个变换，存储片元到2D三角形中
			{
				if (backCulling[i] || frustumCulling[i] || frontCulling[i]) // 剔除
					continue;
				idx[0] = (uint32_t)m->face[i][0];
				idx[1] = (uint32_t)m->face[i][1];
				idx[2] = (uint32_t)m->face[i][2];
				vaild = 0;
				tooNear = 0;
				// 一个三角形的三个顶点
				for (int j = 0; j < 3; ++j)
				{
					w = clipPosition[idx[j]].w + 0.0001f;
					m->triangles[i][j].x = ((clipPosition[idx[j]].x / w) * 0.5f + 0.5f) * windowWidth; // 第i个三角形，对应的顶点的索引
					m->triangles[i][j].y = ((clipPosition[idx[j]].y / w) * 0.5f + 0.5f) * windowHeight;
					// 深度缓存
					// camera->zBuffer[(int)screenPosition[i].x][(int)screenPosition[i].y] = clipPosition[i].z / clipPosition[i].w * 0.5f + 0.5f; // 最深就是1，最近就是0
					// m.triangles[i].vertexDepth[j] = clipPosition[(uint16_t)m.face[i][j]].z / clipPosition[(uint16_t)m.face[i][j]].w * 0.5f + 0.5f; // 最深就是1，最近就是0
					m->triangles[i][j].z = clipPosition[idx[j]].z / w; // 最深就是1，最近就是0
					//printf("%d x = %.f  | y = %.f 对应顶点 %d\n", j, m.triangles[i][j].x, m.triangles[i][j].y, (uint16_t)m.face[i][j]);
					//printf("%d z = %.5f 对应顶点 %d\n", j, m.triangles[i][j].z, (uint16_t)m.face[i][j]);
					//printf("%d %d z=%f\n", i, j, m->triangles[i][j].z);

					//printf("%d %d z=%f\n", i, j, m->triangles[i][j].z);
					//if (m->triangles[i][j].x < 0 && m->triangles[i][j].x > 1.0f && m->triangles[i][j].y < 0 && m->triangles[i][j].y > 1.0f && m->triangles[i][j].z < 0 && m->triangles[i][j].z > 1.0f)
				//printf("XXXXX tri = %d %f,%f,%f XXXXX\n", i,m->triangles[i][j].x, m->triangles[i][j].y, m->triangles[i][j].z);
				if ((m->triangles[i][j].x < 0 || m->triangles[i][j].x > windowWidth) || (m->triangles[i][j].y < 0 || m->triangles[i][j].y > windowHeight) || (m->triangles[i][j].z > 1.103679f)/*太近了*/)
				{
					//printf("XXXXX %f,%f,%f XXXXX\n", m->triangles[i][j].x, m->triangles[i][j].y, m->triangles[i][j].z);
					vaild++;
				}
				if (m->triangles[i][j].z > 1.055f || m->triangles[i][j].z < 0.2f)
				{
					tooNear++;
				}
				}
				//如果一个三角形的三个顶点都在外面，也就是不在[0,1]范围内，则剔除
				if (vaild == 3)
				{
					int i = 0;
					//二次判断，四个边界顶点需要至少在这个三角形内部
					for (int i = 0; i < 4; i++)
					{
						// 有一个在内部
						if (PointInTriangle(rectPos[i], m->triangles[i][0].toVector2(), m->triangles[i][1].toVector2(), m->triangles[i][2].toVector2()))
						{
							break;
						}
					}
					if (i == 4)
					{
						printf("裁剪%d\n", i);
						frustumCulling[i] = true;
					}
				}
				// 三个太近了
				if (tooNear == 3)
				{
					frustumCulling[i] = true;
				}
			}
		}
	}
	// 判断p是否在二维空间三个顶点内部
	bool PointInTriangle(Vector2 &P, Vector2 A, Vector2 B, Vector2 C) {
		Vector2 AB = B - A;
		Vector2 AP = P - A;

		Vector2 BC = C - B;
		Vector2 BP = P - B;

		Vector2 CA = A - C;
		Vector2 CP = P - C;

		// 计算三个叉积
		double cp1 = crossProduct(AB, AP);
		double cp2 = crossProduct(BC, BP);
		double cp3 = crossProduct(CA, CP);

		// 如果三个叉积都具有相同的符号，则点在三角形内部
		return (cp1 >= 0 && cp2 >= 0 && cp3 >= 0) || (cp1 <= 0 && cp2 <= 0 && cp3 <= 0);
	}
	// 光栅化阶段 ，GPU
	virtual void Rasterization()
	{
		int triNum = m->triangleNum;
		//int num = 12;
		// 对每个三角形进行遍历，得到其中的片元，并且插值，得到所有片元的2D屏幕位置，并且插值得到其颜色、法线等等（后续）
		uint32_t a_idx, b_idx, c_idx;
		//uint32_t a_uv_idx, b_uv_idx, c_uv_idx;
		Vector3 worldPos[3];
		//Vector3 faceWorldNormal = m->triNormals[i][0]; // 不插值法线？直接取其中一个？
		//frustumCulling[0] = false;
		for (int i = 0; i < triNum; ++i)// 如果背面剔除，那么对应的背面三角形就直接过
		{
			if (backCulling[i] || frustumCulling[i] || frontCulling[i]) // 剔除
				continue;
			a_idx = (uint32_t)m->face[i][0];
			b_idx = (uint32_t)m->face[i][1];
			c_idx = (uint32_t)m->face[i][2];

			//a_uv_idx = (uint32_t)m->face_uv[i][0];
			//b_uv_idx = (uint32_t)m->face_uv[i][1];
			//c_uv_idx = (uint32_t)m->face_uv[i][2];
			// 对深度、法线、位置、uv进行插值
			worldPos[0] = worldPosition[a_idx];
			worldPos[1] = worldPosition[b_idx];
			worldPos[2] = worldPosition[c_idx];

			Vector3 viewPosZ = Vector3(viewPosition[a_idx].z, viewPosition[b_idx].z, viewPosition[c_idx].z);
			Vector2 uvs[3]{ m->triUvs[i][0],m->triUvs[i][1] ,m->triUvs[i][2] };
			//int before = fragments.size();
			
			DrawTriangle(m->triangles[i], fragments, worldPos, worldNormal[i], viewPosZ, uvs, m->TBN[i][0]); // 一个个push_back好像有点不太高效？
			//printf("三角形%d 增加%d\n", i,fragments.size() - before);
		}
	}

	virtual void FragmentShader() // 三角形长度
	{
		//fill(colorBuffer, colorBuffer + windowWidth * windowHeight, Vector3(0,0,0));
		// 逐个片元进行着色
		//Vector3 tmpColor;
		// 逐个片元进行着色
		//Vector3 worldLightDir = Vector3(0,0,0); // 可能是平行光或者点光源

		//if (typeid(*light) == typeid(PointLight)) {
			//PointLight* pl = dynamic_cast<PointLight*>(light);
			//// 访问 PointLight 的成员
			////Vector3 pos = pl->pos;
			//Vector3 lightDir = Vector3();
		Vector3 viewDir = Vector3();
		//}
		//else if (typeid(*light) == typeid(DirectionLight)) {
		DirectionLight* dl = dynamic_cast<DirectionLight*>(light);
		//	 //访问 DirectionLight 的成员
		Vector3 lightDir = (-dl->dir).normalized();// 求反方向
	//}

		Vector3 half = Vector3();
		Vector3 reflect = Vector3();
		Vector3 n = Vector3();
		int fragNum = fragments.size();
		for (uint32_t j = 0; j < fragNum; ++j)
		{
			//fragments[j].color = Vector3(200,200,200);
			//continue;

			// 阴影的计算
			//if (FragmentInShadow(fragments[j].m_worldPos)) // 处于阴影的地方
			//{
			//	fragments[j].color = shadowColor;
			//	continue;
			//}

			//fragments[j].color = normalTex->SampleLinearFilter(fragments[j].uv) * twoDivide255 - 1.0f;
			//cout << fragments[j].color;
			//continue;

			//n = (fragments[j].normal + m->TBN[j] * normalTex->SampleLinearFilter(fragments[j].uv) * twoDivide255 - 1.0f).normalized(); // 法线扰动
			//fragments[j].color = (fragments[j].normal+ 1.0f) * 127.0f;
			//cout << fragments[j].color;
			//continue;
			//cout << fragments[j].normal;
			//Fragment &fg = fragments[j];
			//lightDir = (pl->pos - fragments[j].m_worldPos).normalized();
			viewDir = (this->camera->pos - fragments[j].m_worldPos).normalized();
			//reflect = (n * 2 * dot(n, lightDir) - lightDir).normalized();
			//Vector3 color = LambertModel(fragments[j].normal, lightDir); // Vector3(200, 200, 200);
			//half = (lightDir + viewDir).normalized();
			//fragments[j].normal = Vector3(0, 0, -1);
			Vector3 color = BlinnPhongModel(fragments[j].normal, lightDir, half, fragments[j].uv); // Vector3(200, 200, 200);
			//Vector3 color = BlinnPhongModel(n, lightDir,reflect,fragments[j].uv); // Vector3(200, 200, 200);

			fragments[j].color = color.clamp(0, 255);
		}
	}
	// 法线贴图采样颜色映射到[-1,1]范围  [0,255] -> [-1,1]
	Vector3 NormalTexToNormal(Vector3 color)
	{
		color = color * oneDivide255 * 2.0f - 1.0f; // [0,1]
		//color = color * 2.0f - 1.0f;  // [-1,1]
	}
	bool FragmentInShadow(Vector3 &worldPos)
	{
		//Vector3 ts = Vector3(-2, 0, 2);
		Vector4 w2l = *VP * ToVector4(worldPos);//光源相机的屏幕空间
		//w2l = w2l * 0.5f + Vector3(0.5f,0.5f,0.5f);
		w2l = w2l * 0.5f + Vector4(0.5f, 0.5f, 0.5f, 0.5f);//变换到[0,1]范围内
		float depth = SampleInDepthBuffer(w2l.x,w2l.y);
		//if (equals(worldPos, ts))
		//{
		//	printf("x=%f y=%f z=%f !! depth = %f\n", w2l.x * windowWidth, w2l.y * windowHeight, w2l.z, depth);
		//}
		return (w2l.z + 0.01f) < depth;
	}

	Vector3 LambertModel(Vector3& normal, Vector3& worldLightDir)
	{
		float f = dot(normal, worldLightDir);
		return light->color * this->c_d * max(0, f);
	}
	Vector3 HalfLambertModel(Vector3& normal, Vector3& worldLightDir)
	{
		return light->color * (dot(normal, worldLightDir) * 0.5f + 0.5f);
	}
	Vector3 PhongModel(Vector3& n, Vector3& l, Vector3& r, Vector2& uv) // 这应该封装为一个PhongShader类
	{
		//Vector3 tx = this->tex->SampleLinearFilter(uv);
		Vector3 diff = light->color * this->c_d * max(0, dot(n, l));
		Vector3 spec = light->color * this->c_s * pow(max(0, dot(n, r)), this->gloss);
		return diff + spec;
	}
	Vector3 BlinnPhongModel(Vector3& n, Vector3& l, Vector3& half, Vector2& uv) // 这应该封装为一个PhongShader类
	{
		//n = n * 2.0f;
		//Vector3 tx = this->tex->SampleLinearFilter(uv);
		//Vector3 tx = this->tex->SimpleSample(uv);
		//cout << tx;
		return light->color * tex->SimpleSample(uv) * max(0, dot(n, l)); // this->c_d
		//Vector3 spec = light->color * this->c_s * pow(max(0, dot(n, half)), this->gloss);
		//return diff;
	}
	virtual void zTest()
	{
		// 逐个片元测试，这里只进行深度测试，但是先不测试，有就直接上颜色
		int fragNum = fragments.size();
		//auto zBuffer = camera->zBuffer;
		for (int j = 0; j < fragNum; ++j)
		{
			//Vector3 w2l = *VP * fragments[j].m_worldPos;//光源相机的屏幕空间
			//w2l = w2l * 0.5f + Vector3(0.5f, 0.5f, 0.5f);//变换到[0,1]范围内

			//int x = w2l.x * windowWidth, y = w2l.y * windowHeight;
			int x = fragments[j].m_x, y = fragments[j].m_y;
			//printf("%d %d %f %f\n",x,y, fragments[j].m_depth, camera->zBuffer[x][y]);
			//if (fragments[j].color.x > this->colorBuffer[x][y].x)
			//{
			//	this->colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // 写入颜色缓存
			//}

			//// 先不测试更新颜色缓存
			if (0 <= x && x < windowWidth && 0 <= y && y < windowHeight)
			{
				//colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // 写入颜色缓存
				if (fragments[j].m_depth > camera->zBuffer[x][y]) // 通过测试，缓存中的比较小，有时候共面的三角形深度相似，会误判
				{
					camera->zBuffer[x][y] = fragments[j].m_depth; // 写入深度值
					colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // 写入颜色缓存
				}
			}
		}
	}
	// 后处理抗锯齿，存储结果到新的颜色缓存中
	void FXAA()
	{
		Vector2 uv, uvNW, uvNE, uvSW, uvSE;
		float M, NW, NE, SW, SE;
		for (uint32_t i = 0; i < windowWidth; ++i)
		{
			for (uint32_t j = 0; j < windowHeight; ++j)
			{
				//uv = Vector2(i,j);//当前像素的uv坐标，整数
				//uvNW = Vector2(i - 0.5f, j + 0.5f);
				//uvNE = Vector2(i + 0.5f, j + 0.5f);
				//uvSW = Vector2(i - 0.5f, j - 0.5f);
				//uvSE = Vector2(i + 0.5f, j - 0.5f);

				M = Luminance(SampleInColorBuffer(i, j));
				NW = Luminance(SampleInColorBuffer(i - 0.5f, j + 0.5f));
				NE = Luminance(SampleInColorBuffer(i + 0.5f, j + 0.5f));
				SW = Luminance(SampleInColorBuffer(i - 0.5f, j - 0.5f));
				SE = Luminance(SampleInColorBuffer(i + 0.5f, j - 0.5f));

				float MaxLuma = max(max(NW, NE), max(SW, SE));
				float MinLuma = min(min(NW, NE), min(NW, NE));
				float Contrast = max(MaxLuma, M) - min(MinLuma, M);


				// 对比度不足，表示在内部
				if (Contrast < max(0.5f, MaxLuma * 0.5f))
				{
					fxaaBuffer[i][j] = colorBuffer[i][j]; // 复制即可
					continue;
				}
				//// 否则，计算切线方向
				NE += 1.0f / 384.0f;
				Vector2 dir1 = Vector2(-((NW + NE) - (SW + SE)), (NE + SE) - (NW + SW)).normalized();
				Vector3 N1 = SampleInColorBuffer(i + dir1.x, j + dir1.y);
				Vector3 P1 = SampleInColorBuffer(i - dir1.x, j - dir1.y);
				//Vector2 pdir1 = uv + dir1;
				//Vector2 ndir1 = uv - dir1;

				//fxaaBuffer[i][j] = (N1+P1)*0.5f; 
				Vector3 result = (N1 + P1) * 0.5f;

				// 水平or横向的调整
				const float _Sharpness = 8;
				float dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * _Sharpness;
				Vector2 dir2 = Vector2(Clamp(dir1.x / dirAbsMinTimesC, -2.0, 2.0), Clamp(dir1.y / dirAbsMinTimesC, -2.0, 2.0)) * 2.0f;
				Vector3 N2 = SampleInColorBuffer(i - dir2.x * 0.5f, i - dir2.y * 0.5f);
				Vector3 P2 = SampleInColorBuffer(i + dir2.x * 0.5f, i + dir2.y * 0.5f);
				Vector3 result2 = result * 0.5f + (N2 + P2) * 0.25f;
				// 如果新的结果，亮度在正确范围内，则使用新的结果
				float NewLum = Luminance(result2);
				if ((NewLum >= MinLuma) && (NewLum <= MaxLuma)) {
					result = result2;
				}

				fxaaBuffer[i][j] = result;
			}
		}
	}
	// 双线性插值采样colorBuffer
	Vector3 SampleInColorBuffer(float x, float y) // 这里xy值是坐标值，(0.5,0.5) (498.5,498.5)
	{
		float u_t = Clamp(0.5f, 498.5f, x);
		float v_t = Clamp(0.5f, 498.5f, y);

		int u_0 = (int)u_t; // 左
		int v_0 = (int)v_t; // 下

		float a = u_t - u_0;
		float b = v_t - v_0;

		//float uv00 = (1 - a) * (1 - b); // 左下角权重
		//float uv10 = a * (1 - b);
		//float uv01 = (1 - a) * b;
		//float uv11 = a * b;

		//Vector3 result = 
		//	colorBuffer[u_0][v_0] * uv00 +
		//	colorBuffer[u_0 + 1][v_0] * uv10 +
		//	colorBuffer[u_0][v_0 + 1] * uv01 +
		//	colorBuffer[u_0 + 1][v_0 + 1] * uv11;
		return
			colorBuffer[u_0][v_0] * ((1 - a) * (1 - b)) +
			colorBuffer[u_0 + 1][v_0] * (a * (1 - b)) +
			colorBuffer[u_0][v_0 + 1] * ((1 - a) * b) +
			colorBuffer[u_0 + 1][v_0 + 1] * (a * b);
	}
	// 双线性插值采样colorBuffer
	float SampleInDepthBuffer(float &x, float &y) // 这里xy值是浮点数 [0,1]
	{
		// 可以不限制，如循环纹理
		x = Clamp(0, 1.0f, x);
		y = Clamp(0, 1.0f, y);
		float u_t = x * windowWidth;
		float v_t = y * windowHeight;

		int u_0 = (int)u_t; // 左
		int v_0 = (int)v_t; // 下

		float a = u_t - u_0;
		float b = v_t - v_0;

		return
			depthBuffer[u_0][v_0] * ((1 - a) * (1 - b)) +
			depthBuffer[u_0 + 1][v_0] * (a * (1 - b)) +
			depthBuffer[u_0][v_0 + 1] * ((1 - a) * b) +
			depthBuffer[u_0 + 1][v_0 + 1] * (a * b);
	}
	float Clamp(float min, float max, float value)
	{
		if (value > max)
			value = max;
		if (value < min)
			value = min;
		return value;
	}
	const float epsilon = 0.00001f;
	void DrawTriangle(Triangle<Vector3>& Tri, vector<Fragment>& fms, Vector3 worldPos[3], Vector3 worldNormal[3], Vector3& viewPosZ, Vector2 uvs[3],Matrix3x3 &TBN)
	{
		// 对三个顶点按y坐标从小到大排序
		Vector3 arr[3] = { Tri.x, Tri.y, Tri.z }; // 屏幕坐标 + z
		//Vector2 worldPos[3] = { Tri.x , Tri.y , Tri.z };
		std::sort(arr, arr + 3, cmp2dz);
		//std::sort(worldPos, worldPos + 3, cmp3d);

		// 将arr和uvs数组的元素组合成Vertex类型的对象
		//VertexData TriVertices[3];
		//for (int i = 0; i < 3; i++) 
		//{
		//	TriVertices[i] = VertexData(arr[i],uvs[i],worldPos[i],viewPosZ[i]); // 三角形全部属性，这是插值需要的，对应于屏幕的三个点顺序，因此组合再一起然后排序
		//}

		float ymin = arr[0].y, ymax = arr[2].y;

		if (equals(arr[0].y, arr[1].y)) // 下三角形
		{
			DrawDownTriangle(arr, ymax, ymin, fms, worldPos, viewPosZ, worldNormal, Tri, uvs, TBN);
		}
		else if (equals(arr[1].y, arr[2].y)) // 上三角形
		{
			//printf("上三角");
			DrawUpTriangle(arr, ymax, ymin, fms, worldPos, viewPosZ, worldNormal, Tri, uvs, TBN);
		}
		//else if (equals3(arr[0].y, arr[1].y, arr[2].y,0.00001f))
		//{
		//	return;
		//}
		else // 上下都有
		{
			//printf("上下都有");
			float k = 0;
			Vector3 midPoint(0, arr[1].y, 1); // 测试，这里的最后面的是view空间的深度值，这里是没有用的

			if (equals(arr[2].x, arr[0].x))
				midPoint.x = arr[2].x;
			else
			{
				k = (arr[2].y - arr[0].y) / (arr[2].x - arr[0].x);
				midPoint.x = 1.0f / k * (midPoint.y - arr[0].y) + arr[0].x;
			}
			// 求z深度值
			//if (equals(arr[2].z, arr[0].z), 0.000001f)
			//	midPoint.z = arr[2].z;
			//else
			//{
			//	k = (arr[2].y - arr[0].y) / (arr[2].z - arr[0].z);
			//	midPoint.z = 1.0f / k * (midPoint.y - arr[0].y) + arr[2].z;
			//}
			if (equals(arr[2].z, arr[0].z))//没有深度变化
			{
				midPoint.z = arr[0].z;
			}
			else
			{
				k = (arr[0].z - arr[2].z) / (arr[0].y - arr[2].y);
				midPoint.z = k * (midPoint.y - arr[2].y) + arr[2].z;
			}

			// 上下三角形
			Vector3 upArr[3] = { arr[0],midPoint,arr[1] };
			Vector3 downArr[3] = { arr[1],arr[2],midPoint };

			std::sort(upArr, upArr + 3, cmp2dz);
			std::sort(downArr, downArr + 3, cmp2dz);

			// 上三角 世界坐标
			//Vector3 tmp = worldPos[2];
			//worldPos[2] = worldMidPoint;
			//std::sort(worldPos, worldPos + 3, cmp3d);
			DrawUpTriangle(upArr, arr[1].y, arr[0].y, fms, worldPos, viewPosZ, worldNormal, Tri, uvs, TBN);

			// 下三角 世界坐标
			//worldPos[2] = tmp;
			//worldPos[1] = worldMidPoint;
			//std::sort(worldPos, worldPos + 3, cmp3d);
			DrawDownTriangle(downArr, arr[2].y, arr[1].y, fms, worldPos, viewPosZ, worldNormal, Tri, uvs, TBN);
		}
	}

	void DrawUpTriangle(Vector3 arr[3], const float& ymax, const float& ymin, vector<Fragment>& fms, Vector3 worldPos[3], Vector3& viewPosZ, Vector3 worldNormal[3], Triangle<Vector3>& Tri, Vector2 uvs[3], Matrix3x3& TBN)// 只有第一个数组需要排序，其他数据初始顺序就可以
	{
		/*
		   [0]
			*
		   * *
		  *   *
		 *     *
		*       *
	[1]***********[2]
	   */
	   // 上 0 下左 1 下右 2
	   //Vector3 v1 = worldPos[0];
	   //Vector3 v2 = worldPos[1];
	   //Vector3 v3 = worldPos[2];
	   // 2D屏幕空间坐标
		float midx = arr[0].x;
		float height = arr[2].y - arr[0].y + 1.0f;
		float step1 = (midx - arr[1].x) /height;
		//int step1 = equals(midx, arr[1].x) ? 0 : (midx - arr[1].x) / height;
		float step2 = (midx - arr[2].x) / height;
		//int step2 = equals(midx, arr[2].x) ? 0 : (midx - arr[2].x) / height;
		float x1 = arr[1].x, x2 = arr[2].x;

		//printf("左 x = %f  y = %f step1 = %f; 右 x = %f  y = %f step2 = %f ；上 x = %f y = %f\n", x1, arr[1].y, step1, x2, arr[2].y, step2, arr[0].x, arr[0].y);
		// 深度值，因为都是NDC空间，所以可以直接插值
		float ndcZ_downNear = arr[1].z;
		float ndcZ_downFar = arr[2].z;
		float stepNdcZnear = (arr[0].z - ndcZ_downNear) / height; // 近处的z值
		float stepNdcZfar = (arr[0].z - ndcZ_downFar) / height;  // 远处的z值
		float fragNdcZ = 0;
		float fragNdcStepZ = 0; // 每一行x增加时候的NDC坐标步进

		// 重心坐标值
		Vector3 g;
		float zt = 0; // view空间的深度值
		//Vector3 viewPosZ = Vector3(Tri.x.w, Tri.y.w, Tri.z.w);
		Vector3 fragWorldPos;
		Vector2 fragUV;
		Vector3 fragNormal;
		for (int y = ymax; y >= ymin - 1; --y)
		{
			// z NDC 改行的深度边界
			ndcZ_downNear += stepNdcZnear;
			ndcZ_downFar += stepNdcZfar;

			// 每行对z进行步进
			fragNdcZ = ndcZ_downNear;

			fragNdcStepZ = (ndcZ_downFar - ndcZ_downNear) / (x2 - x1);
			for (float x = x1; x <= x2; x++)
			{
				fragNdcZ += fragNdcStepZ;

				// 2D屏幕求重心坐标值
				g = CenterOfGravity(Tri.x, Tri.y, Tri.z, Vector2(x, y)); // 2D重心坐标
				//printf("%f %f %f\n",g.x,g.y,g.z);
				zt = 1.0f / (g.x / viewPosZ.x + g.y / viewPosZ.y + g.z / viewPosZ.z); // view空间z值
				fragWorldPos = GravityToProperty3D(worldPos[0], worldPos[1], worldPos[2], g, viewPosZ, zt); // 世界空间坐标
				fragUV = GravityToProperty2D(uvs[0], uvs[1], uvs[2], g, viewPosZ, zt); // 但是这个也要求是有顺序的
				//Vector3 fragWorldPos = Vector3(fragWorldX, worldY_down, fragWorldZ); 暂时不需要片元的世界坐标

				//fragNormal = GravityToProperty3D(worldNormal[0], worldNormal[1], worldNormal[2], g, viewPosZ, zt).normalized(); // 但是这个也要求是原来顺序的
				//	+ tbn * normalTex->SampleLinearFilter(fragUV) * twoDivide255 - 1.0f).normalized(); // 法线扰动

				fragNormal = GravityToProperty3D(worldNormal[0], worldNormal[1], worldNormal[2], g, viewPosZ, zt).normalized(); // 但是这个也要求是原来顺序的
				if (isNormalMap)
				{
					TBN.rows[0][2] = fragNormal.x;
					TBN.rows[1][2] = fragNormal.y;
					TBN.rows[2][2] = fragNormal.z;
					Vector3 normaldist = normalTex->SampleLinearFilter(fragUV) * twoDivide255 - 1.0f; // 法线扰动
					fragNormal = (TBN * normaldist).normalized();
				}
				//fragNormal = (worldNormal[0] * g.x + worldNormal[1] * g.y + worldNormal[2] * g.z).normalized(); // 但是这个也要求是原来顺序的
				fms.push_back(Fragment(Vector2(x+0.5f, y+0.5f), fragNormal, fragWorldPos, fragNdcZ, fragUV));// 该片元的屏幕坐标、法线、世界坐标、z深度值
			}
			x1 += step1; // 左端点
			x2 += step2; // 右端点
		}
	}
	void DrawDownTriangle(Vector3 arr[3], const float& ymax, const float& ymin, vector<Fragment>& fms, Vector3 worldPos[3], Vector3& viewPosZ,Vector3 worldNormal[3],Triangle<Vector3>& Tri,Vector2 uvs[3], Matrix3x3& TBN)// 只有第一个数组需要排序，其他数据初始顺序就可以
	{
		/*
		[0]*********** [1]
		   *        *
			*      *
			 *    *
			  *  *
			   *
			   [2]
		*/
		float midx = arr[2].x;
		float height = arr[2].y - arr[0].y + 1.0f;
		//int step1 = equals(midx, arr[0].x) ? 0 : (midx - arr[0].x) / height;
		float step1 = (midx - arr[0].x) / height;
		//int step2 = equals(midx, arr[1].x) ? 0 : (midx - arr[1].x) / height;
		float step2 = (midx - arr[1].x) / height;
		float x1 = arr[0].x, x2 = arr[1].x;

		//printf("左 x = %f  y = %f step1 = %f; 右 x = %f  y = %f step2 = %f ；下 x = %f y = %f\n",x1,arr[0].y,step1, x2,arr[1].y,step2,arr[2].x,arr[2].y);
		// 深度值
		float ndcZ_downNear = arr[0].z; // 其实不用管远近，知道是从左到右的就行
		float ndcZ_downFar = arr[1].z;
		float stepNdcZnear = (arr[2].z - ndcZ_downNear) / height; // (arr[2].x - x1)
		float stepNdcZfar = (arr[2].z - ndcZ_downFar) / height;
		float fragNdcZ = 0;
		float fragNdcStepZ = 0; // 每一行x增加时候的NDC坐标步进

		// 重心坐标值
		Vector3 g;
		float zt = 0; // view空间的深度值
		//Vector3 viewPosZ = Vector3(beginningVertex[0].w,beginningVertex[1].w,beginningVertex[2].w);
		Vector3 fragWorldPos;
		Vector2 fragUV;
		Vector3 fragNormal;
		for (int y = ymin; y <= ymax + 1; ++y) // 从上到下，是从小到大
		{
			// z NDC 该行的深度边界
			ndcZ_downNear += stepNdcZnear;
			ndcZ_downFar += stepNdcZfar;

			// 每行对x和z进行步进
			fragNdcZ = ndcZ_downNear;

			fragNdcStepZ = (ndcZ_downFar - ndcZ_downNear) / (x2 - x1); // 这只是深度

			for (float x = x1; x <= x2; x++)
			{
				fragNdcZ += fragNdcStepZ;
				Vector3 fragWorldPos;
				// 2D屏幕求重心坐标值
				g = CenterOfGravity(Tri.x, Tri.y, Tri.z, Vector2(x, y)); // 2D重心坐标

				zt = 1.0f / (g.x / viewPosZ.x + g.y / viewPosZ.y + g.z / viewPosZ.z); // view空间z值
				fragWorldPos = GravityToProperty3D(worldPos[0], worldPos[1], worldPos[2], g, viewPosZ, zt); // 世界空间坐标，注意，uv、世界坐标、法线向量数组都是原来三角形的索引顺序，而不要排序
				fragUV = GravityToProperty2D(uvs[0], uvs[1], uvs[2], g, viewPosZ, zt); // 但是这个也要求是原来顺序的
				//Vector3 fragWorldPos = Vector3(fragWorldX, worldY_down, fragWorldZ);
				//fragNormal = worldNormal[0] * g.x + worldNormal[1] * g.y + worldNormal[2] * g.z; // 但是这个也要求是原来顺序的

				//fragNormal = (GravityToProperty3D(worldNormal[0], worldNormal[1], worldNormal[2], g, viewPosZ, zt) // 但是这个也要求是原来顺序的
				//	+ tbn * normalTex->SampleLinearFilter(fragUV) * twoDivide255 - 1.0f).normalized(); // 法线扰动
				//cout << worldNormal[0];
				//cout << worldNormal[1];
				//cout << worldNormal[2];
				fragNormal = GravityToProperty3D(worldNormal[0], worldNormal[1], worldNormal[2], g, viewPosZ, zt).normalized(); // 但是这个也要求是原来顺序的
				//cout << "fragNormal = " << fragNormal;
				//cout << "g = " << g;
				//printf("XXXXXXXXXXXXXX\n");
				if (isNormalMap)
				{
					TBN.rows[0][2] = fragNormal.x;
					TBN.rows[1][2] = fragNormal.y;
					TBN.rows[2][2] = fragNormal.z;
					Vector3 normaldist = normalTex->SampleLinearFilter(fragUV) * twoDivide255 - 1.0f; // 法线扰动
					fragNormal = (TBN * normaldist).normalized();
				}
				//fragNormal = (worldNormal[0] * g.x + worldNormal[1] * g.y + worldNormal[2] * g.z).normalized(); // 但是这个也要求是原来顺序的
				fms.push_back(Fragment(Vector2(x+0.5f, y+0.5f), fragNormal, fragWorldPos, fragNdcZ, fragUV));// 该片元的屏幕坐标、法线、世界坐标、z深度值
			}
			x1 += step1;
			x2 += step2;
		}
	}
	Vector3 GravityToProperty3D(Vector3& a, Vector3& b, Vector3& c, Vector3& g, Vector3& zAbc, float& zt)
	{
		Vector3 result;
		result.x = (g.x * a.x / zAbc.x + g.y * b.x / zAbc.y + g.z * c.x / zAbc.z) * zt;
		result.y = (g.x * a.y / zAbc.x + g.y * b.y / zAbc.y + g.z * c.y / zAbc.z) * zt;
		result.z = (g.x * a.z / zAbc.x + g.y * b.z / zAbc.y + g.z * c.z / zAbc.z) * zt;
		return result;
	}

	Vector2 GravityToProperty2D(Vector2& a, Vector2& b, Vector2& c, Vector3& g, Vector3& zAbc, float& zt)
	{
		Vector2 result;
		result.x = (g.x * a.x / zAbc.x + g.y * b.x / zAbc.y + g.z * c.x / zAbc.z) * zt;
		result.y = (g.x * a.y / zAbc.x + g.y * b.y / zAbc.y + g.z * c.y / zAbc.z) * zt;
		return result;
	}

	// 屏幕2D空间求重心坐标值
	Vector3 CenterOfGravity(Vector3& v1, Vector3& v2, Vector3& v3, Vector2 p)
	{
		if ((-(v1.x - v2.x) * (v3.y - v2.y) + (v1.y - v2.y) * (v3.x - v2.x)) == 0)
			return Vector3(1, 0, 0);
		if (-(v2.x - v3.x) * (v1.y - v3.y) + (v2.y - v3.y) * (v1.x - v3.x) == 0)
			return Vector3(1, 0, 0);
		float alpha = (-(p.x - v2.x) * (v3.y - v2.y) + (p.y - v2.y) * (v3.x - v2.x)) / (-(v1.x - v2.x) * (v3.y - v2.y) + (v1.y - v2.y) * (v3.x - v2.x));
		float beta = (-(p.x - v3.x) * (v1.y - v3.y) + (p.y - v3.y) * (v1.x - v3.x)) / (-(v2.x - v3.x) * (v1.y - v3.y) + (v2.y - v3.y) * (v1.x - v3.x));
		float gamma = 1 - alpha - beta;
		return Vector3(alpha, beta, gamma).clamp(0.0f, 1.0f);
	}
};



void DDA(Window* w, Vector2 P1, Vector2 P2) {
	float dx = P2.x - P1.x;
	float dy = P2.y - P1.y;

	if (dx == 0) { // 垂直线段
		int y_increment = dy > 0 ? 1 : -1; // 确定y的增量方向
		int y = static_cast<int>(P1.y);

		for (; y != static_cast<int>(P2.y); y += y_increment) {
			SET_WHITE_PIXEL(w, static_cast<int>(P1.x), y);
		}
		SET_WHITE_PIXEL(w, static_cast<int>(P2.x), static_cast<int>(P2.y));
	}
	else if (dy == 0) { // 水平线段
		int x_increment = dx > 0 ? 1 : -1; // 确定x的增量方向
		int x = static_cast<int>(P1.x);

		for (; x != static_cast<int>(P2.x); x += x_increment) {
			SET_WHITE_PIXEL(w, x, static_cast<int>(P1.y));
		}
		SET_WHITE_PIXEL(w, static_cast<int>(P2.x), static_cast<int>(P2.y));
	}
	else { // 常规线段
		float m = dy / dx;
		int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
		float x_increment = dx / steps;
		float y_increment = dy / steps;

		float x = P1.x;
		float y = P1.y;

		for (int i = 0; i < steps; i++) {
			SET_WHITE_PIXEL(w, static_cast<int>(x), static_cast<int>(y));
			x += x_increment;
			y += y_increment;
		}
	}
}

// 重心坐标插值
// 计算三角形的最小包围盒
void CalculateBoundingBox(Vector3& v0, Vector3& v1, Vector3& v2, Vector2& minPos, Vector2& maxPos) {
	float minX = min(min(v0.x, v1.x), v2.x);
	float minY = min(min(v0.y, v1.y), v2.y);
	float maxX = max(max(v0.x, v1.x), v2.x);
	float maxY = max(max(v0.y, v1.y), v2.y);

	minPos.x = std::floor(minX);
	minPos.y = std::floor(minY);
	maxPos.x = std::ceil(maxX);
	maxPos.y = std::ceil(maxY);
}

// 判断点是否在三角形内部
bool isPointInTriangle(Vector3& point, Vector3& v0, Vector3& v1, Vector3& v2) {
	// 计算三角形的面积和重心坐标
	float triangleArea = cross(v1 - v0, v2 - v0).length() / 2.0f;
	float w0 = cross(v1 - v0, point - v0).length() / triangleArea;
	float w1 = cross(v2 - v1, point - v1).length() / triangleArea;
	float w2 = 1.0f - w0 - w1;

	// 判断重心坐标是否合法
	return (w0 >= 0 && w0 <= 1) && (w1 >= 0 && w1 <= 1) && (w2 >= 0 && w2 <= 1);
}

void DrawUpTriangle(Window* w, Vector2 arr[3], const float& ymax, const float& ymin)
{
	//printf("上三角形");
	float midx = arr[0].x;
	float height = arr[2].y - arr[0].y;
	float step1 = (midx - arr[1].x) / height;
	float step2 = (arr[2].x - midx) / height;
	float x1 = arr[1].x, x2 = arr[2].x;
	//printf("step1 %f step2 %f x1 %f x2 %f ymin %f ymax %f ");
	for (int y = ymax; y > ymin; --y)
	{
		x1 += step1;
		x2 -= step2;
		for (int x = x1; x < x2; x++)
		{
			SET_WHITE_PIXEL(w, x, y);
		}
	}
}
void DrawDownTriangle(Window* w, Vector2 arr[], const float& ymax, const float& ymin)
{
	//printf("下三角形");
	float midx = arr[2].x;
	float height = arr[2].y - arr[0].y;
	float step1 = (midx - arr[0].x) / height;
	float step2 = (arr[1].x - midx) / height;
	float x1 = arr[0].x, x2 = arr[1].x;
	//printf("step1 %f step2 %f x1 %f x2 %f ymin %f ymax %f ");
	for (int y = ymin; y < ymax; ++y)
	{
		x1 += step1;
		x2 -= step2;
		for (int x = x1; x < x2; x++)
		{
			SET_WHITE_PIXEL(w, x, y);
		}
	}
}
// 纯绘制，不是写入片元
void DrawTriangle(Window* w, Triangle<Vector2>& Tri)
{
	// 对三个顶点按y坐标从小到大排序
	Vector2 arr[3] = { Tri.x , Tri.y , Tri.z };
	std::sort(arr, arr + 3, cmp2d);
	//sort(Tri.v, Tri.v + 3, [](const Vector2<T>& v1, const Vector2<T>& v2) {
	//	return v1.y < v2.y;
	//});
	float ymin = arr[0].y, ymax = arr[2].y;
	//for (int i = 0; i < 3; i++)
	//{
	//	std::cout << i << " "<< arr[i].x << "," << arr[i].y<<endl;
	//}

	if (equals(arr[0].y, arr[1].y)) // 下三角形
	{
		DrawDownTriangle(w, arr, ymax, ymin);
	}
	else if (equals(arr[1].y, arr[2].y)) // 上三角形
	{
		DrawUpTriangle(w, arr, ymax, ymin);
	}
	else // 上下都有
	{
		//printf("上下都有");
		float k = 0;
		float midx = 0, midy = arr[1].y;
		if (equals(arr[2].x, arr[0].x))
			midx = arr[2].x;
		else
		{
			k = (arr[2].y - arr[0].y) / (arr[2].x - arr[0].x);
			midx = 1 / k * (midy - arr[0].y) + arr[0].x;
		}

		Vector2 midPoint(midx, midy);
		// 上三角形
		Vector2 upArr[3] = { arr[0],arr[1],midPoint };
		Vector2 downArr[3] = { arr[1],midPoint,arr[2] };
		std::sort(upArr, upArr + 3, cmp2d);
		std::sort(downArr, downArr + 3, cmp2d);
		DrawUpTriangle(w, upArr, arr[1].y, arr[0].y);
		DrawDownTriangle(w, downArr, arr[2].y, arr[1].y);
	}
}