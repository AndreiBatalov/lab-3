#pragma once

#include <algorithm>
#include <cstddef>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

class SampleDouble
{
protected:
    int m_size_x = 0;
    int m_size_y = 0;
    std::vector<double> m_data;

public:
    SampleDouble() = default;
    SampleDouble(const SampleDouble& other) = default;
    virtual ~SampleDouble() = default;

    void Resize(int size);

    void Resize(int size_x, int size_y);

    int GetSize() const;

    int GetSizeX() const;

    int GetSizeY() const;

    double& operator()(int i, int j);

    const double& operator()(int i, int j) const;

    bool IsSquare() const;

    bool IsZeroSize() const;

    bool IsEqualSize(const SampleDouble& other) const;

    void SetValue(double value);

    double* GetPointer();

    const double* GetPointer() const;

    double GetMax() const;

    SampleDouble& operator+=(double value);

    SampleDouble& operator*=(double value);

    SampleDouble& operator/=(double value);

    SampleDouble& operator=(const SampleDouble& other);

    SampleDouble& operator+=(const SampleDouble& other);

    SampleDouble& operator-=(const SampleDouble& other);

    virtual SampleDouble& operator*=(const SampleDouble& other);

    std::ostream& Save(std::ostream& output) const;

    std::istream& Load(std::istream& input);

protected:
    void CheckSquare() const;
};
