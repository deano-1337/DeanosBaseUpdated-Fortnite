#pragma once
#include "../Utilities/util.h"
#include "../Visuals/esp.h"
#include "offsets.hpp"
#include "sdk.h"
#include <math.h>
#include <numbers>

namespace LocalPtrs {
	inline uintptr_t Player;
	inline uintptr_t Mesh;
	inline uintptr_t PlayerState ;
	inline uintptr_t RootComponent;
	inline uintptr_t LocalPlayers;
	inline uintptr_t PlayerController;
	inline uintptr_t Gworld;
}
namespace WorldCache {
	inline uintptr_t AcknowledgedPawn = 0;
	inline uintptr_t Mesh = 0;
	inline uintptr_t PlayerState = 0;
	inline uintptr_t RootComponent = 0;
	inline uintptr_t LocalWeapon = 0;
	inline uintptr_t LocalVehicle = 0;
	inline char TeamIndex = 0;
}

inline Camera vCamera;
Vector3 GetBoneWithRotation(DWORD_PTR mesh, int id) {

	int32_t ActiveTransforms = read<int32_t>(mesh + (offsets::BoneArray + 0x48)) * 0x10;
	auto bone_array = read<tarray<FTransform>>((uintptr_t)mesh + offsets::BoneArray + ActiveTransforms);
	auto BoneTransform = read<FTransform>((uintptr_t)bone_array.data + (id * 0x60));
	FTransform ComponentToWorld = read<FTransform>((uintptr_t)mesh + offsets::ComponentToWorld);

	D3DMATRIX Matrix = MatrixMultiplication(BoneTransform.ToMatrixWithScale().ToD3DMATRIX(), ComponentToWorld.ToMatrixWithScale().ToD3DMATRIX());
	return Vector3(Matrix._41, Matrix._42, Matrix._43);
}

struct CamewaDescwipsion {
	Vector3 Location;
	Vector3 Rotation;
	float FieldOfView;
};

Camera GetViewAngles() {
	CamewaDescwipsion camera;
	auto locationPointer = read<uintptr_t>(LocalPtrs::Gworld + CameraLocation);
	auto rotationPointer = read<uintptr_t>(LocalPtrs::Gworld + CameraRotation);

	struct Rotation {
		double a;
		char pad_0008[24];
		double b;
		char pad_0028[424];
		double c;
	};
	Rotation rotation;
	rotation = read<Rotation>(rotationPointer);

	camera.Location = read<Vector3>(locationPointer);
	camera.Rotation.x = asin(rotation.c) * (180.0 / std::numbers::pi);
	camera.Rotation.y = ((atan2(rotation.a * -1, rotation.b) * (180.0 / std::numbers::pi)) * -1) * -1;
	camera.FieldOfView = read<float>(LocalPtrs::PlayerController + 0x3AC) * 90.f;

	return { camera.Location, camera.Rotation, camera.FieldOfView };
}
bool is_dead(uintptr_t pawn_private)
{
	return (read<char>(pawn_private + offsets::b_is_dying) >> 5) & 1;
}

Vector2 ProjectWorldToScreen(Vector3 WorldLocation) {
	vCamera = GetViewAngles();
	
	auto mMatrix = Matrix(vCamera.Rotation);
	auto XAxis = Vector3(mMatrix.m[0][0], mMatrix.m[0][1], mMatrix.m[0][2]);
	auto YAxis = Vector3(mMatrix.m[1][0], mMatrix.m[1][1], mMatrix.m[1][2]);
	auto ZAxis = Vector3(mMatrix.m[2][0], mMatrix.m[2][1], mMatrix.m[2][2]);

	auto vDeltaCoordinates = WorldLocation - vCamera.Location;
	auto tTransform = Vector3(vDeltaCoordinates.Dot(YAxis), vDeltaCoordinates.Dot(ZAxis), vDeltaCoordinates.Dot(XAxis));

	tTransform.z = (tTransform.z < 1.f) ? 1.f : tTransform.z;
	auto fov_radians = tan(vCamera.FieldOfView * std::numbers::pi / 360.f);

	return Vector2(Width / 2 + tTransform.x * ((Width / 2) / fov_radians) / tTransform.z,Height / 2 - tTransform.y * ((Width / 2) / fov_radians) / tTransform.z);
}

bool IsEnemyVisible(uintptr_t Mesh) {
	auto Seconds = read<double>(LocalPtrs::Gworld + 0x2E0);
	auto LastRenderTime = read<float>(Mesh + 0x1A0);
	return Seconds - LastRenderTime <= 0.06f;
}

float GetDistanceToCrosshair(uintptr_t Mesh) {
	Vector3 head = GetBoneWithRotation(Mesh, 110);
	Vector2 head2D = ProjectWorldToScreen({ head.x, head.y, head.z + 20 });
	float centerX = Width / 2.0f;
	float centerY = Height / 2.0f;
	return sqrtf((head2D.x - centerX) * (head2D.x - centerX) + (head2D.y - centerY) * (head2D.y - centerY));
}

float GetFOVDistance(uintptr_t Mesh) {
	Vector3 head = GetBoneWithRotation(Mesh, 110);
	Vector2 head2D = ProjectWorldToScreen({ head.x, head.y, head.z + 20 });
	float centerX = Width / 2.0f;
	float centerY = Height / 2.0f;
	return fabsf(head2D.x - centerX) + fabsf(head2D.y - centerY); 
}
