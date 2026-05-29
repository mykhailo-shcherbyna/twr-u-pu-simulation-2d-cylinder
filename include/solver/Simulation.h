#ifndef SIMULATION_H
#define SIMULATION_H

#include <omp.h>

#include <array>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <filesystem>

#include "containers/Field.h"

#include "physics/Fuel.h"
#include "physics/Neutron.h"
#include "physics/Nuclide.h"
#include "physics/Precursor.h"

#include "geometry/Grid.h"

template<std::size_t rN, std::size_t zN>
class Simulation {
    struct EquationSystem {
        Field<rN, zN> b{};
        Field<rN, zN> b0{};
        Field<rN, zN> alphaR{};
        Field<rN, zN> gammaR{};
        Field<rN, zN> alphaZ{};
        Field<rN, zN> gammaZ{};
        Field<rN, zN> beta{};
        Field<rN, zN> d{};
        EquationSystem() = default;
    };

    struct NumberDensity {
        std::array<Field<rN, zN>, Nuclide::NUM_NUCLIDES> nuclide{};
        std::array<std::array<Field<rN, zN>, Precursor::NUM_GROUPS>, Precursor::NUM_FAMILIES> precursor{};
        NumberDensity() = default;
    };

    class Flux {
        std::array<std::array<Field<rN, zN>, Neutron::NUM_GROUPS>, 2> data{};
        std::array<std::array<Field<rN, zN>, Neutron::NUM_GROUPS>*, 2> pointers{ &data[0], &data[1] };
    public:
        auto& operator()() { return *pointers[0]; }
        auto& oldState() { return *pointers[0]; }
        auto& newState() { return *pointers[1]; }

        const auto& operator()() const { return *pointers[0]; }
        const auto& oldState() const { return *pointers[0]; }
        const auto& newState() const { return *pointers[1]; }

        void swap() { std::swap(pointers[0], pointers[1]); }
    };

    struct Source {
        std::array<double, Neutron::NUM_GROUPS> neutron{};
        bool active{false};

        Source(std::array<double, Neutron::NUM_GROUPS> externalFluxes)
            : neutron(externalFluxes)
        {
            double totalFlux = 0.0;
            for (double f : externalFluxes) {
                totalFlux += f;
            }
            active = totalFlux > 0.0;
        }

        void turnOff() {
            active = false;
            neutron.fill(0.0);
        }
    };

    const Cylinder cylinder;
    const Fuel fuel;

    const double timeStep;
    const Grid<rN, zN> grid;
    const double extrapolatedLength;

    const Neutron neutrons;
    const Nuclide nuclides;
    const Precursor precursors;

    Flux flux{};
    NumberDensity numberDensity{};

    Source externalSource{};

    double time{};
    const double duration;
    const double sourceDuration;

    const double outputInterval;

    std::array<EquationSystem, Neutron::NUM_GROUPS> equationSystems{};
    std::array<Field<rN, zN>, Neutron::NUM_GROUPS> diffusionCoefficients{};

    inline void updateEquationSystems();
    inline void update();

    inline void initializeOutputFiles() const;
    inline void write() const;

    void initializeNumberDensity() {
        numberDensity.nuclide[Nuclide::U235].fill(fuel.nuclideNumberDensity(Fuel::U, 235));
        numberDensity.nuclide[Nuclide::U238].fill(fuel.nuclideNumberDensity(Fuel::U, 238));
    }

    bool isDepleted() const {
        const double fraction = 0.1; // 10%
        const double threshold = fraction * fuel.nuclideNumberDensity(Fuel::U, 238);
        return numberDensity.nuclide[Nuclide::U238](0, zN - 1) < threshold;
    }
/*
    void updateExternalSource() {
        if (time > sourceDuration) {
            source.turnOff();
        }
    }
*/
    void printSummary() const {
        std::cout << std::fixed << std::setprecision(4);

        std::cout << "========================================================\n";
        std::cout << "=              TWR SIMULATION PARAMETERS               =\n";
        std::cout << "========================================================\n";

        std::cout << "[Geometry & Grid]\n";
        std::cout << "  Radius:              " << cylinder.radius << " cm\n";
        std::cout << "  Height:              " << cylinder.height << " cm\n";
        std::cout << "  Extrapolated Length: " << extrapolatedLength << " cm\n";
        std::cout << "  Radial/Axial Layers: " << rN << " / " << zN << "\n";
        std::cout << "  Total Nodes:         " << rN * zN << "\n";
        std::cout << "  Radial Step (dr):    " << grid.rStep(0) << " cm\n";
        std::cout << "  Axial Step (dz):     " << grid.zStep(0) << " cm\n";

        std::cout << "\n[Time Domain]\n";
        std::cout << "  Time Step (dt):  " << timeStep << " s\n";
        std::cout << "  Source Duration: " << sourceDuration << " s (" << (sourceDuration / 86400.0) << " days)\n";
        std::cout << "  Total Duration:  " << duration << " s (" << (duration / 86400.0) << " days)\n";
        std::cout << "  Output Interval: " << outputInterval << " s (" << (outputInterval / 86400.0) << " days)\n";

        std::cout << "\n[Material & Physics]\n";
        std::cout << "  Fuel Type:        " << fuel.name << "\n";
        std::cout << "  Neutron Groups:   " << Neutron::NUM_GROUPS << "\n";
        std::cout << "  Nuclides :        " << Nuclide::NUM_NUCLIDES << "\n";
        std::cout << "  Precursor Groups: " << Precursor::NUM_FAMILIES * Precursor::NUM_GROUPS << "\n";

        std::cout << "\n[Execution Context]\n";
        std::cout << "  Parallel Compute: Enabled (OpenMP)\n";
        std::cout << "  Max Threads:      " << omp_get_max_threads() << "\n";

        std::cout << "========================================================\n" << std::endl;

        std::cout << std::defaultfloat;
    }

public:
    Simulation(const Fuel& simulationFuel,
               double simulationTimeStep,
               const Cylinder& simulationCylinder,
               double simulationExtrapolatedLength,
               const Neutron& simulationNeutrons,
               const Nuclide& simulationNuclides,
               const Precursor& simulationPrecursors,
               const std::array<double, Neutron::NUM_GROUPS>& simulationDiffusionCoefficients,
               const std::array<double, Neutron::NUM_GROUPS>& simulationExternalFluxes,
               double simulationDuration,
               double simulationSourceDuration,
               double simulationOutputInterval)
        : cylinder(simulationCylinder)
        , fuel(simulationFuel)
        , timeStep(simulationTimeStep)
        , grid(simulationCylinder)
        , extrapolatedLength(simulationExtrapolatedLength)
        , neutrons(simulationNeutrons)
        , nuclides(simulationNuclides)
        , precursors(simulationPrecursors)
        , externalSource(simulationExternalFluxes)
        , time(0.0)
        , duration(simulationDuration)
        , sourceDuration(simulationSourceDuration)
        , outputInterval(simulationOutputInterval)
    {
        initializeNumberDensity();

        for (std::size_t gi = 0; gi < Neutron::NUM_GROUPS; ++gi) {
            diffusionCoefficients[gi].fill(simulationDiffusionCoefficients[gi]);
        }

        printSummary();
        initializeOutputFiles();
    }

    inline void run();
};

template<std::size_t rN, std::size_t zN>
inline void Simulation<rN, zN>::initializeOutputFiles() const {
    std::filesystem::create_directories("results/Nuclides/");
    for (std::size_t ni = 0; ni < Nuclide::NUM_NUCLIDES; ++ni) {
        const std::string filename = "results/Nuclides/" + Nuclide::toString(ni) + ".h5";
        // H5F_ACC_TRUNC creates a new file or overwrites an existing one
        H5::H5File file(filename, H5F_ACC_TRUNC);
    }

    std::filesystem::create_directories("results/Neutrons/");
    for (std::size_t gi = 0; gi < Neutron::NUM_GROUPS; ++gi) {
        const std::string filename = "results/Neutrons/" + Neutron::toString(gi) + ".h5";
        H5::H5File file(filename, H5F_ACC_TRUNC);
    }

    std::filesystem::create_directories("results/Precursors/");
    for (std::size_t fi = 0; fi < Precursor::NUM_FAMILIES; ++fi) {
        for (std::size_t gi = 0; gi < Precursor::NUM_GROUPS; ++gi) {
            const std::string filename = "results/Precursors/" + Precursor::toString(fi, gi) + ".h5";
            H5::H5File file(filename, H5F_ACC_TRUNC);
        }
    }

    std::cout << "Output files are successfully initialized." << std::endl;
}

template<std::size_t rN, std::size_t zN>
inline void Simulation<rN, zN>::write() const {
    std::cout << ">>> Writing HDF5 output at time t = " << time / 86400.0 << " days..." << std::flush;

    for (std::size_t ni = 0; ni < Nuclide::NUM_NUCLIDES; ++ni) {
        const std::string filename = "results/Nuclides/" + Nuclide::toString(ni) + ".h5";
        numberDensity.nuclide[ni].write(filename, time);
    }

    for (std::size_t gi = 0; gi < Neutron::NUM_GROUPS; ++gi) {
        const std::string filename = "results/Neutrons/" + Neutron::toString(gi) + ".h5";
        flux.oldState()[gi].write(filename, time);
    }

    for (std::size_t fi = 0; fi < Precursor::NUM_FAMILIES; ++fi) {
        for (std::size_t gi = 0; gi < Precursor::NUM_GROUPS; ++gi) {
            const std::string filename = "results/Precursors/" + Precursor::toString(fi, gi) + ".h5";
            numberDensity.precursor[fi][gi].write(filename, time);
        }
    }

    std::cout << " Done." << std::endl;
}

template<std::size_t rN, std::size_t zN>
inline void Simulation<rN, zN>::run() {
    updateEquationSystems();

    time = 0.0;
    write();
    double lastOutputTime = 0.0;
    for (std::size_t ti = 1; time <= duration; ++ti) {
        if (time >= lastOutputTime + outputInterval) {
            write();
            lastOutputTime = time;
        }

        if (externalSource.active && time > sourceDuration) {
            externalSource.turnOff();
            updateEquationSystems();
        }

        update();
        time = ti * timeStep;

        if (isDepleted()) {
            std::cout << "\n>>> Wave reached the boundary at t = " << time << " s. Stopping simulation." << std::endl;
            write();
            break;
        }
    }
}

template<std::size_t rN, std::size_t zN>
inline void Simulation<rN, zN>::update() {
    struct MacroCrossSection {
        std::array<double, Neutron::NUM_GROUPS> fission{};
        std::array<double, Neutron::NUM_GROUPS> capture{};
        std::array<double, Neutron::NUM_GROUPS> absorption{};
        std::array<std::array<double, Neutron::NUM_GROUPS>, Neutron::NUM_GROUPS> scattering{};
    };

    auto solveImplicitEuler = [](double& N, double S, double L, double dt) {
        assert(N >= 0.0 && S >= 0.0 && L >= 0.0 && dt > 0.0);
        // If no source and no loss, density remains constant
        if (L <= 0.0 && S <= 0.0) return;
        N = (N + dt * S) / (1.0 + dt * L);
    };

    #pragma omp parallel for collapse(2)
    for (std::size_t zi = 0; zi < zN; ++zi) {
        for (std::size_t ri = 0; ri < rN; ++ri) {
            std::array<MacroCrossSection, Nuclide::NUM_NUCLIDES> macroCrossSections{};
            std::array<std::array<double, Neutron::NUM_GROUPS>, Nuclide::NUM_NUCLIDES> fissionRates{};

            std::array<double, Neutron::NUM_GROUPS> totalMacroAbsorptionCrossSection{};
            std::array<std::array<double, Neutron::NUM_GROUPS>, Neutron::NUM_GROUPS> totalMacroScatteringCrossSection{};

            // Cross Section and Fission Rate Update
            for (std::size_t ni = 0; ni < Nuclide::NUM_NUCLIDES; ++ni) {
                const double nuclideNumberDensity = numberDensity.nuclide[ni](ri, zi);

                for (std::size_t gi = 0; gi < Neutron::NUM_GROUPS; ++gi) {
                    const double macroFissionCrossSection = nuclideNumberDensity * nuclides.microCrossSection[ni].fission[gi];
                    macroCrossSections[ni].fission[gi] = macroFissionCrossSection;
                    fissionRates[ni][gi] = flux()[gi](ri, zi) * macroFissionCrossSection;

                    macroCrossSections[ni].capture[gi] = nuclideNumberDensity * nuclides.microCrossSection[ni].capture[gi];

                    const double macroAbsorptionCrossSection = nuclideNumberDensity * nuclides.microCrossSection[ni].absorption[gi];
                    macroCrossSections[ni].absorption[gi] = macroAbsorptionCrossSection;

                    totalMacroAbsorptionCrossSection[gi] += macroAbsorptionCrossSection;

                    for (std::size_t gj = 0; gj < Neutron::NUM_GROUPS; ++gj) {
                        const double macroScatteringCrossSection = nuclideNumberDensity * nuclides.microCrossSection[ni].scattering[gi][gj];
                        macroCrossSections[ni].scattering[gi][gj] = macroScatteringCrossSection;

                        totalMacroScatteringCrossSection[gi][gj] += macroScatteringCrossSection;
                    }
                }
            }

            // Precursor Update
            for (std::size_t pfi = 0; pfi < Precursor::NUM_FAMILIES; ++pfi) {
                const std::size_t ni = precursors.parent(pfi);

                double totalFissionRate = 0.0;
                for (std::size_t gi = 0; gi < Neutron::NUM_GROUPS; ++gi) {
                    totalFissionRate += fissionRates[ni][gi];
                }

                for (std::size_t pgi = 0; pgi < Precursor::NUM_GROUPS; ++pgi) {
                    const double sourceTerm = precursors.fraction[pfi][pgi] * totalFissionRate;
                    solveImplicitEuler(
                        numberDensity.precursor[pfi][pgi](ri, zi),
                        sourceTerm,
                        precursors.decayConstant[pfi][pgi],
                        timeStep
                    );
                }
            }

            std::array<double, Nuclide::NUM_NUCLIDES> nuclideLossCoefficients{};
            std::array<double, Nuclide::NUM_NUCLIDES> nuclideSourceTerms{};

            for (std::size_t ngi = 0; ngi < Neutron::NUM_GROUPS; ++ngi) {
                const double neutronFlux = flux()[ngi](ri, zi);
                for (std::size_t ni = 0; ni < Nuclide::NUM_NUCLIDES; ++ni) {
                    nuclideLossCoefficients[ni] += neutronFlux * nuclides.microCrossSection[ni].absorption[ngi];
                }
                // U238 -> U239 via capture
                nuclideSourceTerms[Nuclide::U239] += neutronFlux * macroCrossSections[Nuclide::U238].capture[ngi];
            }

            // Radioactive decay sources
            nuclideSourceTerms[Nuclide::NP239] = nuclides.decayConstant[Nuclide::U239] * numberDensity.nuclide[Nuclide::U239](ri, zi);
            nuclideSourceTerms[Nuclide::PU239] = nuclides.decayConstant[Nuclide::NP239] * numberDensity.nuclide[Nuclide::NP239](ri, zi);

            for (std::size_t ni = 0; ni < Nuclide::NUM_NUCLIDES; ++ni) {
                nuclideLossCoefficients[ni] += nuclides.decayConstant[ni];

                solveImplicitEuler(
                    numberDensity.nuclide[ni](ri, zi),
                    nuclideSourceTerms[ni],
                    nuclideLossCoefficients[ni],
                    timeStep
                );
            }

            // Neutron Flux Update
            for (std::size_t gi = 0; gi < Neutron::NUM_GROUPS; ++gi) {
                double promptSource = 0.0;
                double scatteringSource = 0.0;
                double delayedSource = 0.0;

                // Fission Source
                for (std::size_t ni = 0; ni < Nuclide::NUM_NUCLIDES; ++ni) {
                    for (std::size_t gj = 0; gj < Neutron::NUM_GROUPS; ++gj) {
                        promptSource += fissionRates[ni][gj] * nuclides.neutronYield[ni][gj][gi];
                    }
                }

                // Delayed Source
                for (std::size_t pfi = 0; pfi < Precursor::NUM_FAMILIES; ++pfi) {
                    const double delayedYield = precursors.neutronYield[pfi][gi];
                    for (std::size_t pgi = 0; pgi < Precursor::NUM_GROUPS; ++pgi) {
                        delayedSource += delayedYield * precursors.decayConstant[pfi][pgi] * numberDensity.precursor[pfi][pgi](ri, zi);
                    }
                }

                // In-Scattering Source
                for (std::size_t gj = 0; gj < Neutron::NUM_GROUPS; ++gj) {
                    if (gi != gj) {
                        scatteringSource += totalMacroScatteringCrossSection[gj][gi] * flux()[gj](ri, zi);
                    }
                }

                auto& eq = equationSystems[gi];
                eq.d(ri, zi) = promptSource + scatteringSource + delayedSource;

                // Bottom Boundary Condition (zi == 0)
                if (zi == 0 && externalSource.active) {
                    const Cell& cell = grid.cell(ri, 0);
                    eq.d(ri, zi) += externalSource.neutron[gi] / cell.zStep;
                }

                double totalMacroRemovalScatteringCrossSection = 0.0;
                for (std::size_t gj = 0; gj < Neutron::NUM_GROUPS; ++gj) {
                    if (gi != gj) {
                        totalMacroRemovalScatteringCrossSection += totalMacroScatteringCrossSection[gi][gj];
                    }
                }
                eq.b(ri, zi) = eq.b0(ri, zi) + totalMacroAbsorptionCrossSection[gi] + totalMacroRemovalScatteringCrossSection;

                // Neighbor contributions for Point Jacobi
                double neighbors = 0.0;
                if (ri > 0)      neighbors += eq.alphaR(ri, zi) * flux.oldState()[gi](ri - 1, zi);
                if (zi > 0)      neighbors += eq.alphaZ(ri, zi) * flux.oldState()[gi](ri, zi - 1);
                if (ri < rN - 1) neighbors += eq.gammaR(ri, zi) * flux.oldState()[gi](ri + 1, zi);
                if (zi < zN - 1) neighbors += eq.gammaZ(ri, zi) * flux.oldState()[gi](ri, zi + 1);

                flux.newState()[gi](ri, zi) = (eq.d(ri, zi) + eq.beta(ri, zi) * flux.oldState()[gi](ri, zi) + neighbors) / eq.b(ri, zi);

                assert(flux.newState()[gi](ri, zi) >= 0.0);
            }
        }
    }
    flux.swap();
}

template<std::size_t rN, std::size_t zN>
void Simulation<rN, zN>::updateEquationSystems() {
    auto harmonicMean = [](double d1, double d2) {
        return (d1 + d2 == 0.0) ? 0.0 : (2.0 * d1 * d2) / (d1 + d2);
    };

    for (std::size_t gi = 0; gi < Neutron::NUM_GROUPS; ++gi) {
        auto& eq = equationSystems[gi];
        const double V = neutrons.velocity[gi];

        for (std::size_t zi = 0; zi < zN; ++zi) {
            for (std::size_t ri = 0; ri < rN; ++ri) {
                const Cell& cell = grid.cell(ri, zi);
                const double D = diffusionCoefficients[gi](ri, zi);

                double aR = 0.0;
                if (ri > 0) {
                    const double DW = diffusionCoefficients[gi](ri - 1, zi);
                    aR = harmonicMean(D, DW) * cell.rAreas[Cell::LEFT] / grid.rStep(ri - 1);
                }

                const double DE = (ri < rN - 1) ? diffusionCoefficients[gi](ri + 1, zi) : 0.0;
                const double cR = (ri == rN - 1) ? D * cell.rAreas[Cell::RIGHT] / extrapolatedLength
                                                 : harmonicMean(D, DE) * cell.rAreas[Cell::RIGHT] / grid.rStep(ri);

                double aZ = 0.0;
                if (zi > 0) {
                    const double DS = diffusionCoefficients[gi](ri, zi - 1);
                    aZ = harmonicMean(D, DS) * cell.zArea / grid.zStep(zi - 1);
                }
                else if (!externalSource.active) {
                    // If source is NOT active, neutrons leak out the bottom (Vacuum BC)
                    aZ = D * cell.zArea / extrapolatedLength;
                }

                const double DN = (zi < zN - 1) ? diffusionCoefficients[gi](ri, zi + 1) : 0.0;
                const double cZ = (zi == zN - 1) ? (D * cell.zArea / extrapolatedLength)
                                                 : (harmonicMean(D, DN) * cell.zArea / grid.zStep(zi));

                eq.alphaR(ri, zi) = aR / cell.volume;
                eq.gammaR(ri, zi) = cR / cell.volume;
                eq.alphaZ(ri, zi) = aZ / cell.volume;
                eq.gammaZ(ri, zi) = cZ / cell.volume;

                eq.beta(ri, zi) = 1.0 / (V * timeStep);
                eq.b0(ri, zi) = (aR + cR + aZ + cZ) / cell.volume + 1.0 / (V * timeStep);
            }
        }
    }
}

#endif // SIMULATION_H
