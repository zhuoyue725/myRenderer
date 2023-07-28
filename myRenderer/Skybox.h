#pragma once
#include "Gobals.h"
#include "Vector.h"
#include "Camera.h"
#include "Light.h"

class SkyboxShader :public Shader
{
public:
	virtual void FragmentShader() override
	{
		int fragNum = fragments.size();
		for (uint32_t j = 0; j < fragNum; ++j)
		{
			fragments[j].color = tex->SimpleSample(fragments[j].uv);
		}
	}
};