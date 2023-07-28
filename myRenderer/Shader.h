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
//void DrawUpTriangle(Vector3 arr[3], const float& ymax, const float& ymin, vector<Fragment>& fms, Vector3 worldPos[3], Vector3& viewPosZ, Vector3 faceNormal[3], Triangle<Vector3>& Tri, Vector2 uvs[3]);// ֻ�е�һ��������Ҫ�����������ݳ�ʼ˳��Ϳ���
//void DrawDownTriangle(Vector3 arr[3], const float& ymax, const float& ymin, vector<Fragment>& fms, Vector3 worldPos[3], Vector3& viewPosZ, Vector3 faceNormal[3], Triangle<Vector3>& Tri, Vector2 uvs[3]);// ֻ�е�һ��������Ҫ�����������ݳ�ʼ˳��Ϳ���

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
		rectPos[0] = Vector2(0, 0);//���Ͻ�
		rectPos[1] = Vector2(windowWidth, 0);//���Ͻ�
		rectPos[2] = Vector2(0, windowHeight);//���½�
		rectPos[3] = Vector2(windowWidth, windowHeight);//���½�
	};
	//Shader() {};//���ø��־���
	~Shader() {};

	Vector4* clipPosition = nullptr; // ����
	Vector2* screenPosition = nullptr; // ��Ļ
	vector<Fragment> fragments; // ƬԪ����
	bool* backCulling = nullptr;// �����޳���������
	bool* frontCulling = nullptr;// �����޳���������
	bool* frustumCulling = nullptr;// �����޳���������
	bool isNDC = false;
	bool isNormalMap = false;
	//bool isTransparent = false;
	bool initialCulling = true;

	// Ӧ����v2f�ģ�
	Vector3* worldPosition = nullptr; // ���綥������
	Vector3(*worldNormal)[3] = nullptr; // ���編�߷���ÿ����������
	Vector3* viewPosition = nullptr;
	Vector2 rectPos[4];//��Ļ������

	Camera* camera = nullptr;
	Model* m = nullptr;
	Light* light = nullptr;

	Matrix4x4* VP = nullptr;
	// ��̬�Ķ�ά���飬������
	//static Vector3(*colorBuffer)[500] = new Vector3[500][500];
	//static Vector3 colorBuffer[windowWidth][windowWidth];
	//Vector3(*colorBuffer)[windowWidth] = new Vector3[windowWidth][windowHeight];

	Vector3(*fxaaBuffer)[windowWidth] = new Vector3[windowWidth][windowHeight];
	//Matrix4x4 modelMat;
	//Matrix4x4 viewMat;
	//Matrix4x4 projectMat;

	// Ӧ�ô���һ��phong shader�еĲ���
	Vector3 shadowColor = Vector3(50,50,50);
	Vector3 c_d = Vector3(200, 250, 100);
	Texture* tex = nullptr;
	Vector3 c_s;
	float gloss;
	Texture* normalTex = nullptr;// ������ͼ

	float _RelativeThreshold = 0.6f;
	float _ContrastThreshold = 0.6f;

	bool backcull = true;
	bool frontcull = false;
	struct a2v
	{
		Vector4* Position = nullptr; // ģ�Ϳռ�����
		unsigned int num = 0; // ��������
	};
	struct v2f
	{
		// ����ռ�����
	};
	string GetFileExtension(const std::string& filePath) 
	{
		size_t dotPos = filePath.find_last_of(".");
		if (dotPos != std::string::npos && dotPos < filePath.length() - 1) {
			return filePath.substr(dotPos + 1);
		}
		return "";
	}
	//// ����Ϊ͸��(���)������Ȳ���ʱ�ر����д��
	//void SetTransparent()
	//{
	//	isTransparent = true;
	//}
	void SetParameter(Camera* c, Model& m, Light* l, const char* path, Vector3 c_s, Vector3 c_d, float gloss, bool bc = false/*�����޳�*/, Matrix4x4* vp = nullptr/*�������ͶӰ����*/, const char* normalPath = nullptr,bool fc = false)
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
		// ʹ�� std::fill() ����������Ԫ�ظ�ֵΪ -100.0f
		std::fill(reinterpret_cast<float*>(c->zBuffer), reinterpret_cast<float*>(c->zBuffer) + windowHeight * windowWidth, -100.0f);
		//memset(c->zBuffer, -100.0f, windowHeight * windowWidth * sizeof(float));
		//std::fill(colorBuffer, colorBuffer + windowWidth * windowHeight, Vector3(0,0,0));
		// ����ά����ת��Ϊһά���鲢���
		std::fill(reinterpret_cast<Vector3*>(colorBuffer), reinterpret_cast<Vector3*>(colorBuffer) + windowHeight * windowWidth, Vector3(0, 0, 0));
	}
	virtual void ClearFragment()
	{
		fragments.clear();
	}
	virtual void VertexShader()// ģ�Ϳռ� ���� �ü��ռ���������
	{
		int vexNum = m->veretexNum;// ��������
		int triNum = m->triangleNum;// ������

		// MVP�任���ü��ռ�
		if (clipPosition == nullptr)
		{
			//int size = sizeof(input.Position) / sizeof(Vector4);
			//unsigned int size = input.num;
			worldPosition = new Vector3[vexNum]; // ����ƬԪ��ֵ�õ���������
			clipPosition = new Vector4[vexNum];  // ���ڼ�����Ļ����λ��+��Ȼ���
			viewPosition = new Vector3[vexNum];  // ����͸��ͶӰ��ֵ����
			for (int i = 0; i < vexNum; ++i)
			{
				//Vector4 model = *camera->modelMat * m.vertex[i];
				Vector4 model = *m->modelMat * m->vertexs[i];
				Vector4 view = *camera->viewMat * model;
				clipPosition[i] = *camera->projectMat * view;//������͸��ͶӰ
				//clipPosition[i] = *camera->perspectMat * view;

				worldPosition[i] = model.ToVector3();

				viewPosition[i] = view.ToVector3();
				//*camera->modelMat * *camera->viewMat * *camera->MVP

				// ��Դλ�õ�NDC�ռ�
				//lightSpace2dz[i] = *camera->VP * worldPosition[i];
				//lightSpace2dz[i] = lightSpace2dz[i] * 0.5f + Vector3(0.5f,0.5f,0.5f);
			}
		}
		// ��ʼ���޳�����
		if (initialCulling)
		{
			// ����������޳�
			backCulling = new bool[triNum];
			frontCulling = new bool[triNum];
			// ��׶���޳�������
			frustumCulling = new bool[triNum];
			memset(frustumCulling, false, triNum * sizeof(bool));//ȫ���رգ�ÿ֡����
			memset(backCulling, false, triNum * sizeof(bool));
			memset(frontCulling, false, triNum * sizeof(bool));


			//��һ�μ��㱳��������޳�
			// �����޳�����
			if (backcull)
			{
				//backCulling = new bool[triNum];
				memset(backCulling, false, triNum * sizeof(bool)); //ȫ�����޳�
				if (backcull)
				{
					for (uint32_t i = 0; i < triNum; ++i)
					{
						Vector3 n = cross((worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x])).normalized();
						Vector3 viewDir = (worldPosition[(uint32_t)m->face[i].x] - camera->pos).normalized(); // ����һ����
						if (dot(n, viewDir) + 0.00001f >= 0) // >= 0���ɼ�
						{
							backCulling[i] = true;
						}
					}
				}
			}
			// �����޳�����
			else if (frontcull)
			{
				//frontCulling = new bool[triNum];
				memset(frontCulling, false, triNum * sizeof(bool)); //ȫ�����޳�
				if (frontcull)
				{
					for (uint32_t i = 0; i < triNum; ++i)
					{
						Vector3 n = cross((worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x])).normalized();
						Vector3 viewDir = (worldPosition[(uint32_t)m->face[i].x] - camera->pos).normalized(); // ����һ����
						if (dot(n, viewDir) + 0.00001f < 0) // �ɼ��޳�
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
			m->TBN = new Matrix3x3[triNum][3]; // ÿ��������Ƭ������TBN����
			worldNormal = new Vector3[triNum][3];
			if (normalTex == nullptr)//û�з�����ͼ��Ĭ��TBN����Ϊ��λ����Ϊ���߲���Ҫ�任
			{
				// ������ռ䷨�߷���ÿ�����Ӧ�������㣬һ����������ж������
				for (int j = 0; j < triNum; ++j)
					for (int k = 0; k < 3; ++k)
					{
						//worldNormal[j][k] = m->TBN[j][k] * (normalTex->SampleLinearFilter(m->triUvs[j][k]) * twoDivide255 - 1.0f); // û�е��ӵģ�ֱ�����Ŷ���ֵ
						worldNormal[j][k] = (*m->normalToWorldMat * m->triNormals[j][k]).normalized();
						//printf("%d %d : %f %f %f\n",j,k, worldNormal[j][k].x, worldNormal[j][k].y, worldNormal[j][k].z);
					}
			}
			else
			{
				// ����ÿ�����Ӧ��TBN����(3x3)
				Vector3 E1, E2, n, t, b;
				Vector2 deltaE1, deltaE2;
				float deno = 0.0f;
				for (uint32_t i = 0; i < triNum; i++)
				{
					//m->TBN[i][0] = Matrix3x3(Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
					//m->TBN[i][1] = Matrix3x3(Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));
					//m->TBN[i][2] = Matrix3x3(Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(0, 1, 0));

					// 2��TBN����
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
					// ���Խ���ʩ��������������ʱ�Ȳ���
					m->TBN[i][2] = Matrix3x3(Vector3(t.x,b.x,n.x), Vector3(t.y, b.y, n.y), Vector3(t.z, b.z, n.z));
					//m->TBN[i][0] = Matrix3x3(t, n, b);

					// 1��TBN����
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
					// ���Խ���ʩ��������������ʱ�Ȳ���
					//m->TBN[i][1] = Matrix3x3(t, n, b);
					m->TBN[i][1] = Matrix3x3(Vector3(t.x, b.x, n.x), Vector3(t.y, b.y, n.y), Vector3(t.z, b.z, n.z));

					// 0��TBN����
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
					// ���Խ���ʩ��������������ʱ�Ȳ���
					//m->TBN[i][2] = Matrix3x3(t, n, b);
					m->TBN[i][0] = Matrix3x3(Vector3(t.x, b.x, n.x), Vector3(t.y, b.y, n.y), Vector3(t.z, b.z, n.z));

					// ���з�����ͼ����
					for (int k = 0; k < 3; ++k)
					{
						//Vector3 sp = normalTex->SampleLinearFilter(m->triUvs[i][k]);
						//sp = sp * twoDivide255 - 1.0f;
						//worldNormal[i][k] = (m->TBN[i][k] * sp).normalized(); // û�е��ӵģ�ֱ�����Ŷ���ֵ
						//n = (*m->normalToWorldMat * m->triNormals[i][k]).normalized();
						//if (!equals(worldNormal[i][k],n))
						//{
						//	cout << worldNormal[i][k];
						//	cout << n;
						//	printf("xxxxxxx\n");
						//}
						//worldNormal[i][k] = (*m->normalToWorldMat * m->triNormals[i][k] + m->TBN[i][k] * (normalTex->SampleLinearFilter(m->triUvs[i][k]) * twoDivide255 - 1.0f)).normalized(); // û�е��ӵģ�ֱ�����Ŷ���ֵ
						worldNormal[i][k] = (*m->normalToWorldMat * m->triNormals[i][k]).normalized();
						//printf("%d %d : %f %f %f\n",j,k, worldNormal[j][k].x, worldNormal[j][k].y, worldNormal[j][k].z);
					}
				}
			}
		}
		if (camera->view_update)  // ��������Ѹ���
		{
			// ����view����
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
				// ���¼���һ���Ƿ��޳�
				memset(backCulling, false, triNum * sizeof(bool));
				for (uint32_t i = 0; i < triNum; ++i)
				{
					Vector3 n = cross((worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x])).normalized();
					//Vector3 n = cross((worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x])).normalized();
					Vector3 viewDir = (worldPosition[(uint32_t)m->face[i].x] - camera->pos).normalized(); // ����һ����
					if (dot(n, viewDir) + 0.00001f >= 0) //���ɼ�
					{
						backCulling[i] = true;
					}
				}
			}
			// �����޳�
			if (frontcull)
			{
				// ���¼���һ���Ƿ��޳�
				memset(frontCulling, false, triNum * sizeof(bool));
				for (uint32_t i = 0; i < triNum; ++i)
				{
					Vector3 n = cross((worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x])).normalized();
					//Vector3 n = cross((worldPosition[(uint32_t)m->face[i].y] - worldPosition[(uint32_t)m->face[i].x]), (worldPosition[(uint32_t)m->face[i].z] - worldPosition[(uint32_t)m->face[i].x])).normalized();
					Vector3 viewDir = (worldPosition[(uint32_t)m->face[i].x] - camera->pos).normalized(); // ����һ����
					if (dot(n, viewDir) + 0.00001f < 0) // �ɼ�
					{
						frontCulling[i] = true;
					}
				}
			}
			// ��׶���޳���Ĭ�Ͽ���������ȫ��false����clip2NDC�׶β����ж�
			memset(frustumCulling, false, triNum * sizeof(bool));
		}
	}

	// �ü��ռ䵽NDC�ռ䣬�ٵ���Ļ�ռ� �Զ�����
	void ClipToNDCToScreen()
	{
		int triNum = m->triangleNum;
		if (!isNDC)
		{
			uint32_t idx[3];
			int vaild = 0, tooNear = 0;
			float w;
			m->triangles = new Triangle<Vector3>[triNum];
			for (int i = 0; i < triNum; ++i) // 12�������Σ���������任���洢ƬԪ��2D��������
			{
				if (backCulling[i] || frustumCulling[i] || frontCulling[i]) // �޳�
					continue;
				idx[0] = (uint32_t)m->face[i][0];
				idx[1] = (uint32_t)m->face[i][1];
				idx[2] = (uint32_t)m->face[i][2];
				// һ�������ε���������
				vaild = 0;
				tooNear = 0;
				for (int j = 0; j < 3; ++j)
				{
					w = clipPosition[idx[j]].w + 0.0001f;
					m->triangles[i][j].x = ((clipPosition[idx[j]].x / w) * 0.5f + 0.5f) * windowWidth; // ��i�������Σ���Ӧ�Ķ��������
					m->triangles[i][j].y = ((clipPosition[idx[j]].y / w) * 0.5f + 0.5f) * windowHeight;
					// ��Ȼ���
					// camera->zBuffer[(int)screenPosition[i].x][(int)screenPosition[i].y] = clipPosition[i].z / clipPosition[i].w * 0.5f + 0.5f; // �������1���������0
					// m.triangles[i].vertexDepth[j] = clipPosition[(uint16_t)m.face[i][j]].z / clipPosition[(uint16_t)m.face[i][j]].w * 0.5f + 0.5f; // �������1���������0
					m->triangles[i][j].z = clipPosition[idx[j]].z / w; // �������1���������0
					//printf("%d x = %.f  | y = %.f ��Ӧ���� %d\n", j, m->triangles[i][j].x, m->triangles[i][j].y, (uint16_t)m->face[i][j]);
					//printf("%d z = %.5f ��Ӧ���� %d\n", j, m->triangles[i][j].z, (uint16_t)m->face[i][j]);

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
				//���һ�������ε��������㶼�����棬Ҳ���ǲ���[0,1]��Χ�ڣ����޳�
				//if (vaild == 3)
				//{
				//	printf("�ü�%d\n", i);
				//	frustumCulling[i] = true;
				//}
				//���һ�������ε��������㶼�����棬Ҳ���ǲ���[0,1]��Χ�ڣ����޳�
				if (vaild == 3)
				{
					int i = 0;
					//�����жϣ��ĸ��߽綥����Ҫ����������������ڲ�
					for (int i = 0 ; i < 4; i++)
					{
						// ��һ�����ڲ�
						if (PointInTriangle(rectPos[i],m->triangles[i][0].toVector2(),m->triangles[i][1].toVector2(), m->triangles[i][2].toVector2()))
						{
							break;
						}
					}
					if (i == 4)
					{
						printf("�ü�%d\n", i);
						frustumCulling[i] = true;
					}
				}
				// ����̫����
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
			for (int i = 0; i < triNum; ++i) // 12�������Σ���������任���洢ƬԪ��2D��������
			{
				if (backCulling[i] || frustumCulling[i] || frontCulling[i]) // �޳�
					continue;
				idx[0] = (uint32_t)m->face[i][0];
				idx[1] = (uint32_t)m->face[i][1];
				idx[2] = (uint32_t)m->face[i][2];
				vaild = 0;
				tooNear = 0;
				// һ�������ε���������
				for (int j = 0; j < 3; ++j)
				{
					w = clipPosition[idx[j]].w + 0.0001f;
					m->triangles[i][j].x = ((clipPosition[idx[j]].x / w) * 0.5f + 0.5f) * windowWidth; // ��i�������Σ���Ӧ�Ķ��������
					m->triangles[i][j].y = ((clipPosition[idx[j]].y / w) * 0.5f + 0.5f) * windowHeight;
					// ��Ȼ���
					// camera->zBuffer[(int)screenPosition[i].x][(int)screenPosition[i].y] = clipPosition[i].z / clipPosition[i].w * 0.5f + 0.5f; // �������1���������0
					// m.triangles[i].vertexDepth[j] = clipPosition[(uint16_t)m.face[i][j]].z / clipPosition[(uint16_t)m.face[i][j]].w * 0.5f + 0.5f; // �������1���������0
					m->triangles[i][j].z = clipPosition[idx[j]].z / w; // �������1���������0
					//printf("%d x = %.f  | y = %.f ��Ӧ���� %d\n", j, m.triangles[i][j].x, m.triangles[i][j].y, (uint16_t)m.face[i][j]);
					//printf("%d z = %.5f ��Ӧ���� %d\n", j, m.triangles[i][j].z, (uint16_t)m.face[i][j]);
					//printf("%d %d z=%f\n", i, j, m->triangles[i][j].z);

					//printf("%d %d z=%f\n", i, j, m->triangles[i][j].z);
					//if (m->triangles[i][j].x < 0 && m->triangles[i][j].x > 1.0f && m->triangles[i][j].y < 0 && m->triangles[i][j].y > 1.0f && m->triangles[i][j].z < 0 && m->triangles[i][j].z > 1.0f)
				//printf("XXXXX tri = %d %f,%f,%f XXXXX\n", i,m->triangles[i][j].x, m->triangles[i][j].y, m->triangles[i][j].z);
				if ((m->triangles[i][j].x < 0 || m->triangles[i][j].x > windowWidth) || (m->triangles[i][j].y < 0 || m->triangles[i][j].y > windowHeight) || (m->triangles[i][j].z > 1.103679f)/*̫����*/)
				{
					//printf("XXXXX %f,%f,%f XXXXX\n", m->triangles[i][j].x, m->triangles[i][j].y, m->triangles[i][j].z);
					vaild++;
				}
				if (m->triangles[i][j].z > 1.055f || m->triangles[i][j].z < 0.2f)
				{
					tooNear++;
				}
				}
				//���һ�������ε��������㶼�����棬Ҳ���ǲ���[0,1]��Χ�ڣ����޳�
				if (vaild == 3)
				{
					int i = 0;
					//�����жϣ��ĸ��߽綥����Ҫ����������������ڲ�
					for (int i = 0; i < 4; i++)
					{
						// ��һ�����ڲ�
						if (PointInTriangle(rectPos[i], m->triangles[i][0].toVector2(), m->triangles[i][1].toVector2(), m->triangles[i][2].toVector2()))
						{
							break;
						}
					}
					if (i == 4)
					{
						printf("�ü�%d\n", i);
						frustumCulling[i] = true;
					}
				}
				// ����̫����
				if (tooNear == 3)
				{
					frustumCulling[i] = true;
				}
			}
		}
	}
	// �ж�p�Ƿ��ڶ�ά�ռ����������ڲ�
	bool PointInTriangle(Vector2 &P, Vector2 A, Vector2 B, Vector2 C) {
		Vector2 AB = B - A;
		Vector2 AP = P - A;

		Vector2 BC = C - B;
		Vector2 BP = P - B;

		Vector2 CA = A - C;
		Vector2 CP = P - C;

		// �����������
		double cp1 = crossProduct(AB, AP);
		double cp2 = crossProduct(BC, BP);
		double cp3 = crossProduct(CA, CP);

		// ������������������ͬ�ķ��ţ�������������ڲ�
		return (cp1 >= 0 && cp2 >= 0 && cp3 >= 0) || (cp1 <= 0 && cp2 <= 0 && cp3 <= 0);
	}
	// ��դ���׶� ��GPU
	virtual void Rasterization()
	{
		int triNum = m->triangleNum;
		//int num = 12;
		// ��ÿ�������ν��б������õ����е�ƬԪ�����Ҳ�ֵ���õ�����ƬԪ��2D��Ļλ�ã����Ҳ�ֵ�õ�����ɫ�����ߵȵȣ�������
		uint32_t a_idx, b_idx, c_idx;
		//uint32_t a_uv_idx, b_uv_idx, c_uv_idx;
		Vector3 worldPos[3];
		//Vector3 faceWorldNormal = m->triNormals[i][0]; // ����ֵ���ߣ�ֱ��ȡ����һ����
		//frustumCulling[0] = false;
		for (int i = 0; i < triNum; ++i)// ��������޳�����ô��Ӧ�ı��������ξ�ֱ�ӹ�
		{
			if (backCulling[i] || frustumCulling[i] || frontCulling[i]) // �޳�
				continue;
			a_idx = (uint32_t)m->face[i][0];
			b_idx = (uint32_t)m->face[i][1];
			c_idx = (uint32_t)m->face[i][2];

			//a_uv_idx = (uint32_t)m->face_uv[i][0];
			//b_uv_idx = (uint32_t)m->face_uv[i][1];
			//c_uv_idx = (uint32_t)m->face_uv[i][2];
			// ����ȡ����ߡ�λ�á�uv���в�ֵ
			worldPos[0] = worldPosition[a_idx];
			worldPos[1] = worldPosition[b_idx];
			worldPos[2] = worldPosition[c_idx];

			Vector3 viewPosZ = Vector3(viewPosition[a_idx].z, viewPosition[b_idx].z, viewPosition[c_idx].z);
			Vector2 uvs[3]{ m->triUvs[i][0],m->triUvs[i][1] ,m->triUvs[i][2] };
			//int before = fragments.size();
			
			DrawTriangle(m->triangles[i], fragments, worldPos, worldNormal[i], viewPosZ, uvs, m->TBN[i][0]); // һ����push_back�����е㲻̫��Ч��
			//printf("������%d ����%d\n", i,fragments.size() - before);
		}
	}

	virtual void FragmentShader() // �����γ���
	{
		//fill(colorBuffer, colorBuffer + windowWidth * windowHeight, Vector3(0,0,0));
		// ���ƬԪ������ɫ
		//Vector3 tmpColor;
		// ���ƬԪ������ɫ
		//Vector3 worldLightDir = Vector3(0,0,0); // ������ƽ�й���ߵ��Դ

		//if (typeid(*light) == typeid(PointLight)) {
			//PointLight* pl = dynamic_cast<PointLight*>(light);
			//// ���� PointLight �ĳ�Ա
			////Vector3 pos = pl->pos;
			//Vector3 lightDir = Vector3();
		Vector3 viewDir = Vector3();
		//}
		//else if (typeid(*light) == typeid(DirectionLight)) {
		DirectionLight* dl = dynamic_cast<DirectionLight*>(light);
		//	 //���� DirectionLight �ĳ�Ա
		Vector3 lightDir = (-dl->dir).normalized();// �󷴷���
	//}

		Vector3 half = Vector3();
		Vector3 reflect = Vector3();
		Vector3 n = Vector3();
		int fragNum = fragments.size();
		for (uint32_t j = 0; j < fragNum; ++j)
		{
			//fragments[j].color = Vector3(200,200,200);
			//continue;

			// ��Ӱ�ļ���
			//if (FragmentInShadow(fragments[j].m_worldPos)) // ������Ӱ�ĵط�
			//{
			//	fragments[j].color = shadowColor;
			//	continue;
			//}

			//fragments[j].color = normalTex->SampleLinearFilter(fragments[j].uv) * twoDivide255 - 1.0f;
			//cout << fragments[j].color;
			//continue;

			//n = (fragments[j].normal + m->TBN[j] * normalTex->SampleLinearFilter(fragments[j].uv) * twoDivide255 - 1.0f).normalized(); // �����Ŷ�
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
	// ������ͼ������ɫӳ�䵽[-1,1]��Χ  [0,255] -> [-1,1]
	Vector3 NormalTexToNormal(Vector3 color)
	{
		color = color * oneDivide255 * 2.0f - 1.0f; // [0,1]
		//color = color * 2.0f - 1.0f;  // [-1,1]
	}
	bool FragmentInShadow(Vector3 &worldPos)
	{
		//Vector3 ts = Vector3(-2, 0, 2);
		Vector4 w2l = *VP * ToVector4(worldPos);//��Դ�������Ļ�ռ�
		//w2l = w2l * 0.5f + Vector3(0.5f,0.5f,0.5f);
		w2l = w2l * 0.5f + Vector4(0.5f, 0.5f, 0.5f, 0.5f);//�任��[0,1]��Χ��
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
	Vector3 PhongModel(Vector3& n, Vector3& l, Vector3& r, Vector2& uv) // ��Ӧ�÷�װΪһ��PhongShader��
	{
		//Vector3 tx = this->tex->SampleLinearFilter(uv);
		Vector3 diff = light->color * this->c_d * max(0, dot(n, l));
		Vector3 spec = light->color * this->c_s * pow(max(0, dot(n, r)), this->gloss);
		return diff + spec;
	}
	Vector3 BlinnPhongModel(Vector3& n, Vector3& l, Vector3& half, Vector2& uv) // ��Ӧ�÷�װΪһ��PhongShader��
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
		// ���ƬԪ���ԣ�����ֻ������Ȳ��ԣ������Ȳ����ԣ��о�ֱ������ɫ
		int fragNum = fragments.size();
		//auto zBuffer = camera->zBuffer;
		for (int j = 0; j < fragNum; ++j)
		{
			//Vector3 w2l = *VP * fragments[j].m_worldPos;//��Դ�������Ļ�ռ�
			//w2l = w2l * 0.5f + Vector3(0.5f, 0.5f, 0.5f);//�任��[0,1]��Χ��

			//int x = w2l.x * windowWidth, y = w2l.y * windowHeight;
			int x = fragments[j].m_x, y = fragments[j].m_y;
			//printf("%d %d %f %f\n",x,y, fragments[j].m_depth, camera->zBuffer[x][y]);
			//if (fragments[j].color.x > this->colorBuffer[x][y].x)
			//{
			//	this->colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // д����ɫ����
			//}

			//// �Ȳ����Ը�����ɫ����
			if (0 <= x && x < windowWidth && 0 <= y && y < windowHeight)
			{
				//colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // д����ɫ����
				if (fragments[j].m_depth > camera->zBuffer[x][y]) // ͨ�����ԣ������еıȽ�С����ʱ�����������������ƣ�������
				{
					camera->zBuffer[x][y] = fragments[j].m_depth; // д�����ֵ
					colorBuffer[x][y] = fragments[j].color;//fragments[j].color; // д����ɫ����
				}
			}
		}
	}
	// ������ݣ��洢������µ���ɫ������
	void FXAA()
	{
		Vector2 uv, uvNW, uvNE, uvSW, uvSE;
		float M, NW, NE, SW, SE;
		for (uint32_t i = 0; i < windowWidth; ++i)
		{
			for (uint32_t j = 0; j < windowHeight; ++j)
			{
				//uv = Vector2(i,j);//��ǰ���ص�uv���꣬����
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


				// �ԱȶȲ��㣬��ʾ���ڲ�
				if (Contrast < max(0.5f, MaxLuma * 0.5f))
				{
					fxaaBuffer[i][j] = colorBuffer[i][j]; // ���Ƽ���
					continue;
				}
				//// ���򣬼������߷���
				NE += 1.0f / 384.0f;
				Vector2 dir1 = Vector2(-((NW + NE) - (SW + SE)), (NE + SE) - (NW + SW)).normalized();
				Vector3 N1 = SampleInColorBuffer(i + dir1.x, j + dir1.y);
				Vector3 P1 = SampleInColorBuffer(i - dir1.x, j - dir1.y);
				//Vector2 pdir1 = uv + dir1;
				//Vector2 ndir1 = uv - dir1;

				//fxaaBuffer[i][j] = (N1+P1)*0.5f; 
				Vector3 result = (N1 + P1) * 0.5f;

				// ˮƽor����ĵ���
				const float _Sharpness = 8;
				float dirAbsMinTimesC = min(abs(dir1.x), abs(dir1.y)) * _Sharpness;
				Vector2 dir2 = Vector2(Clamp(dir1.x / dirAbsMinTimesC, -2.0, 2.0), Clamp(dir1.y / dirAbsMinTimesC, -2.0, 2.0)) * 2.0f;
				Vector3 N2 = SampleInColorBuffer(i - dir2.x * 0.5f, i - dir2.y * 0.5f);
				Vector3 P2 = SampleInColorBuffer(i + dir2.x * 0.5f, i + dir2.y * 0.5f);
				Vector3 result2 = result * 0.5f + (N2 + P2) * 0.25f;
				// ����µĽ������������ȷ��Χ�ڣ���ʹ���µĽ��
				float NewLum = Luminance(result2);
				if ((NewLum >= MinLuma) && (NewLum <= MaxLuma)) {
					result = result2;
				}

				fxaaBuffer[i][j] = result;
			}
		}
	}
	// ˫���Բ�ֵ����colorBuffer
	Vector3 SampleInColorBuffer(float x, float y) // ����xyֵ������ֵ��(0.5,0.5) (498.5,498.5)
	{
		float u_t = Clamp(0.5f, 498.5f, x);
		float v_t = Clamp(0.5f, 498.5f, y);

		int u_0 = (int)u_t; // ��
		int v_0 = (int)v_t; // ��

		float a = u_t - u_0;
		float b = v_t - v_0;

		//float uv00 = (1 - a) * (1 - b); // ���½�Ȩ��
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
	// ˫���Բ�ֵ����colorBuffer
	float SampleInDepthBuffer(float &x, float &y) // ����xyֵ�Ǹ����� [0,1]
	{
		// ���Բ����ƣ���ѭ������
		x = Clamp(0, 1.0f, x);
		y = Clamp(0, 1.0f, y);
		float u_t = x * windowWidth;
		float v_t = y * windowHeight;

		int u_0 = (int)u_t; // ��
		int v_0 = (int)v_t; // ��

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
		// ���������㰴y�����С��������
		Vector3 arr[3] = { Tri.x, Tri.y, Tri.z }; // ��Ļ���� + z
		//Vector2 worldPos[3] = { Tri.x , Tri.y , Tri.z };
		std::sort(arr, arr + 3, cmp2dz);
		//std::sort(worldPos, worldPos + 3, cmp3d);

		// ��arr��uvs�����Ԫ����ϳ�Vertex���͵Ķ���
		//VertexData TriVertices[3];
		//for (int i = 0; i < 3; i++) 
		//{
		//	TriVertices[i] = VertexData(arr[i],uvs[i],worldPos[i],viewPosZ[i]); // ������ȫ�����ԣ����ǲ�ֵ��Ҫ�ģ���Ӧ����Ļ��������˳����������һ��Ȼ������
		//}

		float ymin = arr[0].y, ymax = arr[2].y;

		if (equals(arr[0].y, arr[1].y)) // ��������
		{
			DrawDownTriangle(arr, ymax, ymin, fms, worldPos, viewPosZ, worldNormal, Tri, uvs, TBN);
		}
		else if (equals(arr[1].y, arr[2].y)) // ��������
		{
			//printf("������");
			DrawUpTriangle(arr, ymax, ymin, fms, worldPos, viewPosZ, worldNormal, Tri, uvs, TBN);
		}
		//else if (equals3(arr[0].y, arr[1].y, arr[2].y,0.00001f))
		//{
		//	return;
		//}
		else // ���¶���
		{
			//printf("���¶���");
			float k = 0;
			Vector3 midPoint(0, arr[1].y, 1); // ���ԣ��������������view�ռ�����ֵ��������û���õ�

			if (equals(arr[2].x, arr[0].x))
				midPoint.x = arr[2].x;
			else
			{
				k = (arr[2].y - arr[0].y) / (arr[2].x - arr[0].x);
				midPoint.x = 1.0f / k * (midPoint.y - arr[0].y) + arr[0].x;
			}
			// ��z���ֵ
			//if (equals(arr[2].z, arr[0].z), 0.000001f)
			//	midPoint.z = arr[2].z;
			//else
			//{
			//	k = (arr[2].y - arr[0].y) / (arr[2].z - arr[0].z);
			//	midPoint.z = 1.0f / k * (midPoint.y - arr[0].y) + arr[2].z;
			//}
			if (equals(arr[2].z, arr[0].z))//û����ȱ仯
			{
				midPoint.z = arr[0].z;
			}
			else
			{
				k = (arr[0].z - arr[2].z) / (arr[0].y - arr[2].y);
				midPoint.z = k * (midPoint.y - arr[2].y) + arr[2].z;
			}

			// ����������
			Vector3 upArr[3] = { arr[0],midPoint,arr[1] };
			Vector3 downArr[3] = { arr[1],arr[2],midPoint };

			std::sort(upArr, upArr + 3, cmp2dz);
			std::sort(downArr, downArr + 3, cmp2dz);

			// ������ ��������
			//Vector3 tmp = worldPos[2];
			//worldPos[2] = worldMidPoint;
			//std::sort(worldPos, worldPos + 3, cmp3d);
			DrawUpTriangle(upArr, arr[1].y, arr[0].y, fms, worldPos, viewPosZ, worldNormal, Tri, uvs, TBN);

			// ������ ��������
			//worldPos[2] = tmp;
			//worldPos[1] = worldMidPoint;
			//std::sort(worldPos, worldPos + 3, cmp3d);
			DrawDownTriangle(downArr, arr[2].y, arr[1].y, fms, worldPos, viewPosZ, worldNormal, Tri, uvs, TBN);
		}
	}

	void DrawUpTriangle(Vector3 arr[3], const float& ymax, const float& ymin, vector<Fragment>& fms, Vector3 worldPos[3], Vector3& viewPosZ, Vector3 worldNormal[3], Triangle<Vector3>& Tri, Vector2 uvs[3], Matrix3x3& TBN)// ֻ�е�һ��������Ҫ�����������ݳ�ʼ˳��Ϳ���
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
	   // �� 0 ���� 1 ���� 2
	   //Vector3 v1 = worldPos[0];
	   //Vector3 v2 = worldPos[1];
	   //Vector3 v3 = worldPos[2];
	   // 2D��Ļ�ռ�����
		float midx = arr[0].x;
		float height = arr[2].y - arr[0].y + 1.0f;
		float step1 = (midx - arr[1].x) /height;
		//int step1 = equals(midx, arr[1].x) ? 0 : (midx - arr[1].x) / height;
		float step2 = (midx - arr[2].x) / height;
		//int step2 = equals(midx, arr[2].x) ? 0 : (midx - arr[2].x) / height;
		float x1 = arr[1].x, x2 = arr[2].x;

		//printf("�� x = %f  y = %f step1 = %f; �� x = %f  y = %f step2 = %f ���� x = %f y = %f\n", x1, arr[1].y, step1, x2, arr[2].y, step2, arr[0].x, arr[0].y);
		// ���ֵ����Ϊ����NDC�ռ䣬���Կ���ֱ�Ӳ�ֵ
		float ndcZ_downNear = arr[1].z;
		float ndcZ_downFar = arr[2].z;
		float stepNdcZnear = (arr[0].z - ndcZ_downNear) / height; // ������zֵ
		float stepNdcZfar = (arr[0].z - ndcZ_downFar) / height;  // Զ����zֵ
		float fragNdcZ = 0;
		float fragNdcStepZ = 0; // ÿһ��x����ʱ���NDC���경��

		// ��������ֵ
		Vector3 g;
		float zt = 0; // view�ռ�����ֵ
		//Vector3 viewPosZ = Vector3(Tri.x.w, Tri.y.w, Tri.z.w);
		Vector3 fragWorldPos;
		Vector2 fragUV;
		Vector3 fragNormal;
		for (int y = ymax; y >= ymin - 1; --y)
		{
			// z NDC ���е���ȱ߽�
			ndcZ_downNear += stepNdcZnear;
			ndcZ_downFar += stepNdcZfar;

			// ÿ�ж�z���в���
			fragNdcZ = ndcZ_downNear;

			fragNdcStepZ = (ndcZ_downFar - ndcZ_downNear) / (x2 - x1);
			for (float x = x1; x <= x2; x++)
			{
				fragNdcZ += fragNdcStepZ;

				// 2D��Ļ����������ֵ
				g = CenterOfGravity(Tri.x, Tri.y, Tri.z, Vector2(x, y)); // 2D��������
				//printf("%f %f %f\n",g.x,g.y,g.z);
				zt = 1.0f / (g.x / viewPosZ.x + g.y / viewPosZ.y + g.z / viewPosZ.z); // view�ռ�zֵ
				fragWorldPos = GravityToProperty3D(worldPos[0], worldPos[1], worldPos[2], g, viewPosZ, zt); // ����ռ�����
				fragUV = GravityToProperty2D(uvs[0], uvs[1], uvs[2], g, viewPosZ, zt); // �������ҲҪ������˳���
				//Vector3 fragWorldPos = Vector3(fragWorldX, worldY_down, fragWorldZ); ��ʱ����ҪƬԪ����������

				//fragNormal = GravityToProperty3D(worldNormal[0], worldNormal[1], worldNormal[2], g, viewPosZ, zt).normalized(); // �������ҲҪ����ԭ��˳���
				//	+ tbn * normalTex->SampleLinearFilter(fragUV) * twoDivide255 - 1.0f).normalized(); // �����Ŷ�

				fragNormal = GravityToProperty3D(worldNormal[0], worldNormal[1], worldNormal[2], g, viewPosZ, zt).normalized(); // �������ҲҪ����ԭ��˳���
				if (isNormalMap)
				{
					TBN.rows[0][2] = fragNormal.x;
					TBN.rows[1][2] = fragNormal.y;
					TBN.rows[2][2] = fragNormal.z;
					Vector3 normaldist = normalTex->SampleLinearFilter(fragUV) * twoDivide255 - 1.0f; // �����Ŷ�
					fragNormal = (TBN * normaldist).normalized();
				}
				//fragNormal = (worldNormal[0] * g.x + worldNormal[1] * g.y + worldNormal[2] * g.z).normalized(); // �������ҲҪ����ԭ��˳���
				fms.push_back(Fragment(Vector2(x+0.5f, y+0.5f), fragNormal, fragWorldPos, fragNdcZ, fragUV));// ��ƬԪ����Ļ���ꡢ���ߡ��������ꡢz���ֵ
			}
			x1 += step1; // ��˵�
			x2 += step2; // �Ҷ˵�
		}
	}
	void DrawDownTriangle(Vector3 arr[3], const float& ymax, const float& ymin, vector<Fragment>& fms, Vector3 worldPos[3], Vector3& viewPosZ,Vector3 worldNormal[3],Triangle<Vector3>& Tri,Vector2 uvs[3], Matrix3x3& TBN)// ֻ�е�һ��������Ҫ�����������ݳ�ʼ˳��Ϳ���
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

		//printf("�� x = %f  y = %f step1 = %f; �� x = %f  y = %f step2 = %f ���� x = %f y = %f\n",x1,arr[0].y,step1, x2,arr[1].y,step2,arr[2].x,arr[2].y);
		// ���ֵ
		float ndcZ_downNear = arr[0].z; // ��ʵ���ù�Զ����֪���Ǵ����ҵľ���
		float ndcZ_downFar = arr[1].z;
		float stepNdcZnear = (arr[2].z - ndcZ_downNear) / height; // (arr[2].x - x1)
		float stepNdcZfar = (arr[2].z - ndcZ_downFar) / height;
		float fragNdcZ = 0;
		float fragNdcStepZ = 0; // ÿһ��x����ʱ���NDC���경��

		// ��������ֵ
		Vector3 g;
		float zt = 0; // view�ռ�����ֵ
		//Vector3 viewPosZ = Vector3(beginningVertex[0].w,beginningVertex[1].w,beginningVertex[2].w);
		Vector3 fragWorldPos;
		Vector2 fragUV;
		Vector3 fragNormal;
		for (int y = ymin; y <= ymax + 1; ++y) // ���ϵ��£��Ǵ�С����
		{
			// z NDC ���е���ȱ߽�
			ndcZ_downNear += stepNdcZnear;
			ndcZ_downFar += stepNdcZfar;

			// ÿ�ж�x��z���в���
			fragNdcZ = ndcZ_downNear;

			fragNdcStepZ = (ndcZ_downFar - ndcZ_downNear) / (x2 - x1); // ��ֻ�����

			for (float x = x1; x <= x2; x++)
			{
				fragNdcZ += fragNdcStepZ;
				Vector3 fragWorldPos;
				// 2D��Ļ����������ֵ
				g = CenterOfGravity(Tri.x, Tri.y, Tri.z, Vector2(x, y)); // 2D��������

				zt = 1.0f / (g.x / viewPosZ.x + g.y / viewPosZ.y + g.z / viewPosZ.z); // view�ռ�zֵ
				fragWorldPos = GravityToProperty3D(worldPos[0], worldPos[1], worldPos[2], g, viewPosZ, zt); // ����ռ����꣬ע�⣬uv���������ꡢ�����������鶼��ԭ�������ε�����˳�򣬶���Ҫ����
				fragUV = GravityToProperty2D(uvs[0], uvs[1], uvs[2], g, viewPosZ, zt); // �������ҲҪ����ԭ��˳���
				//Vector3 fragWorldPos = Vector3(fragWorldX, worldY_down, fragWorldZ);
				//fragNormal = worldNormal[0] * g.x + worldNormal[1] * g.y + worldNormal[2] * g.z; // �������ҲҪ����ԭ��˳���

				//fragNormal = (GravityToProperty3D(worldNormal[0], worldNormal[1], worldNormal[2], g, viewPosZ, zt) // �������ҲҪ����ԭ��˳���
				//	+ tbn * normalTex->SampleLinearFilter(fragUV) * twoDivide255 - 1.0f).normalized(); // �����Ŷ�
				//cout << worldNormal[0];
				//cout << worldNormal[1];
				//cout << worldNormal[2];
				fragNormal = GravityToProperty3D(worldNormal[0], worldNormal[1], worldNormal[2], g, viewPosZ, zt).normalized(); // �������ҲҪ����ԭ��˳���
				//cout << "fragNormal = " << fragNormal;
				//cout << "g = " << g;
				//printf("XXXXXXXXXXXXXX\n");
				if (isNormalMap)
				{
					TBN.rows[0][2] = fragNormal.x;
					TBN.rows[1][2] = fragNormal.y;
					TBN.rows[2][2] = fragNormal.z;
					Vector3 normaldist = normalTex->SampleLinearFilter(fragUV) * twoDivide255 - 1.0f; // �����Ŷ�
					fragNormal = (TBN * normaldist).normalized();
				}
				//fragNormal = (worldNormal[0] * g.x + worldNormal[1] * g.y + worldNormal[2] * g.z).normalized(); // �������ҲҪ����ԭ��˳���
				fms.push_back(Fragment(Vector2(x+0.5f, y+0.5f), fragNormal, fragWorldPos, fragNdcZ, fragUV));// ��ƬԪ����Ļ���ꡢ���ߡ��������ꡢz���ֵ
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

	// ��Ļ2D�ռ�����������ֵ
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

	if (dx == 0) { // ��ֱ�߶�
		int y_increment = dy > 0 ? 1 : -1; // ȷ��y����������
		int y = static_cast<int>(P1.y);

		for (; y != static_cast<int>(P2.y); y += y_increment) {
			SET_WHITE_PIXEL(w, static_cast<int>(P1.x), y);
		}
		SET_WHITE_PIXEL(w, static_cast<int>(P2.x), static_cast<int>(P2.y));
	}
	else if (dy == 0) { // ˮƽ�߶�
		int x_increment = dx > 0 ? 1 : -1; // ȷ��x����������
		int x = static_cast<int>(P1.x);

		for (; x != static_cast<int>(P2.x); x += x_increment) {
			SET_WHITE_PIXEL(w, x, static_cast<int>(P1.y));
		}
		SET_WHITE_PIXEL(w, static_cast<int>(P2.x), static_cast<int>(P2.y));
	}
	else { // �����߶�
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

// ���������ֵ
// ���������ε���С��Χ��
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

// �жϵ��Ƿ����������ڲ�
bool isPointInTriangle(Vector3& point, Vector3& v0, Vector3& v1, Vector3& v2) {
	// ���������ε��������������
	float triangleArea = cross(v1 - v0, v2 - v0).length() / 2.0f;
	float w0 = cross(v1 - v0, point - v0).length() / triangleArea;
	float w1 = cross(v2 - v1, point - v1).length() / triangleArea;
	float w2 = 1.0f - w0 - w1;

	// �ж����������Ƿ�Ϸ�
	return (w0 >= 0 && w0 <= 1) && (w1 >= 0 && w1 <= 1) && (w2 >= 0 && w2 <= 1);
}

void DrawUpTriangle(Window* w, Vector2 arr[3], const float& ymax, const float& ymin)
{
	//printf("��������");
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
	//printf("��������");
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
// �����ƣ�����д��ƬԪ
void DrawTriangle(Window* w, Triangle<Vector2>& Tri)
{
	// ���������㰴y�����С��������
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

	if (equals(arr[0].y, arr[1].y)) // ��������
	{
		DrawDownTriangle(w, arr, ymax, ymin);
	}
	else if (equals(arr[1].y, arr[2].y)) // ��������
	{
		DrawUpTriangle(w, arr, ymax, ymin);
	}
	else // ���¶���
	{
		//printf("���¶���");
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
		// ��������
		Vector2 upArr[3] = { arr[0],arr[1],midPoint };
		Vector2 downArr[3] = { arr[1],midPoint,arr[2] };
		std::sort(upArr, upArr + 3, cmp2d);
		std::sort(downArr, downArr + 3, cmp2d);
		DrawUpTriangle(w, upArr, arr[1].y, arr[0].y);
		DrawDownTriangle(w, downArr, arr[2].y, arr[1].y);
	}
}