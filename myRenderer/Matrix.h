#pragma once
#include "Vector.h"

class Matrix3x3
{
public:
	Matrix3x3()
		: rows{ Vector3(1,0,0), Vector3(0,1,0), Vector3(0,0,1) }
	{}

	Matrix3x3(Vector3 row0, Vector3 row1, Vector3 row2)
		: rows{ row0, row1, row2 }
	{}

	Vector3& operator[](int index) { return rows[index]; }
	const Vector3& operator[](int index) const { return rows[index]; }
	Vector3 rows[3];
};
class Matrix4x4
{
public:
	Matrix4x4()
		: rows{ Vector4(1,0,0,0), Vector4(0,1,0,0), Vector4(0,0,1,0), Vector4(0,0,0,1) }
	{}

	Matrix4x4(Vector4 row0, Vector4 row1, Vector4 row2, Vector4 row3)
		: rows{ row0, row1, row2, row3 }
	{}

	Vector4& operator[](int index) { return rows[index]; }
	const Vector4& operator[](int index) const { return rows[index]; }
	Matrix3x3 toMatrix3x3()
	{
		return Matrix3x3(Vector3(rows[0].x, rows[0].y, rows[0].z), Vector3(rows[1].x, rows[1].y, rows[1].z), Vector3(rows[2].x, rows[2].y, rows[2].z));
	}
private:
	Vector4 rows[4];
};