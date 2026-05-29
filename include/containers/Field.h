#ifndef FIELD_H
#define FIELD_H

#include <filesystem>
#include <iostream>
#include <cstddef>
#include <string>
#include <array>

#include <H5Cpp.h>

template<std::size_t rN, std::size_t zN>
class Field {
    std::array<double, rN * zN> data{};

public:
    Field() = default;

    double& operator()(std::size_t ri, std::size_t zi) {
        return data[zi * rN + ri];
    }

    double operator()(std::size_t ri, std::size_t zi) const {
        return data[zi * rN + ri];
    }

    void fill(double value) {
        data.fill(value);
    }

    inline void write(const std::string& filename, double time) const;
};

template<std::size_t rN, std::size_t zN>
void Field<rN, zN>::write(const std::string& filename, double time) const {
    try {
        H5::H5File file;
        if (std::filesystem::exists(filename)) {
            file = H5::H5File(filename, H5F_ACC_RDWR);
        } else {
            file = H5::H5File(filename, H5F_ACC_TRUNC);
        }

        std::array<hsize_t, 2> dims = {zN, rN};
        H5::DataSpace dataspace(2, dims.data());

        H5::DataSet dataset = file.createDataSet(std::to_string(time), H5::PredType::NATIVE_DOUBLE, dataspace);
        dataset.write(data.data(), H5::PredType::NATIVE_DOUBLE);

        file.close();
    }
    catch (const H5::Exception& error) {
        std::cerr << "Error while writing field data: " << error.getDetailMsg() << std::endl;
    }
}

template<std::size_t rN, std::size_t zN>
std::ostream& operator<<(std::ostream& os, const Field<rN, zN>& field) {
    for (std::size_t zi = 0; zi < zN; ++zi) {
        for (std::size_t ri = 0; ri < rN; ++ri) {
            os << field(ri, zi) << "  ";
        }
        os << std::endl;
    }

    os << std::flush;
    return os;
}

#endif // FIELD_H
