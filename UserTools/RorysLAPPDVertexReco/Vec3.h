#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>
#include <vector>

struct Vec3 {
    double x, y, z;

    Vec3(double x_= 0.0, double y_= 0.0, double z_= 0.0); 

    // Vector addition
    Vec3 operator+(const Vec3& other) const;

    // Scalar multiplication
    Vec3 operator*(double scalar) const;

    // Cross product
    Vec3 cross(const Vec3& other) const;

    Vec3 normalise() const;

    void Print();

    std::vector<double> GetPositionAsVector() const;
};

#endif
