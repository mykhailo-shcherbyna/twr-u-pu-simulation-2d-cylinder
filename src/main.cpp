#include <iostream>
#include <limits>

#include "solver/Simulation.h"

void sayWithCow(const std::string& message) {
    std::string command = "cowsay \"" + message + "\"";
    const int status = std::system(command.c_str());

    if (status != 0) {
        std::cerr << "Error: cowsay failed to say \"" << message << "\"" << std::endl;
    }
}

int main() {
    sayWithCow("Hello, Travelling Wave Reactor!");

    const Fuel fuel(
        "Uranium Carbide", // name
        {
            {Fuel::U, 1.0, {{235, 0.05}, {238, 0.95}}} // Component: Element U, 1 atom per molecule, 5% U-235 enrichment
        },
        250.04, // Molar mass of UC (g/mol)
        13.63   // Density of UC (g/cm^3)
    );

    // Neutron Setup
    Neutron neutrons{};
    neutrons.setVelocity(Neutron::EPITHERMAL, 1.0e6);

    // Nuclide Setup
    Nuclide nuclides{};

    // U-235: Fissile
    nuclides.setMicroCrossSection(Nuclide::U235, {200.0e-24}, {0.0e-24}, {{{1.0}}});
    nuclides.setDecayConstant(Nuclide::U235, std::numeric_limits<double>::infinity());
    nuclides.setNeutronYield(Nuclide::U235, {{{2.43}}});

    // U-238: Fertile
    nuclides.setMicroCrossSection(Nuclide::U238, {0.0e-24}, {250.0e-24}, {{{1.0}}});
    nuclides.setDecayConstant(Nuclide::U238, std::numeric_limits<double>::infinity());

    // U-239: Short-lived Intermediate
    nuclides.setMicroCrossSection(Nuclide::U239, {0.0e-24}, {0.0e-24}, {{{1.0}}});
    nuclides.setDecayConstant(Nuclide::U239, 23.5 * 60.0); // 23.5 minutes

    // Np-239: Intermediate
    nuclides.setMicroCrossSection(Nuclide::NP239, {0.0e-24}, {0.0e-24}, {{{1.0}}});
    nuclides.setDecayConstant(Nuclide::NP239, 2.356 * 24.0 * 3600.0); // 2.356 days

    // Pu-239: Produced Fissile Fuel
    nuclides.setMicroCrossSection(Nuclide::PU239, {500.0e-24}, {0.0e-24}, {{{1.0}}});
    nuclides.setDecayConstant(Nuclide::PU239, std::numeric_limits<double>::infinity());
    nuclides.setNeutronYield(Nuclide::PU239, {{{2.9}}});

    // Precursor Setup
    Precursor precursors{};

    // Family 1: U-235
    precursors.setDecayConstant(Precursor::U235, {55.72, 22.72, 6.22, 2.30, 0.61, 0.23});
    precursors.setFraction(Precursor::U235, {0.00021, 0.00140, 0.00126, 0.00252, 0.00074, 0.00027});
    precursors.setNeutronYield(Precursor::U235, {1.0});

    // Family 2: Pu-239
    precursors.setDecayConstant(Precursor::PU239, {54.28, 23.04, 5.60, 2.13, 0.618, 0.257});
    precursors.setFraction(Precursor::PU239, {0.00007, 0.00063, 0.00044, 0.00068, 0.00018, 0.00009});
    precursors.setNeutronYield(Precursor::PU239, {1.0});

    // Define Geometry
    const Cylinder core(5.0, 10.0);

    // Instantiate and Run the Simulation
    Simulation<51, 101> simulation(
        fuel,
        1.0, // time step (s)
        core, // cylinder
        1.0, // extrapolated length (cm)
        neutrons, // neutron setup
        nuclides, // nuclide setup
        precursors, // precursor setup
        {0.1}, // diffusion coefficients (cm)
        {1e15}, // external fluxes (neutrons/cm2/s)
        1000.0 * 86400.0, // duration (s)
        30.0 * 86400.0, // source duration (s)
        0.1 * 86400.0 // output interval (s)
    );

    simulation.run();

    return 0;
}
