#pragma once
#include <atomic>
#include <mutex>
#include <d3d9.h>
#include <dwmapi.h>
#include <xmmintrin.h>
#include <array>
#include <vector>
#include <cstdlib>
#include <random>
#include <direct.h>
#include <fstream>
#include <string>
#include <sstream>
#include <numbers>
#include "../Renderer/ImGui/imgui.h"

#include "../Renderer/ImGui/imgui_impl_dx11.h"
#include "../Renderer/ImGui/imgui_impl_win32.h"
#include "../../driver/driver.hpp"

inline int Width = GetSystemMetrics(SM_CXSCREEN);
inline int Height = GetSystemMetrics(SM_CYSCREEN);

class Vector2 {
public:
	Vector2() : x(0.f), y(0.f) {}
	Vector2(double _x, double _y) : x(_x), y(_y) {}
	~Vector2() {}

	double x, y;
};

class Vector3 {
public:
	Vector3() : x(0.f), y(0.f), z(0.f) {}
	Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
	~Vector3() {}

	double x, y, z;

	inline double Dot(Vector3 v) { return x * v.x + y * v.y + z * v.z; }
	inline double Distance(Vector3 v) { return sqrt(pow(v.x - x, 2.0) + pow(v.y - y, 2.0) + pow(v.z - z, 2.0)); }
	inline double Length() { return sqrt(x * x + y * y + z * z); }

	Vector3 operator+(Vector3 v) { return Vector3(x + v.x, y + v.y, z + v.z); }
	Vector3 operator-(Vector3 v) { return Vector3(x - v.x, y - v.y, z - v.z); }
	Vector3 operator*(double flNum) { return Vector3(x * flNum, y * flNum, z * flNum); }
};

struct FPlane : Vector3 { double W; FPlane() : W(0) {} FPlane(double W) : W(W) {} };

class FMatrix {
public:
	double m[4][4];
	FPlane XPlane, YPlane, ZPlane, WPlane;

	FMatrix() : XPlane(), YPlane(), ZPlane(), WPlane() {}
	FMatrix(FPlane X, FPlane Y, FPlane Z, FPlane W) : XPlane(X), YPlane(Y), ZPlane(Z), WPlane(W) {}

	D3DMATRIX ToD3DMATRIX() const {
		D3DMATRIX r;
		r._11 = XPlane.x; r._12 = XPlane.y; r._13 = XPlane.z; r._14 = XPlane.W;
		r._21 = YPlane.x; r._22 = YPlane.y; r._23 = YPlane.z; r._24 = YPlane.W;
		r._31 = ZPlane.x; r._32 = ZPlane.y; r._33 = ZPlane.z; r._34 = ZPlane.W;
		r._41 = WPlane.x; r._42 = WPlane.y; r._43 = WPlane.z; r._44 = WPlane.W;
		return r;
	}
};

struct FQuat { double x, y, z, w; };

struct FTransform final {
	FQuat Rotation;
	Vector3 Translation;
	uint8_t Pad1[0x8];
	Vector3 Scale3D;
	uint8_t Pad2[0x8];

	FMatrix ToMatrixWithScale() const {
		Vector3 S((Scale3D.x == 0.0) ? 1.0 : Scale3D.x, (Scale3D.y == 0.0) ? 1.0 : Scale3D.y, (Scale3D.z == 0.0) ? 1.0 : Scale3D.z);
		double x2 = Rotation.x + Rotation.x;
		double y2 = Rotation.y + Rotation.y;
		double z2 = Rotation.z + Rotation.z;
		double xx2 = Rotation.x * x2;
		double yy2 = Rotation.y * y2;
		double zz2 = Rotation.z * z2;
		double yz2 = Rotation.y * z2;
		double wx2 = Rotation.w * x2;
		double xy2 = Rotation.x * y2;
		double wz2 = Rotation.w * z2;
		double xz2 = Rotation.x * z2;
		double wy2 = Rotation.w * y2;

		FMatrix M;
		M.WPlane.x = Translation.x; M.WPlane.y = Translation.y; M.WPlane.z = Translation.z;
		M.XPlane.x = (1.0 - (yy2 + zz2)) * S.x; M.YPlane.y = (1.0 - (xx2 + zz2)) * S.y; M.ZPlane.z = (1.0 - (xx2 + yy2)) * S.z;
		M.ZPlane.y = (yz2 - wx2) * S.z; M.YPlane.z = (yz2 + wx2) * S.y;
		M.YPlane.x = (xy2 - wz2) * S.y; M.XPlane.y = (xy2 + wz2) * S.x;
		M.ZPlane.x = (xz2 + wy2) * S.z; M.XPlane.z = (xz2 - wy2) * S.x;
		M.XPlane.W = 0; M.YPlane.W = 0; M.ZPlane.W = 0; M.WPlane.W = 1;
		return M;
	}
};

struct Camera { Vector3 Location, Rotation; float FieldOfView; };

template<class T>
class tarray {
public:
	T* data = nullptr;
	std::int32_t count = 0, maxx = 0;
	tarray() {}
	tarray(T* d, std::int32_t c, std::int32_t m) : data(d), count(c), maxx(m) {}

	bool is_valid() const noexcept { return data != nullptr; }
	std::int32_t size() const noexcept { return count; }
	bool is_valid_index(std::int32_t i) const noexcept { return i < size(); }

	T& operator[](std::int32_t i) noexcept { return data[i]; }
	const T& operator[](std::int32_t i) const noexcept { return data[i]; }
};

inline D3DMATRIX MatrixMultiplication(D3DMATRIX m1, D3DMATRIX m2) {
	D3DMATRIX o;
	o._11 = m1._11 * m2._11 + m1._12 * m2._21 + m1._13 * m2._31 + m1._14 * m2._41;
	o._12 = m1._11 * m2._12 + m1._12 * m2._22 + m1._13 * m2._32 + m1._14 * m2._42;
	o._13 = m1._11 * m2._13 + m1._12 * m2._23 + m1._13 * m2._33 + m1._14 * m2._43;
	o._14 = m1._11 * m2._14 + m1._12 * m2._24 + m1._13 * m2._34 + m1._14 * m2._44;
	o._21 = m1._21 * m2._11 + m1._22 * m2._21 + m1._23 * m2._31 + m1._24 * m2._41;
	o._22 = m1._21 * m2._12 + m1._22 * m2._22 + m1._23 * m2._32 + m1._24 * m2._42;
	o._23 = m1._21 * m2._13 + m1._22 * m2._23 + m1._23 * m2._33 + m1._24 * m2._43;
	o._24 = m1._21 * m2._14 + m1._22 * m2._24 + m1._23 * m2._34 + m1._24 * m2._44;
	o._31 = m1._31 * m2._11 + m1._32 * m2._21 + m1._33 * m2._31 + m1._34 * m2._41;
	o._32 = m1._31 * m2._12 + m1._32 * m2._22 + m1._33 * m2._32 + m1._34 * m2._42;
	o._33 = m1._31 * m2._13 + m1._32 * m2._23 + m1._33 * m2._33 + m1._34 * m2._43;
	o._34 = m1._31 * m2._14 + m1._32 * m2._24 + m1._33 * m2._34 + m1._34 * m2._44;
	o._41 = m1._41 * m2._11 + m1._42 * m2._21 + m1._43 * m2._31 + m1._44 * m2._41;
	o._42 = m1._41 * m2._12 + m1._42 * m2._22 + m1._43 * m2._32 + m1._44 * m2._42;
	o._43 = m1._41 * m2._13 + m1._42 * m2._23 + m1._43 * m2._33 + m1._44 * m2._43;
	o._44 = m1._41 * m2._14 + m1._42 * m2._24 + m1._43 * m2._34 + m1._44 * m2._44;
	return o;
}

inline D3DMATRIX Matrix(Vector3 r) {
	float p = r.x * float(std::numbers::pi) / 180.f;
	float y = r.y * float(std::numbers::pi) / 180.f;
	float ro = r.z * float(std::numbers::pi) / 180.f;

	float SP = sinf(p), CP = cosf(p);
	float SY = sinf(y), CY = cosf(y);
	float SR = sinf(ro), CR = cosf(ro);

	D3DMATRIX m;
	m.m[0][0] = CP * CY; m.m[0][1] = CP * SY; m.m[0][2] = SP; m.m[0][3] = 0.f;
	m.m[1][0] = SR * SP * CY - CR * SY; m.m[1][1] = SR * SP * SY + CR * CY; m.m[1][2] = -SR * CP; m.m[1][3] = 0.f;
	m.m[2][0] = -(CR * SP * CY + SR * SY); m.m[2][1] = CY * SR - CR * SP * SY; m.m[2][2] = CR * CP; m.m[2][3] = 0.f;
	m.m[3][0] = 0; m.m[3][1] = 0; m.m[3][2] = 0; m.m[3][3] = 1.f;
	return m;
}

inline void DrawCornerBox(int X, int Y, int W, int H, const ImColor c, int t) {
	float lw = W / 3.f, lh = H / 3.f;
	auto draw = ImGui::GetBackgroundDrawList();
	draw->AddLine(ImVec2(X, Y), ImVec2(X, Y + lh), c, t);
	draw->AddLine(ImVec2(X, Y), ImVec2(X + lw, Y), c, t);
	draw->AddLine(ImVec2(X + W - lw, Y), ImVec2(X + W, Y), c, t);
	draw->AddLine(ImVec2(X + W, Y), ImVec2(X + W, Y + lh), c, t);
	draw->AddLine(ImVec2(X, Y + H - lh), ImVec2(X, Y + H), c, t);
	draw->AddLine(ImVec2(X, Y + H), ImVec2(X + lw, Y + H), c, t);
	draw->AddLine(ImVec2(X + W - lw, Y + H), ImVec2(X + W, Y + H), c, t);
	draw->AddLine(ImVec2(X + W, Y + H - lh), ImVec2(X + W, Y + H), c, t);
}

static float powf_(float _X, float _Y)
{
	return (_mm_cvtss_f32(_mm_pow_ps(_mm_set_ss(_X), _mm_set_ss(_Y))));
}

static float sqrtf_(float _X) {
	return (_mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss(_X))));
}
static double get_cross_distance(double x1, double y1, double x2, double y2) {
	return sqrtf(powf((x2 - x1), 2) + powf_((y2 - y1), 2));
}
Vector3 PredictLocation(Vector3 target, Vector3 targetVelocity, float projectileSpeed, float projectileGravityScale, float distance)
{
	float horizontalTime = distance / projectileSpeed;
	float verticalTime = distance / projectileSpeed;

	target.x += targetVelocity.x * horizontalTime;
	target.y += targetVelocity.y * horizontalTime;
	target.z += targetVelocity.z * verticalTime +
		abs(-980 * projectileGravityScale) * 0.5f * (verticalTime * verticalTime);

	return target;
}