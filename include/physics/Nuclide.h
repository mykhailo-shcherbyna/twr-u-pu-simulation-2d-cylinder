#ifndef NUCLIDE_H
#define NUCLIDE_H

#include <cstddef>
#include <numbers>
#include <cassert>
#include <string>
#include <array>
#include <cmath>

#include "Neutron.h"

struct Nuclide {
    enum : std::size_t {
        U235 = 0,
        U238,
        U239,
        NP239,
        PU239,
        NUM_NUCLIDES
    };

    static std::string toString(std::size_t index) {
        switch (index) {
            case U235: return "U-235";
            case U238: return "U-238";
            case U239: return "U-239";
            case NP239: return "Np-239";
            case PU239: return "Pu-239";
            default: return "Unknown";
        }
    }

    struct MicroCrossSection {
        std::array<double, Neutron::NUM_GROUPS> fission{};
        std::array<double, Neutron::NUM_GROUPS> capture{};
        std::array<double, Neutron::NUM_GROUPS> absorption{};
        std::array<std::array<double, Neutron::NUM_GROUPS>, Neutron::NUM_GROUPS> scattering{};
        MicroCrossSection() = default;
    };

    std::array<MicroCrossSection, NUM_NUCLIDES> microCrossSection{};
    std::array<double, NUM_NUCLIDES> decayConstant{};
    std::array<std::array<std::array<double, Neutron::NUM_GROUPS>, Neutron::NUM_GROUPS>, NUM_NUCLIDES> neutronYield{};

    Nuclide() = default;

    void setMicroCrossSection(std::size_t nuclide,
                              const std::array<double, Neutron::NUM_GROUPS>& microFissionCrossSections,
                              const std::array<double, Neutron::NUM_GROUPS>& microCaptureCrossSections,
                              const std::array<std::array<double, Neutron::NUM_GROUPS>, Neutron::NUM_GROUPS>& microScatteringCrossSections)
    {
        microCrossSection[nuclide].fission = microFissionCrossSections;
        microCrossSection[nuclide].capture = microCaptureCrossSections;
        microCrossSection[nuclide].scattering = microScatteringCrossSections;

        for (std::size_t ngi = 0; ngi < Neutron::NUM_GROUPS; ++ngi) {
            microCrossSection[nuclide].absorption[ngi] = microFissionCrossSections[ngi] + microCaptureCrossSections[ngi];
        }
    }

    void setDecayConstant(std::size_t nuclide, double halfLife) {
        assert(halfLife > 0.0);
        decayConstant[nuclide] = !std::isinf(halfLife) ? std::numbers::ln2 / halfLife : 0.0;
    }

    void setNeutronYield(std::size_t nuclide,
                         const std::array<std::array<double, Neutron::NUM_GROUPS>, Neutron::NUM_GROUPS>& neutronYields)
    {
        neutronYield[nuclide] = neutronYields;
    }
};

#endif // NUCLIDE_H
