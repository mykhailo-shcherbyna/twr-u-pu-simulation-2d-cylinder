#ifndef NEUTRON_H
#define NEUTRON_H

#include <cstddef>
#include <array>

struct Neutron {
    enum : std::size_t {
        EPITHERMAL = 0,
        NUM_GROUPS
    };

    static std::string toString(std::size_t index) {
        switch (index) {
            case EPITHERMAL: return "Epithermal";
            default: return "Unknown";
        }
    }

    std::array<double, NUM_GROUPS> velocity{};

    Neutron() = default;

    void setVelocity(std::size_t group, double velocity) {
        this->velocity.at(group) = velocity;
    }
};

#endif // NEUTRON_H
