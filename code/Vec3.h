#pragma once
#include <cmath>

class Vec3 
{
    public:
        double x, y, z;
        
        Vec3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
        
        Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
        Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
        Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
        Vec3 operator/(double s) const { return Vec3(x / s, y / s, z / s); }
        Vec3& operator+=(const Vec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
        
        double magnitude() const { return sqrt(x*x + y*y + z*z); }
        double distance(const Vec3& v) const { return (*this - v).magnitude(); }
        Vec3 normalized() const 
        { 
            double mag = magnitude();
            if (mag > 0) return *this / mag;
            return Vec3(0, 0, 0);
        }
};