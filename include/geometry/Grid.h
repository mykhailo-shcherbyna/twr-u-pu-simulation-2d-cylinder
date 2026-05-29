#ifndef GRID_H
#define GRID_H

#include <stdexcept> // for std::invalid_argument
#include <iostream>
#include <cstddef>   // for std::size_t
#include <numbers>   // for std::numbers::pi
#include <array>
#include <cmath>

#include "Cylinder.h"
#include "Cell.h"

template<std::size_t rN, std::size_t zN>
class Grid {
    using Layer = std::array<Cell, rN>;

    // Steps between nodes. Size is N-1 because N points have N-1 intervals.
    std::array<double, rN - 1> rSteps{};
    std::array<double, zN - 1> zSteps{};

    std::array<Layer, zN> layers{};

    bool matches(const Cylinder& cylinder) const;

    void initializeLayers();

    Cell& cell(std::size_t ri, std::size_t zi) {
        return layers[zi][ri];
    }

public:
    Grid(const Cylinder& cylinder) {
        const double rStep = cylinder.radius / static_cast<double>(rN - 1);
        const double zStep = cylinder.height / static_cast<double>(zN - 1);

        rSteps.fill(rStep);
        zSteps.fill(zStep);

        initializeLayers();

        if (!matches(cylinder)) {
            throw std::invalid_argument("Grid does not match the cylinder.");
        }

        std::cout << "Grid created and verified successfully!" << std::endl;
    }

    double rStep(std::size_t ri) const {
        return rSteps.at(ri);
    }

    double zStep(std::size_t zi) const {
        return zSteps.at(zi);
    }

    const Cell& cell(std::size_t ri, std::size_t zi) const {
        const Layer& layer = layers.at(zi);
        return layer.at(ri);
    }
};

template<std::size_t rN, std::size_t zN>
bool Grid<rN, zN>::matches(const Cylinder &cylinder) const {
    const double tolerance = 1e-6;

    const double scale = 2.0 * std::numbers::pi / Cell::SECTOR_ANGLE;

    double radius = 0.0;
    double height = 0.0;
    double baseArea = 0.0;
    double volume = 0.0;

    for (const auto &layer : layers) {
        for (const auto &cell : layer) {
            radius += cell.rStep / static_cast<double>(zN);
            height += cell.zStep / static_cast<double>(rN);

            baseArea += scale * cell.zArea / static_cast<double>(zN);

            volume += scale * cell.volume;
        }
    }

    const double lateralArea = 2.0 * std::numbers::pi * radius * height;
    const double totalArea = 2.0 * baseArea + lateralArea;

    auto square = [](double value) {
        return value * value;
    };

    const double distance = std::sqrt(
        square(cylinder.height - height) +
        square(cylinder.radius - radius) +
        square(cylinder.baseArea() - baseArea) +
        square(cylinder.volume() - volume) +
        square(cylinder.lateralArea() - lateralArea) +
        square(cylinder.totalArea() - totalArea)
    );

    return distance < tolerance;
}

template<std::size_t rN, std::size_t zN>
void Grid<rN, zN>::initializeLayers() {
    double z = 0.0;

    for (std::size_t zi = 0; zi < zN; ++zi) {
        std::array<double, 2> dz{};

        dz[Cell::LEFT] = (zi == 0) ? 0.0 : 0.5 * zSteps.at(zi - 1);
        dz[Cell::RIGHT] = (zi == zN - 1) ? 0.0 : 0.5 * zSteps.at(zi);

        double r = 0.0;

        for (std::size_t ri = 0; ri < rN; ++ri) {
            std::array<double, 2> dr{};

            dr[Cell::LEFT] = (ri == 0) ? 0.0 : 0.5 * rSteps.at(ri - 1);
            dr[Cell::RIGHT] = (ri == rN - 1) ? 0.0 : 0.5 * rSteps.at(ri);

            cell(ri, zi) = Cell({r, z}, dr, dz);

            if (ri < rN - 1) {
                r += rSteps.at(ri);
            }
        }

        if (zi < zN - 1) {
            z += zSteps.at(zi);
        }
    }
}

#endif // GRID_H
