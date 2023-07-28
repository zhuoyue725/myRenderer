#include "Matrix.h"
#include "math.h"
#pragma once
class Camera
{
public:
	bool view_update = false;
	bool proj_update = false;
	bool isSurround = true;

	float Near = 0.1f;
	float Far = 10;
	float Size = 5;
	float FOV = 30;

	//Matrix4x4* perspectMat = nullptr;
	//Matrix4x4* orthoMat = nullptr;
	Matrix4x4* projectMat = nullptr;
	Matrix4x4* viewMat = nullptr;
	//Matrix4x4* modelMat = nullptr;
	//Matrix3x3* normalToWorldMat = nullptr;

	Matrix4x4* MVP = nullptr;
	Matrix4x4* VP = nullptr;//��Դ��view��project���󣬱任����Դ�����ƽ�й�Դ������ͶӰ������Ļ�ռ�
	Vector3 pos;
	Vector3 worldUp; //����ռ�Ĵ�ֱ����
	Vector3 target;

	// ���������������
	Vector3 up; //����ռ�Ĵ�ֱ����
	Vector3 r; // ��
	Vector3 dir; // ǰ

	bool isOrtho = false; // �Ƿ�����ͶӰ
	float(*zBuffer)[windowWidth] = new float[windowWidth][windowHeight];

	// ����ע��Ŀ����λ��
	void SetTagPosition(Vector3& tag)
	{
		if (equals(tag, this->target))
		{
			//printf("equal\n");
			return;
		}
		if (isSurround)
		{
			this->target = tag;
			dir = (target - pos).normalized();
		}
		else
		{
			return;
		}
		view_update = true;
		dir = (tag - pos).normalized();
		r = cross(dir, worldUp).normalized(); // dir��up��˽��
		up = cross(r, dir).normalized();

		Vector4 r1(r.x, r.y, r.z, 0);
		Vector4 r2(up.x, up.y, up.z, 0);
		Vector4 r3(dir.x, dir.y, dir.z, 0);
		//Vector4 r1(r.x, up.x, dir.x, 0);
		//Vector4 r2(r.y, up.y, dir.y, 0);
		//Vector4 r3(r.z, up.z, dir.z, 0);
		Vector4 r4(0, 0, 0, 1);
		Matrix4x4 Rrot(r1, r2, r3, r4);
		r1 = Vector4(1, 0, 0, -pos.x);
		r2 = Vector4(0, 1, 0, -pos.y);
		r3 = Vector4(0, 0, 1, -pos.z);
		r4 = Vector4(0, 0, 0, 1);
		Matrix4x4 Rtrans(r1, r2, r3, r4);
		//Matrix4x4 Rrt = ;
		*viewMat = Matrix4x4(Rrot * Rtrans);
	}
	// ����Ŀ�귽��
	void SetDirection(Vector3& d)
	{
		if (equals(d, dir))
		{
			//printf("equal\n");
			return;
		}
		if (isSurround)
		{
			return;
		}
		else
		{
			dir = d.normalized();
			cout << dir;
		}
		view_update = true;
		r = cross(dir, worldUp).normalized(); // dir��up��˽��
		up = cross(r, dir).normalized();

		Vector4 r1(r.x, r.y, r.z, 0);
		Vector4 r2(up.x, up.y, up.z, 0);
		Vector4 r3(dir.x, dir.y, dir.z, 0);
		//Vector4 r1(r.x, up.x, dir.x, 0);
		//Vector4 r2(r.y, up.y, dir.y, 0);
		//Vector4 r3(r.z, up.z, dir.z, 0);
		Vector4 r4(0, 0, 0, 1);
		Matrix4x4 Rrot(r1, r2, r3, r4);
		r1 = Vector4(1, 0, 0, -pos.x);
		r2 = Vector4(0, 1, 0, -pos.y);
		r3 = Vector4(0, 0, 1, -pos.z);
		r4 = Vector4(0, 0, 0, 1);
		Matrix4x4 Rtrans(r1, r2, r3, r4);
		//Matrix4x4 Rrt = ;
		*viewMat = Matrix4x4(Rrot * Rtrans);
	}
	void SetPosition(Vector3& p)// �޸��������ռ�����
	{
		if (equals(p,this->pos))
		{
			return;
		}
		// ���������������ͬ�����
		//Vector4 r1(1, 0, 0, 0);
		//Vector4 r2(0, 1, 0, 0);
		//Vector4 r3(0, 0, 1, 0);
		//Vector4 r4(0, 0, 0, 1);
		//Matrix4x4 Rrot(r1, r2, r3, r4);
		view_update = true;
		this->pos = p;
		if (isSurround)
		{
			dir = (target - p).normalized();
		}
		else
		{
			dir = dir.normalized();
		}
		r = cross(dir, worldUp).normalized(); // dir��up��˽��
		up = cross(r, dir).normalized();

		Vector4 r1(r.x, r.y, r.z, 0);
		Vector4 r2(up.x, up.y, up.z, 0);
		Vector4 r3(dir.x, dir.y, dir.z, 0);
		//Vector4 r1(r.x, up.x, dir.x, 0);
		//Vector4 r2(r.y, up.y, dir.y, 0);
		//Vector4 r3(r.z, up.z, dir.z, 0);
		Vector4 r4(0, 0, 0, 1);
		Matrix4x4 Rrot(r1, r2, r3, r4);
		r1 = Vector4(1, 0, 0, -pos.x);
		r2 = Vector4(0, 1, 0, -pos.y);
		r3 = Vector4(0, 0, 1, -pos.z);
		r4 = Vector4(0, 0, 0, 1);
		Matrix4x4 Rtrans(r1, r2, r3, r4);
		//Matrix4x4 Rrt = ;
		*viewMat = Matrix4x4(Rrot * Rtrans);
	}
	Camera(Vector3 p, Vector3 u, Vector3 tag = Vector3(0, 5, 1000),bool isO = false/*�Ƿ�����*/,bool isS = true/*�Ƿ������*/, Vector3 d = Vector3(0, 0, 1)/*�������z������*/) :pos(p), worldUp(u), target(tag),isOrtho(isO)
	{
		// ����͸�Ӿ����������
		if (viewMat == nullptr)
		{
			// ���������������ͬ�����
			//Vector4 r1(1, 0, 0, 0);
			//Vector4 r2(0, 1, 0, 0);
			//Vector4 r3(0, 0, 1, 0);
			//Vector4 r4(0, 0, 0, 1);
			//Matrix4x4 Rrot(r1, r2, r3, r4);
			isSurround = isS;
			if (isS)
			{
				this->dir = (tag - pos).normalized();
				target = tag;
			}
			else
			{
				this->dir = d;
				dir = d.normalized();
			}
			r = cross(dir,worldUp).normalized(); // dir��up��˽��
			up = cross(r,dir).normalized();

			//Vector4 r1(r.x, r.y, r.z, 0);
			//Vector4 r2(up.x, up.y, up.z, 0);
			//Vector4 r3(dir.x, dir.y, dir.z, 0);
			Vector4 r1(r.x, up.x, dir.x, 0);
			Vector4 r2(r.y, up.y, dir.y, 0);
			Vector4 r3(r.z, up.z, dir.z, 0);
			Vector4 r4(0, 0, 0, 1);
			Matrix4x4 Rrot(r1, r2, r3, r4);
			r1 = Vector4(1, 0, 0, -pos.x);
			r2 = Vector4(0, 1, 0, -pos.y);
			r3 = Vector4(0, 0, 1, -pos.z);
			r4 = Vector4(0, 0, 0, 1);
			Matrix4x4 Rtrans(r1, r2, r3, r4);
			//Matrix4x4 Rrt = ;
			viewMat = new Matrix4x4(Rrot * Rtrans);
		}
		if (!isO && projectMat == nullptr)
		{
			float Aspect = windowWidth / windowHeight;
			double radian = FOV / 2.0f * M_PI / 180.0; // ���Ƕ�ת��Ϊ����
			float cotHalfFOV = 1.0f / tan(radian);
			//float cotFOV = tan(camera->FOV);
			Vector4 r1(cotHalfFOV / Aspect, 0, 0, 0);
			Vector4 r2(0, cotHalfFOV, 0, 0);
			Vector4 r3(0, 0, -(Far + Near) / (Far - Near), -2.0f * Near * Far / (Far - Near));
			Vector4 r4(0, 0, -1.0f, 0);
			//perspectMat = new Matrix4x4(r1, r2, r3, r4);
			projectMat = new Matrix4x4(r1, r2, r3, r4);
		}
		else if(isO && projectMat == nullptr)
		{
			float Aspect = windowWidth / windowHeight;
			Vector4 r1(1.0f / (Aspect * Size), 0, 0, 0);
			Vector4 r2(0, -1.0f / Size, 0, 0);
			Vector4 r3(0, 0, 2.0f / (Near - Far), (Far + Near) / (Near - Far));
			Vector4 r4(0, 0, 0, 1);
			//orthoMat = new Matrix4x4(r1, r2, r3, r4);
			projectMat = new Matrix4x4(r1, r2, r3, r4);

			Vector4 ts = Vector4(-2, 0, 2,1);
			Vector4 res = *projectMat * *viewMat * ts;
			Matrix4x4 mat = *projectMat * *viewMat;
			printf("x=%f y=%f z=%f w=%f\n", res.x,res.y,res.z,res.w);
			res = res * 0.5f + Vector4(0.5f, 0.5f, 0.5f,0.5f);
			printf("x=%f y=%f z=%f correct��\n", res.x * windowWidth, res.y * windowHeight, res.z);
		}
		if (isOrtho)
		{
			VP = new Matrix4x4(*projectMat * *viewMat);

			//printf("3xxxxxxxxx");
			//Vector3 ts = Vector3(-2, 0, 2);
			//Vector3 res = *VP * ts;
			//cout << res << endl;
			//res = res * 0.5f + Vector3(0.5f, 0.5f, 0.5f);
			//printf("x=%f y=%f z=%f\n", res.x * windowWidth, res.y * windowHeight, res.z);
		}
		std::fill(reinterpret_cast<float*>(zBuffer), reinterpret_cast<float*>(zBuffer) + windowHeight * windowWidth, -100.0f);
		//memset(zBuffer, -100.0f, windowHeight * windowWidth * sizeof(float));
	}
	void SetModel(Vector3 trans, Vector3 rotate, Vector3 scale)
	{
		// ����ģ�;���
		//if (modelMat == nullptr)
		//{
		//	// λ�ƾ���
		//	Vector4 r1(1, 0, 0, trans.x);
		//	Vector4 r2(0, 1, 0, trans.y);
		//	Vector4 r3(0, 0, 1, trans.z);
		//	Vector4 r4(0, 0, 0, 1);
		//	Matrix4x4 Rtrans(r1, r2, r3, r4);
		//	// ŷ������Ҫ������ת˳�������Unity��ͬ��zxy
		//	r1 = Vector4(cos(rotate.z * M_PI / 180.0), -sin(rotate.z * M_PI / 180.0), 0, 0);
		//	r2 = Vector4(sin(rotate.z * M_PI / 180.0), cos(rotate.z * M_PI / 180.0), 0, 0);
		//	r3 = Vector4(0, 0, 1, 0);
		//	r4 = Vector4(0, 0, 0, 1);
		//	Matrix4x4 RrotZ(r1, r2, r3, r4);
		//	r1 = Vector4(1, 0, 0, 0);
		//	r2 = Vector4(0, cos(rotate.x * M_PI / 180.0), -sin(rotate.x * M_PI / 180.0), 0);
		//	r3 = Vector4(0, sin(rotate.x * M_PI / 180.0), cos(rotate.x * M_PI / 180.0), 0);
		//	r4 = Vector4(0, 0, 0, 1);
		//	Matrix4x4 RrotX(r1, r2, r3, r4);
		//	r1 = Vector4(cos(rotate.y * M_PI / 180.0), 0, sin(rotate.y * M_PI / 180.0), 0);
		//	r2 = Vector4(0, 1, 0, 0);
		//	r3 = Vector4(-sin(rotate.y * M_PI / 180.0), 0, cos(rotate.y * M_PI / 180.0), 0);
		//	r4 = Vector4(0, 0, 0, 1);
		//	Matrix4x4 RrotY(r1, r2, r3, r4);
		//	// ���ž���
		//	r1 = Vector4(scale.x, 0, 0, 0);
		//	r2 = Vector4(0, scale.y, 0, 0);
		//	r3 = Vector4(0, 0, scale.z, 0);
		//	r4 = Vector4(0, 0, 0, 1);
		//	Matrix4x4 Rscale(r1, r2, r3, r4);

		//	modelMat = new Matrix4x4(Rtrans * RrotZ * RrotX * RrotY * Rscale);
		//	normalToWorldMat = new Matrix3x3(Matrix4x4(RrotZ * RrotX * RrotY).toMatrix3x3()); // ���Ż���Ӱ�죬��ʱ������

		//	MVP = new Matrix4x4(*modelMat * *viewMat * *perspectMat);
		//}
	}
	Camera() :pos(), up() {}
	~Camera() {}

private:

};