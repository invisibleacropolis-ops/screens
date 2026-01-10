#include "Math.h"

Vec3 Vec3Sub(const Vec3 &a, const Vec3 &b) {
  return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vec3 Vec3Cross(const Vec3 &a, const Vec3 &b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec3 Vec3Normalize(const Vec3 &v) {
  float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  if (len == 0.0f) {
    return {0.0f, 0.0f, 0.0f};
  }
  return {v.x / len, v.y / len, v.z / len};
}

Mat4 Mat4Identity() {
  Mat4 out{};
  out.m = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
           0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  return out;
}

Mat4 Mat4Multiply(const Mat4 &a, const Mat4 &b) {
  Mat4 out{};
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      out.m[row + col * 4] =
          a.m[row + 0] * b.m[col * 4 + 0] + a.m[row + 4] * b.m[col * 4 + 1] +
          a.m[row + 8] * b.m[col * 4 + 2] + a.m[row + 12] * b.m[col * 4 + 3];
    }
  }
  return out;
}

Mat4 Mat4Translate(float x, float y, float z) {
  Mat4 out = Mat4Identity();
  out.m[12] = x;
  out.m[13] = y;
  out.m[14] = z;
  return out;
}

Mat4 Mat4Scale(float x, float y, float z) {
  Mat4 out = Mat4Identity();
  out.m[0] = x;
  out.m[5] = y;
  out.m[10] = z;
  return out;
}

Mat4 Mat4RotateX(float radians) {
  Mat4 out = Mat4Identity();
  float c = std::cos(radians);
  float s = std::sin(radians);
  out.m[5] = c;
  out.m[6] = s;
  out.m[9] = -s;
  out.m[10] = c;
  return out;
}

Mat4 Mat4RotateY(float radians) {
  Mat4 out = Mat4Identity();
  float c = std::cos(radians);
  float s = std::sin(radians);
  out.m[0] = c;
  out.m[2] = -s;
  out.m[8] = s;
  out.m[10] = c;
  return out;
}

Mat4 Mat4RotateZ(float radians) {
  Mat4 out = Mat4Identity();
  float c = std::cos(radians);
  float s = std::sin(radians);
  out.m[0] = c;
  out.m[1] = s;
  out.m[4] = -s;
  out.m[5] = c;
  return out;
}

Mat4 Mat4Perspective(float fovRadians, float aspect, float nearPlane,
                     float farPlane) {
  Mat4 out{};
  float f = 1.0f / std::tan(fovRadians / 2.0f);
  out.m = {f / aspect,
           0.0f,
           0.0f,
           0.0f,
           0.0f,
           f,
           0.0f,
           0.0f,
           0.0f,
           0.0f,
           (farPlane + nearPlane) / (nearPlane - farPlane),
           -1.0f,
           0.0f,
           0.0f,
           (2.0f * farPlane * nearPlane) / (nearPlane - farPlane),
           0.0f};
  return out;
}

Mat4 Mat4LookAt(const Vec3 &eye, const Vec3 &target, const Vec3 &up) {
  Vec3 forward = Vec3Normalize(Vec3Sub(target, eye));
  Vec3 side = Vec3Normalize(Vec3Cross(forward, up));
  Vec3 upVec = Vec3Cross(side, forward);

  Mat4 out = Mat4Identity();
  out.m[0] = side.x;
  out.m[1] = side.y;
  out.m[2] = side.z;
  out.m[4] = upVec.x;
  out.m[5] = upVec.y;
  out.m[6] = upVec.z;
  out.m[8] = -forward.x;
  out.m[9] = -forward.y;
  out.m[10] = -forward.z;
  out.m[12] = -(side.x * eye.x + side.y * eye.y + side.z * eye.z);
  out.m[13] = -(upVec.x * eye.x + upVec.y * eye.y + upVec.z * eye.z);
  out.m[14] = (forward.x * eye.x + forward.y * eye.y + forward.z * eye.z);
  return out;
}
