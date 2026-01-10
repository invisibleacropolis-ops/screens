#pragma once

#include <array>
#include <cmath>

// 3D Vector
struct Vec3 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

// 4x4 Matrix (column-major)
struct Mat4 {
  std::array<float, 16> m = {};
};

// Constants
constexpr float kPi = 3.14159265358979323846f;

// Vec3 operations
Vec3 Vec3Sub(const Vec3& a, const Vec3& b);
Vec3 Vec3Cross(const Vec3& a, const Vec3& b);
Vec3 Vec3Normalize(const Vec3& v);

// Mat4 operations
Mat4 Mat4Identity();
Mat4 Mat4Multiply(const Mat4& a, const Mat4& b);
Mat4 Mat4Translate(float x, float y, float z);
Mat4 Mat4Scale(float x, float y, float z);
Mat4 Mat4RotateX(float radians);
Mat4 Mat4RotateY(float radians);
Mat4 Mat4RotateZ(float radians);
Mat4 Mat4Perspective(float fovRadians, float aspect, float nearPlane, float farPlane);
Mat4 Mat4LookAt(const Vec3& eye, const Vec3& target, const Vec3& up);
