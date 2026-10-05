#pragma once

#include <algorithm>
#include <cstddef>
#include <istream>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

template <class PAR>
class Sample
{
protected:
    int m_size_x = 0;
    int m_size_y = 0;
    std::vector<PAR> m_data;

public:
    Sample() = default;
    Sample(const Sample& other) = default;
    virtual ~Sample() = default;

    void Resize(int size)
    {
        Resize(size, size);
    }

    void Resize(int size_x, int size_y)
    {
        if (size_x < 0 || size_y < 0)
        {
            throw std::invalid_argument("Sample::Resize: sizes must be non-negative");
        }

        const auto countX = static_cast<std::size_t>(size_x);
        const auto countY = static_cast<std::size_t>(size_y);

        if (countY != 0 && countX > m_data.max_size() / countY)
        {
            throw std::length_error("Sample::Resize: sample is too large");
        }

        m_data.resize(countX * countY);
        m_size_x = size_x;
        m_size_y = size_y;
    }

    int GetSize() const
    {
        return m_size_x;
    }

    int GetSizeX() const
    {
        return m_size_x;
    }

    int GetSizeY() const
    {
        return m_size_y;
    }

    PAR& operator()(int i, int j)
    {
        if (i < 0 || i >= m_size_x || j < 0 || j >= m_size_y)
        {
            throw std::out_of_range("Sample::operator(): index out of range");
        }

        return m_data[static_cast<std::size_t>(i) * static_cast<std::size_t>(m_size_y)
                      + static_cast<std::size_t>(j)];
    }

    const PAR& operator()(int i, int j) const
    {
        if (i < 0 || i >= m_size_x || j < 0 || j >= m_size_y)
        {
            throw std::out_of_range("Sample::operator(): index out of range");
        }

        return m_data[static_cast<std::size_t>(i) * static_cast<std::size_t>(m_size_y)
                      + static_cast<std::size_t>(j)];
    }

    bool IsSquare() const
    {
        return m_size_x == m_size_y;
    }

    bool IsZeroSize() const
    {
        return m_size_x == 0 || m_size_y == 0;
    }

    bool IsEqualSize(const Sample<PAR>& other) const
    {
        return m_size_x == other.m_size_x && m_size_y == other.m_size_y;
    }

    void SetValue(PAR value)
    {
        std::fill(m_data.begin(), m_data.end(), value);
    }

    PAR* GetPointer()
    {
        return m_data.empty() ? nullptr : m_data.data();
    }

    const PAR* GetPointer() const
    {
        return m_data.empty() ? nullptr : m_data.data();
    }

    PAR GetMax() const
    {
        if (m_data.empty())
        {
            throw std::runtime_error("Sample::GetMax: empty sample");
        }

        return *std::max_element(m_data.begin(), m_data.end());
    }

    Sample<PAR>& operator+=(PAR value)
    {
        for (auto& element : m_data)
        {
            element += value;
        }

        return *this;
    }

    Sample<PAR>& operator*=(PAR value)
    {
        for (auto& element : m_data)
        {
            element *= value;
        }

        return *this;
    }

    Sample<PAR>& operator/=(PAR value)
    {
        if (value == PAR{})
        {
            throw std::invalid_argument("Sample::operator/=: division by zero");
        }

        for (auto& element : m_data)
        {
            element /= value;
        }

        return *this;
    }

    Sample<PAR>& operator=(const Sample<PAR>& other)
    {
        if (this != &other)
        {
            std::vector<PAR> data(other.m_data);
            m_data.swap(data);
            m_size_x = other.m_size_x;
            m_size_y = other.m_size_y;
        }

        return *this;
    }

    Sample<PAR>& operator+=(const Sample<PAR>& other)
    {
        if (!IsEqualSize(other))
        {
            throw std::invalid_argument("Sample::operator+=: size mismatch");
        }

        for (std::size_t index = 0; index < m_data.size(); ++index)
        {
            m_data[index] += other.m_data[index];
        }

        return *this;
    }

    Sample<PAR>& operator-=(const Sample<PAR>& other)
    {
        if (!IsEqualSize(other))
        {
            throw std::invalid_argument("Sample::operator-=: size mismatch");
        }

        for (std::size_t index = 0; index < m_data.size(); ++index)
        {
            m_data[index] -= other.m_data[index];
        }

        return *this;
    }

    virtual Sample<PAR>& operator*=(const Sample<PAR>& other)
    {
        if (!IsEqualSize(other))
        {
            throw std::invalid_argument("Sample::operator*=: size mismatch");
        }

        for (std::size_t index = 0; index < m_data.size(); ++index)
        {
            m_data[index] *= other.m_data[index];
        }

        return *this;
    }

    std::ostream& Save(std::ostream& output) const
    {
        const auto originalPrecision = output.precision();

        if constexpr (std::is_floating_point_v<PAR>)
        {
            output.precision(std::numeric_limits<PAR>::max_digits10);
        }

        output << m_size_x << ' ' << m_size_y;

        for (const auto& element : m_data)
        {
            output << ' ' << element;
        }

        output << '\n';
        output.precision(originalPrecision);

        if (!output)
        {
            throw std::runtime_error("Sample::Save: write failed");
        }

        return output;
    }

    std::istream& Load(std::istream& input)
    {
        int sizeX = 0;
        int sizeY = 0;

        if (!(input >> sizeX >> sizeY))
        {
            throw std::runtime_error("Sample::Load: invalid header");
        }

        Sample<PAR> loaded;
        loaded.Resize(sizeX, sizeY);

        for (auto& element : loaded.m_data)
        {
            if (!(input >> element))
            {
                throw std::runtime_error("Sample::Load: invalid or missing element");
            }
        }

        m_data.swap(loaded.m_data);
        m_size_x = sizeX;
        m_size_y = sizeY;
        return input;
    }

protected:
    void CheckSquare() const
    {
        if (!IsSquare())
        {
            throw std::runtime_error("Sample::CheckSquare: sample is not square");
        }
    }
};
