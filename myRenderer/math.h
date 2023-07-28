#pragma once
#include "Vector.h"
#include "Matrix.h"

Vector2 operator+(const Vector2& v1, const Vector2& v2)
{
	return Vector2(v1.x+v2.x, v1.y + v2.y);
}
Vector2 operator-(const Vector2& v1, const Vector2& v2)
{
	return Vector2(v1.x - v2.x, v1.y - v2.y);
}
Vector2 operator*(const Vector2& v,const float&f)
{
	return Vector2(v.x * f, v.y * f);
}
Vector2 operator-(const Vector2& v, const float& f)
{
	return Vector2(v.x - f, v.y - f);
}

Vector3 operator-(const Vector3& v)
{
	return Vector3(-v.x, -v.y, -v.z);
}
Vector3 operator*(const Matrix3x3& m, const Vector3& v)
{
	float x = m[0].x * v.x + m[0].y * v.y + m[0].z * v.z;
	float y = m[1].x * v.x + m[1].y * v.y + m[1].z * v.z;
	float z = m[2].x * v.x + m[2].y * v.y + m[2].z * v.z;

	return Vector3(x, y, z);
}
Vector4 operator*(const Matrix4x4& m, const Vector4& v)
{
	float x = m[0].x * v.x + m[0].y * v.y + m[0].z * v.z + m[0].w * v.w;
	float y = m[1].x * v.x + m[1].y * v.y + m[1].z * v.z + m[1].w * v.w;
	float z = m[2].x * v.x + m[2].y * v.y + m[2].z * v.z + m[2].w * v.w;
	float w = m[3].x * v.x + m[3].y * v.y + m[3].z * v.z + m[3].w * v.w;

	return Vector4(x, y, z, w);
}
Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2)
{
	Matrix4x4 result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] = 0;
			for (int k = 0; k < 4; k++) {
				result[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	return result;
}
Vector3 operator*(const Vector3& v, const float& f)
{
	return Vector3(v.x * f, v.y * f, v.z * f);
}
// 分量相乘，注意需要[0,1]范围内，否则会越界
// TODO：优化？
const float division255 = 1.0f / 255;
Vector3 operator*(const Vector3& v1, const Vector3& v2)
{
	return Vector3(v1.x * division255 * v2.x * division255, v1.y * division255 * v2.y * division255, v1.z * division255 * v2.z * division255) * 255.0f;
}

Vector3 operator+(const Vector3& v1, const Vector3& v2)
{
	return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vector3 operator-(const Vector3& v1, const float f)
{
	return Vector3(v1.x - f, v1.y - f, v1.z - f);
}
Vector3 operator+(const Vector3& v1, const float f)
{
	return Vector3(v1.x + f, v1.y + f, v1.z + f);
}

// 计算两个向量的叉积
float crossProduct(Vector2 &a, Vector2 &b) {
	return a.x * b.y - a.y * b.x;
}

Vector3 operator-(const Vector3& v1, const Vector3& v2)
{
	return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

float dot(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;;
}
// 计算向量叉积
Vector3 cross(const Vector3& v1, const Vector3& v2)
{
	//return Vector3(v1.y * v2.z - v2.y * v1.z, v1.z * v2.x - v2.z * v1.x, v1.x * v2.y - v2.x * v1.y);
	return Vector3(
		v1.z * v2.y - v1.y * v2.z,
		v1.x * v2.z - v1.z * v2.x,
		v1.y * v2.x - v1.x * v2.y
	);
}
Vector4 operator*(const Vector4& v, const float& f)
{
	return Vector4(v.x * f, v.y * f, v.z * f, v.w * f);
}
Vector4 operator+(const Vector4& v1, const Vector4& v2)
{
	return Vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

bool equals(float a, float b, float epsilon = 0.00001f)
{
	return fabs(a - b) < epsilon;
}

bool equals3(float a, float b, float c, float epsilon = 0.00001f)
{
	return fabs(a - b) < epsilon && fabs(a - c) < epsilon;
}
bool equals(Vector3& a, Vector3& b, float epsilon = 0.00001f)
{
	return equals(a.x, b.x) && equals(a.y, b.y) && equals(a.z, b.z);
}
bool cmp2dz(const Vector3& v1, const Vector3& v2) // 2D屏幕坐标+z深度值
{
	if (!equals(v1.y, v2.y))
	{
		return v1.y < v2.y;// 屏幕空间中的上是大
	}
	else
	{
		return v1.x < v2.x;
	}
}
bool cmp2d(const Vector2& v1, const Vector2& v2)
{
	if (!equals(v1.y, v2.y))
	{
		return v1.y < v2.y;// 屏幕空间中的上是小
	}
	else
	{
		return v1.x < v2.x;
	}
}
bool cmp2dzz(const Vector4& v1, const Vector4& v2) // 2D屏幕坐标+z深度值
{
	if (!equals(v1.y, v2.y))
	{
		return v1.y < v2.y;// 屏幕空间中的上是小
	}
	else
	{
		return v1.x < v2.x;
	}
}
bool cmp3d(const Vector3& v1, const Vector3& v2)
{
	if (!equals(v1.y, v2.y))
	{
		return v1.y > v2.y; // 世界空间中的上是大
	}
	else
	{
		return v1.x < v2.x;
	}
}
bool cmp4d(const Vector4& v1, const Vector4& v2) // 最后一项是NDC的深度
{
	if (!equals(v1.y, v2.y))
	{
		return v1.y < v2.y;// 屏幕空间中的上是大
	}
	else
	{
		return v1.x < v2.x;
	}
}
Vector4 ToVector4(Vector3& v, float w = 1.0f)
{
	return Vector4(v.x, v.y, v.z, w);
}