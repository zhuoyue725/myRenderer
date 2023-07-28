#pragma once
#include <iostream>
using namespace std;

class Vector2
{
public:
	Vector2() : x(0), y(0) {}
	Vector2(float x, float y) : x(x), y(y) {}
	float x;
	float y;
	Vector2 normalized()const
	{
		float len = length();
		return Vector2(x/len,y/len);
	}
	float length()const
	{
		return sqrt(x*x + y*y);
	}
	float& operator[](size_t index)
	{
		return (index == 0)? x: y;
	}
};
//ostream& operator<<(ostream& out, const Vector2& p)
//{
//	cout << "(" << p.x << ", " << p.y << ")";
//	//out << A.m_real << " + " << A.m_imag << " i ";
//	return out;
//}
class Vector3
{
public:
	Vector3() : x(0), y(0), z(0) {}
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
	float x;
	float y;
	float z;
	//Vector4 ToVector4(float w)
	//{
	//	return Vector4(x, y, z, w);
	//}
	friend ostream& operator<<(ostream& os, const Vector3& v)
	{
		os << v.x << "," << v.y << "," << v.z << endl;
		return os;
	}
	float& operator[](int index)
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
	//void push_back(float f)
	//{

	//}
	Vector3& clamp(float a, float b)
	{
		this->x = max(a, min(b, x));
		this->y = max(a, min(b, y));
		this->z = max(a, min(b, z));
		return *this;
	}
	Vector3 normalized()const
	{
		float len = this->length();
		return Vector3(x / len, y / len, z / len);
	}
	float length()const
	{
		return sqrt(x * x + y * y + z * z);
	}
	const float& operator[](int index) const
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
	Vector3 toRBG()
	{
		return Vector3(x , z, y);
	}
	Vector2 toVector2()
	{
		return Vector2(x, y);
	}

};
class Vector4
{
public:
	Vector4() : x(0), y(0), z(0), w(1) {}
	Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	float x;
	float y;
	float z;
	float w;
	Vector3 ToVector3()
	{
		return Vector3(x, y, z);
	}
	float& operator[](int index)
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
		case 3:
			return w;
			break;
		default:
			throw std::out_of_range("Index out of range!");
			break;
		}
	}
	const float& operator[](int index) const
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
		case 3:
			return w;
			break;
		default:
			throw std::out_of_range("Index out of range!");
			break;
		}
	}
};