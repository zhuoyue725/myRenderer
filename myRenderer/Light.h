#pragma once
#include"Vector.h"

class Light
{
public:
	Light() {};
	virtual ~Light() {};
	Vector3 color;
	Vector3 pos;
};
class PointLight : public Light
{
public:
	PointLight() {};
	PointLight(Vector3 p, Vector3 c) { color = c; pos = p; };
	~PointLight() {};

	Vector3 pos;
};
class DirectionLight : public Light
{
public:
	DirectionLight() {};
	DirectionLight(Vector3 d, Vector3 c) { dir = d; color = c; };
	~DirectionLight() {};
	Vector3 dir;
};