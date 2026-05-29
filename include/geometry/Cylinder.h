#ifndef CYLINDER_H
#define CYLINDER_H

#include <stdexcept> // for std::invalid_argument
#include <numbers>   // for std::numbers::pi

struct Cylinder {
    double radius;
    double height;

    Cylinder(double cylinderRadius, double cylinderHeight)
        : radius(cylinderRadius), height(cylinderHeight)
    {
        if (radius <= 0.0) throw std::invalid_argument("Cylinder radius must be positive.");
        if (height <= 0.0) throw std::invalid_argument("Cylinder height must be positive.");
    }

    double baseArea() const {
        return std::numbers::pi * radius * radius;
    }

    double volume() const {
        return baseArea() * height;
    }

    double lateralArea() const {
        return 2.0 * std::numbers::pi * radius * height;
    }

    double totalArea() const {
        return 2.0 * baseArea() + lateralArea();
    }
};

#endif // CYLINDER_H
