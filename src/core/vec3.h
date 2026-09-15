#pragma once

#include <cmath>

// Cartesian 3-vector as a plain value type: it lives on the stack, needs no heap
// allocation and is cheap to copy. Replaces the size-3 arma::vec of NSL_SIMULATOR.
struct Vec3 {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

  double& operator[](int axis) { return axis == 0 ? x : (axis == 1 ? y : z); }
  double operator[](int axis) const { return axis == 0 ? x : (axis == 1 ? y : z); }

  Vec3& operator+=(const Vec3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }
  Vec3& operator-=(const Vec3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }
  Vec3& operator*=(double factor) {
    x *= factor;
    y *= factor;
    z *= factor;
    return *this;
  }
};

inline Vec3 operator+(Vec3 a, const Vec3& b) { return a += b; }
inline Vec3 operator-(Vec3 a, const Vec3& b) { return a -= b; }
inline Vec3 operator-(const Vec3& a) { return {-a.x, -a.y, -a.z}; }
inline Vec3 operator*(Vec3 a, double factor) { return a *= factor; }
inline Vec3 operator*(double factor, Vec3 a) { return a *= factor; }
inline double dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline double norm2(const Vec3& a) { return dot(a, a); }
inline double norm(const Vec3& a) { return std::sqrt(norm2(a)); }
