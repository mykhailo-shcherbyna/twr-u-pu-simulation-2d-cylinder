#ifndef CELL_H
#define CELL_H

#include <cassert>
#include <array>

struct Cell {
    struct Point {
        double r = 0.0;
        double z = 0.0;
    };

    enum {
        LEFT = 0, RIGHT = 1
    };

    static constexpr double SECTOR_ANGLE = 1.0;

    Point node{};

    std::array<double, 2> rBounds{};
    std::array<double, 2> zBounds{};

    double rStep{};
    double zStep{};

    std::array<double, 2> rAreas{};
    double zArea{};

    double volume{};

    Cell() = default;

    Cell(Point point,
         std::array<double, 2> rSteps,
         std::array<double, 2> zSteps)
        : node{point}
        , rBounds{point.r - rSteps[LEFT], point.r + rSteps[RIGHT]}
        , zBounds{point.z - zSteps[LEFT], point.z + zSteps[RIGHT]}
        , rStep{rSteps[LEFT] + rSteps[RIGHT]}
        , zStep{zSteps[LEFT] + zSteps[RIGHT]}
        , rAreas{}
        , zArea{}
        , volume{}
    {
        assert(rBounds[LEFT] >= 0.0 && rBounds[RIGHT] > rBounds[LEFT]);
        assert(zBounds[LEFT] >= 0.0 && zBounds[RIGHT] > zBounds[LEFT]);

        rAreas[LEFT]  = SECTOR_ANGLE * zStep * rBounds[LEFT];
        rAreas[RIGHT] = SECTOR_ANGLE * zStep * rBounds[RIGHT];

        zArea = 0.5 * SECTOR_ANGLE * (rBounds[RIGHT] * rBounds[RIGHT] - rBounds[LEFT] * rBounds[LEFT]);

        volume = zArea * zStep;
    }
};

#endif // CELL_H
