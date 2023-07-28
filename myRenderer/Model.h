#pragma once
// 定义模型需要的三角形、片元、模型类
#include "Vector.h"
#include "Matrix.h"
#include "math.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

class Fragment
{
public:
	Fragment() {};
	Fragment(Vector2 v, Vector3 n, Vector3 p, float d, Vector2 uv) :m_x(v.x), m_y(v.y), normal(n), m_worldPos(p), m_depth(d), uv(uv) {};
	~Fragment() {};
	Vector3 normal;
	uint16_t m_x;
	uint16_t m_y; //屏幕空间坐标
	Vector3 color;
	Vector3 m_worldPos; // 世界坐标
	Vector2 uv; // [0,1]
	float m_depth; // NDC空间深度

private:

};

template <typename T>
class Triangle
{
public:
	T x;
	T y;
	T z;
	//vector<Fragment> fragments;
	Vector3 vertexDepth;
	Triangle() :x(), y(), z() {}
	Triangle(T x, T y, T z) :x(x), y(y), z(z) {}
	T& operator[](int index)
	{
		switch (index)
		{
		case 0:
			return x;
			break;
		case 1:
			return y;
			break;
		case 2:
			return z;
			break;
		default:
			throw std::out_of_range("Index out of range!");
			break;
		}
	}
	const T& operator[](int index) const
	{
		switch (index)
		{
		case 0:
			return x;
			break;
		case 1:
			return y;
			break;
		case 2:
			return z;
			break;
		default:
			throw std::out_of_range("Index out of range!");
			break;
		}
	}
	void operator << (const Triangle<T>& T)
	{
		cout << T.x << "," << T.y << "," << T.z << endl;
	}
};

class Model
{
private:
	// 新的创建方式
	vector<Vector2> texs;
	vector<Vector3> normals;
public:
	// shader中三角形，晚点改到shader里面
	Triangle<Vector3>* triangles = nullptr; // 存储2D屏幕空间 + z深度值

	//Vector4* vertex = nullptr;
	//Vector3* face = nullptr;
	//Vector3* face_uv = nullptr;
	//Vector3** normal = nullptr; //和面一一对应，而不是顶点，顶点可能重合
	//Vector2* uvs = nullptr;
	Matrix3x3* normalToWorldMat = nullptr;

	Vector3 pos;
	Vector3 rotate;
	Vector3 scale;

	uint16_t veretexNum = 0;
	uint16_t triangleNum = 0;

	Matrix4x4* modelMat = nullptr; // 每个物体对应自己的模型矩阵

	vector<Vector4> vertexs;
	vector<Vector3> face;
	vector<vector<Vector2>> triUvs;
	vector<vector<Vector3>> triNormals;

	Matrix3x3(*TBN)[3] = nullptr;//用于法线贴图到世界空间变换
	Model() {};
	Model(const char* filename)
	{
		//const char* filename = "./models/floor.obj";
		std::ifstream in;
		in.open(filename, std::ifstream::in);
		if (in.fail()) 
		{
			std::cerr << "read obj failed" << std::endl;
			return;
		}
		std::string line;
		Vector3 ver; // 顶点索引
		size_t ver_idx = 0;
		std::vector<Vector2> tex; // uv数组
		std::vector<Vector3> norm; //法线数组
		while (!in.eof()) 
		{
			std::getline(in, line);
			std::istringstream iss(line.c_str());
			char trash;

			if (!line.compare(0, 2, "v ")) 
			{
				iss >> trash;
				Vector4 v;
				for (int i = 0; i < 3; i++) iss >> v[i];
				vertexs.push_back(v);
			}
			else if (!line.compare(0, 2, "f ")) 
			{
				ver_idx = 0; tex.clear(); norm.clear();
				size_t texIdx, vertexIdx, normalIdx;
				iss >> trash;
				while (iss >> vertexIdx >> trash >> texIdx >> trash >> normalIdx) 
				{
					vertexIdx--; // in wavefront obj all indices start at 1, not zero
					texIdx--;
					--normalIdx;
					ver[ver_idx++] = vertexIdx;
					tex.push_back(texs[texIdx]);
					norm.push_back(normals[normalIdx]);
				}
				//triVertexs.push_back(ver);
				face.push_back(ver);
				triUvs.push_back(tex);
				triNormals.push_back(norm);

			}
			else if (!line.compare(0, 3, "vt ")) 
			{
				iss >> trash >> trash;
				Vector2 v;
				for (int i = 0; i < 2; i++) iss >> v[i];
				texs.push_back(v);
				//iss >> v[0];
			}
			else if (!line.compare(0, 3, "vn ")) 
			{
				iss >> trash >> trash;
				Vector3 v;
				for (int i = 0; i < 3; i++) iss >> v[i];
				normals.push_back(v);
			}
		}
		triangleNum = face.size();
		veretexNum = vertexs.size();
		std::cerr << "# v# " << vertexs.size() << " f# " << face.size() << " vt# " << texs.size() << " vn# " << normals.size() << std::endl;
	};
	
	~Model() {};
	void SetModel(Vector3 p,Vector3 r,Vector3 s)
	{
		pos = p;
		rotate = r;
		scale = s;
		// 计算自己对应的mode矩阵（和相机无关）
		// 设置模型矩阵
		if (modelMat == nullptr)
		{
			// 位移矩阵
			Vector4 r1(1, 0, 0, pos.x);
			Vector4 r2(0, 1, 0, pos.y);
			Vector4 r3(0, 0, 1, pos.z);
			Vector4 r4(0, 0, 0, 1);
			Matrix4x4 Rtrans(r1, r2, r3, r4);
			// 欧拉角需要定义旋转顺序，这里和Unity相同，zxy
			r1 = Vector4(cos(rotate.z * M_PI / 180.0), -sin(rotate.z * M_PI / 180.0), 0, 0);
			r2 = Vector4(sin(rotate.z * M_PI / 180.0), cos(rotate.z * M_PI / 180.0), 0, 0);
			r3 = Vector4(0, 0, 1, 0);
			r4 = Vector4(0, 0, 0, 1);
			Matrix4x4 RrotZ(r1, r2, r3, r4);
			r1 = Vector4(1, 0, 0, 0);
			r2 = Vector4(0, cos(rotate.x * M_PI / 180.0), -sin(rotate.x * M_PI / 180.0), 0);
			r3 = Vector4(0, sin(rotate.x * M_PI / 180.0), cos(rotate.x * M_PI / 180.0), 0);
			r4 = Vector4(0, 0, 0, 1);
			Matrix4x4 RrotX(r1, r2, r3, r4);
			r1 = Vector4(cos(rotate.y * M_PI / 180.0), 0, sin(rotate.y * M_PI / 180.0), 0);
			r2 = Vector4(0, 1, 0, 0);
			r3 = Vector4(-sin(rotate.y * M_PI / 180.0), 0, cos(rotate.y * M_PI / 180.0), 0);
			r4 = Vector4(0, 0, 0, 1);
			Matrix4x4 RrotY(r1, r2, r3, r4);
			// 缩放矩阵
			r1 = Vector4(scale.x, 0, 0, 0);
			r2 = Vector4(0, scale.y, 0, 0);
			r3 = Vector4(0, 0, scale.z, 0);
			r4 = Vector4(0, 0, 0, 1);
			Matrix4x4 Rscale(r1, r2, r3, r4);

			modelMat = new Matrix4x4(Rtrans * RrotZ * RrotX * RrotY * Rscale);
			normalToWorldMat = new Matrix3x3(Matrix4x4(RrotZ * RrotX * RrotY).toMatrix3x3()); // 缩放会有影响，暂时不考虑
		}
	}

};