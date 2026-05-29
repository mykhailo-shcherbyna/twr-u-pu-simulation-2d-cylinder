# twr-u-pu-simulation-2d-cylinder

A high-performance 2D cylindrical numerical simulation engine designed to model the self-sustaining breeding-burning wave propagation in Traveling Wave Reactors (TWR). Written in C++ and accelerated using OpenMP parallelization.

## Overview

This engine solves the multi-group neutron diffusion equations coupled with isotope transmutation kinetics to simulate the spatial and temporal evolution of a traveling wave reactor. Starting from a localized ignition zone, the reactor breeds its own fissile fuel ($^{239}\text{Pu}$) from fertile material ($^{238}\text{U}$) ahead of the active burning zone, creating a self-contained, propagating wave.

## Core Features

- **Physics Engine:** Multi-group 2D cylindrical $(r, z)$ neutron diffusion solver.
- **Kinetics Tracking:** Dynamically computes transmutation and decay chains for the $^{238}\text{U} \rightarrow {}^{239}\text{Pu}$ breeding cycle.
- **Parallelization:** Parallelized spatial mesh loops utilizing OpenMP directives for optimized multi-core execution.
- **Data Architecture:** Fully object-oriented layout featuring custom `Grid`, `Cylinder`, `Cell`, and `Field` structural containers.
- **Modern I/O:** High-speed data exporting using structured HDF5 files (`.h5`) for robust analysis.

## Physics Model

The code models the classic Uranium-Plutonium breeding chain:

$$\text{n} + {}^{238}_{92}\text{U} \xrightarrow{} {}^{239}_{92}\text{U} \xrightarrow{\beta^-} {}^{239}_{93}\text{Np} \xrightarrow{\beta^-} {}^{239}_{94}\text{Pu}$$

The spatial distribution of neutrons is handled via a multi-group diffusion approximation solved across a customized cylindrical grid system with vacuum boundary conditions.

## Project Structure

```text
twr-u-pu-simulation-2d-cylinder/
├── include/
│   ├── containers/
│   │   └── Field.h       # Memory buffers for spatial physics distributions
│   ├── geometry/
│   │   ├── Cell.h        # Grid cell properties and volumes
│   │   ├── Cylinder.h    # Core cylinder dimensions and geometry
│   │   └── Grid.h        # 2D cylindrical (r,z) mesh building and mapping
│   ├── physics/
│   │   ├── Fuel.h        # Nuclear fuel properties
│   │   ├── Neutron.h     # Flux properties
│   │   ├── Nuclide.h     # Nuclide properties
│   │   └── Precursor.h   # Delayed neutron precursors properties
│   └── solver/
│       └── Simulation.h  # Simulation engine and HDF5 I/O routines
├── src/
│   └── main.cpp          # Application entry point and simulation config
├── CMakeLists.txt        # Build configuration script
└── .gitignore            # Automatic exclusion mapping for binary data and HDF5s
```

## Getting Started

### Prerequisites

To build and run the simulation, you need a Linux environment (e.g., Debian/Ubuntu) with the following tools installed:

- A modern C++ compiler supporting C++17 or higher (GCC/Clang)
- CMake (version 3.10+)
- OpenMP development libraries
- HDF5 C++ development libraries

On Debian/Ubuntu systems, install the dependencies with:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libomp-dev libhdf5-dev
```

### Building the Project

1. Open your terminal in the project directory.
2. Create a compilation build workspace:
   ```bash
   mkdir build && cd build
   ```
3. Generate the build scripts explicitly configuring **Release Mode** to enable your high-performance flags (-O3, -march=native):
   ```bash
   cmake -DCMAKE_BUILD_TYPE=Release ..
   ```
4. Compile the target using all available CPU cores:
   ```bash
   make -j$(nproc)
   ```

### Running the Engine

Execute the compiled binary directly from the build folder:
```bash
./twr-u-pu-simulation-2d-cylinder
```
The application will execute the temporal stepping loops, tracking spatial flux and isotope burnup, dumping snapshots into `.h5` structure files sequentially per time interval.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
This software is provided purely for research and educational purposes.
