#ifndef FUEL_H
#define FUEL_H

#include <unordered_map>
#include <stdexcept>
#include <iostream>
#include <string>
#include <vector>

struct Fuel {
    static constexpr double AVOGADRO_NUMBER = 6.02214076e23;

    enum Element {
        U = 0,
        NUM_ELEMENTS
    };

    static std::string toString(Element element) {
        switch (element) {
            case U:  return "U";
            default: return "?";
        }
    }

    using Fractions = std::unordered_map<std::size_t, double>;

    struct Component {
        Element element;
        double atomsPerMolecule;
        Fractions fractions;
    };

    const std::string name;
    const std::vector<Component> composition;
    const double numberDensity;

    Fuel(const std::string& name,
         const std::vector<Component>& composition,
         double molarMass,
         double density)
        : name(name)
        , composition(composition)
        , numberDensity((density / molarMass) * AVOGADRO_NUMBER)
    {
        if (molarMass <= 0.0) throw std::invalid_argument("Molar mass must be positive.");
        if (density <= 0.0) throw std::invalid_argument("Density must be positive.");
    }

    double nuclideNumberDensity(Element element, std::size_t massNumber) const {
        if (const Component* component = getComponent(element)) {
            double elementNumberDensity = component->atomsPerMolecule * numberDensity;

            auto it = component->fractions.find(massNumber);
            if (it != component->fractions.end()) {
                double fraction = it->second;
                return fraction * elementNumberDensity;
            }
        }
        return 0.0;
    }

private:
    const Component* getComponent(Element element) const {
        for (const auto& component : composition) {
            if (component.element == element) {
                return &component;
            }
        }
        return nullptr;
    }
};

#endif // FUEL_H
