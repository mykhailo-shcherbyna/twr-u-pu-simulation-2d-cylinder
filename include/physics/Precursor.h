#ifndef PRECURSOR_H
#define PRECURSOR_H

#include <cstddef>
#include <numbers>
#include <cassert>
#include <array>
#include <cmath>

#include "Nuclide.h"
#include "Neutron.h"

struct Precursor {
    enum : std::size_t {
        U235 = 0,
        PU239,
        NUM_FAMILIES
    };

    static std::string toString(std::size_t family, std::size_t group) {
        assert(group < NUM_GROUPS);

        std::string name;
        switch (family) {
            case U235:
                name = Nuclide::toString(parent(U235));
                break;
            case PU239:
                name = Nuclide::toString(parent(PU239));
                break;
            default: return "Unknown";
        }

        return name.append("-Group-" + std::to_string(group));
    }

    static constexpr std::array<std::size_t, NUM_FAMILIES> PARENTS = {
        Nuclide::U235,
        Nuclide::PU239
    };

    static std::size_t parent(std::size_t family) {
        return PARENTS.at(family);
    }

    static constexpr std::size_t NUM_GROUPS = 6;

    std::array<std::array<double, NUM_GROUPS>, NUM_FAMILIES> decayConstant{};
    std::array<std::array<double, Neutron::NUM_GROUPS>, NUM_FAMILIES> neutronYield{};
    std::array<std::array<double, NUM_GROUPS>, NUM_FAMILIES> fraction{};

    Precursor() = default;

    void setDecayConstant(std::size_t family, const std::array<double, NUM_GROUPS>& halfLives) {
        for (std::size_t gi = 0; gi < NUM_GROUPS; ++gi) {
            assert(halfLives[gi] > 0.0 && !std::isinf(halfLives[gi]));
            decayConstant.at(family)[gi] = std::numbers::ln2 / halfLives[gi];
        }
    }

    void setNeutronYield(std::size_t family, const std::array<double, Neutron::NUM_GROUPS>& neutronYields) {
        neutronYield.at(family) = neutronYields;
    }

    void setFraction(std::size_t family, const std::array<double, NUM_GROUPS>& fractions) {
        fraction.at(family) = fractions;
    }
};

#endif // PRECURSOR_H
