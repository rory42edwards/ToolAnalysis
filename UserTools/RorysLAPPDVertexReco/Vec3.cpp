#include "Vec3.h"

// Constructor
Vec3::Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}

// Vector addition
Vec3 Vec3::operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
}

// Scalar multiplication
Vec3 Vec3::operator*(double scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
}

// Cross product
Vec3 Vec3::cross(const Vec3& other) const {
    return Vec3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

// Normalize the vector
Vec3 Vec3::normalise() const {
    double length = std::sqrt(x * x + y * y + z * z);
    return Vec3(x / length, y / length, z / length);
}

void Vec3::Print() {
    std::cout<<"X: "<<x<<", Y: "<<y<<", Z: "<<z<<std::endl;
}

std::vector<double> Vec3::GetPositionAsVector() const{
    std::vector<double> vec_out;
    vec_out.push_back(x);vec_out.push_back(y);vec_out.push_back(z);
    return vec_out;
}
