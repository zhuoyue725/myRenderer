//#pragma once
//
//Triangle<Vector4> OrthoToClipSpace(const Triangle<Vector4>& viewTri)
//{
//	if (camera->orthoMat == nullptr)
//	{
//		float Aspect = windowWidth / windowHeight;
//		Vector4 r1(1.0f / (Aspect * camera->Size), 0, 0, 0);
//		Vector4 r2(0, 1.0f / camera->Size, 0, 0);
//		Vector4 r3(0, 0, -2.0f / (camera->Far - camera->Near), (camera->Far + camera->Near) / (camera->Near - camera->Far));
//		Vector4 r4(0, 0, 0, 1);
//		camera->orthoMat = new Matrix4x4(r1, r2, r3, r4);
//	}
//
//	Triangle<Vector4> ClipTri;
//	for (int i = 0; i < 3; ++i)
//	{
//		ClipTri[i] = *camera->orthoMat * viewTri[i];
//	}
//	return ClipTri;
//}
//
//// Clip空间进行裁剪，并转换为NDC空间
//
//// NDC空间到屏幕空间
//Triangle<Vector2> ClipToScreenSpace(const Triangle<Vector4>& ClipTri)
//{
//	Triangle<Vector2> ScrTri;
//	for (int i = 0; i < 3; i++)
//	{
//		ScrTri[i].x = (ClipTri[i].x / ClipTri[i].w * 0.5 + 0.5) * windowWidth;
//		ScrTri[i].y = (ClipTri[i].y / ClipTri[i].w * 0.5 + 0.5) * windowHeight;
//		// 深度缓存
//	}
//	return ScrTri;
//}
//Triangle<Vector4> PerspectToClipSpace(const Triangle<Vector4>& PerspectTri)
//{
//	if (camera->perspectMat == nullptr)
//	{
//		float Aspect = windowWidth / windowHeight;
//		double radian = camera->FOV / 2.0f * M_PI / 180.0; // 将角度转换为弧度
//		float cotHalfFOV = 1.0f / tan(radian);
//		//float cotFOV = tan(camera->FOV);
//		Vector4 r1(cotHalfFOV / Aspect, 0, 0, 0);
//		Vector4 r2(0, cotHalfFOV, 0, 0);
//		Vector4 r3(0, 0, (camera->Far + camera->Near) / (camera->Near - camera->Far), -2.0f * camera->Near * camera->Far / (camera->Far - camera->Near));
//		Vector4 r4(0, 0, -1, 0);
//		camera->perspectMat = new Matrix4x4(r1, r2, r3, r4);
//	}
//
//	Triangle<Vector4> ClipTri;
//	for (int i = 0; i < 3; ++i)
//	{
//		ClipTri[i] = *camera->perspectMat * PerspectTri[i];
//	}
//	//return OrthoToClipSpace(ClipTri);
//	return ClipTri;
//}
//// 注视相机
//Matrix4x4 LookAt(
//	Vector3 pos, // 摄像机位置
//	Vector3 target, // 目标位置
//	Vector3 up// 上向量
//)
//{
//	return Matrix4x4();
//}
//
//Triangle<Vector4> WorldToViewSpace(const Triangle<Vector4>& worldTri)
//{
//	if (camera->viewMat == nullptr)
//	{
//		Vector4 r1(1, 0, 0, 0);
//		Vector4 r2(0, 1, 0, 0);
//		Vector4 r3(0, 0, 1, 0);
//		Vector4 r4(0, 0, 0, 1);
//		Matrix4x4 Rrot(r1, r2, r3, r4);
//		r1 = Vector4(1, 0, 0, -camera->pos.x);
//		r2 = Vector4(0, 1, 0, -camera->pos.y);
//		r3 = Vector4(0, 0, 1, -camera->pos.z);
//		r4 = Vector4(0, 0, 0, 1);
//		Matrix4x4 Rtrans(r1, r2, r3, r4);
//		//Matrix4x4 Rrt = ;
//		camera->viewMat = new Matrix4x4(Rrot * Rtrans);
//	}
//	Triangle<Vector4> viewTri;
//	for (int i = 0; i < 3; ++i)
//	{
//		viewTri[i] = *camera->viewMat * worldTri[i];
//	}
//	return viewTri;
//}
//
//// 定义模型位置、缩放系数、朝向
//Triangle<Vector4> ModelToWorldSpace(const Triangle<Vector4>& modelTri, const Vector3& trans, const Vector3& scale, const Vector3& rotate)
//{
//	if (camera->modelMat == nullptr)
//	{
//		Vector4 r1(1, 0, 0, trans.x);
//		Vector4 r2(0, 1, 0, trans.y);
//		Vector4 r3(0, 0, 1, trans.z);
//		Vector4 r4(0, 0, 0, 1);
//		Matrix4x4 Rtrans(r1, r2, r3, r4);
//		r1 = Vector4(cos(rotate.y * M_PI / 180.0), 0, sin(rotate.y * M_PI / 180.0), 0);
//		r2 = Vector4(0, 1, 0, 0);
//		r3 = Vector4(-sin(rotate.y * M_PI / 180.0), 0, cos(rotate.y * M_PI / 180.0), 0);
//		r4 = Vector4(0, 0, 0, 1);
//		Matrix4x4 Rrot(r1, r2, r3, r4);
//		r1 = Vector4(scale.x, 0, 0, 0);
//		r2 = Vector4(0, scale.y, 0, 0);
//		r3 = Vector4(0, 0, scale.z, 0);
//		r4 = Vector4(0, 0, 0, 1);
//		Matrix4x4 Rscale(r1, r2, r3, r4);
//
//		//auto later = ;
//		camera->modelMat = new Matrix4x4(Rtrans * Rrot * Rscale);
//	}
//	Triangle<Vector4> worldTri;
//	for (int i = 0; i < 3; ++i)
//	{
//		worldTri[i] = *camera->modelMat * modelTri[i];
//	}
//	return worldTri;
//}