#include "sample_double.h"

void SampleDouble::Resize(int size)
{
    Resize(size, size);
}

void SampleDouble::Resize(int size_x, int size_y)
{
    if (size_x < 0 || size_y < 0)
    {
        throw std::invalid_argument("SampleDouble::Resize: sizes must be non-negative");
    }

    const auto countX = static_cast<std::size_t>(size_x);
    const auto countY = static_cast<std::size_t>(size_y);

    if (countY != 0 && countX > m_data.max_size() / countY)
    {
        throw std::length_error("SampleDouble::Resize: sample is too large");
    }

    m_data.resize(countX * countY);
    m_size_x = size_x;
    m_size_y = size_y;
}

int SampleDouble::GetSize() const
{
    return m_size_x;
}

int SampleDouble::GetSizeX() const
{
    return m_size_x;
}

int SampleDouble::GetSizeY() const
{
    return m_size_y;
}

double& SampleDouble::operator()(int i, int j)
{
    if (i < 0 || i >= m_size_x || j < 0 || j >= m_size_y)
    {
        throw std::out_of_range("SampleDouble::operator(): index out of range");
    }

    return m_data[static_cast<std::size_t>(i) * static_cast<std::size_t>(m_size_y)
                  + static_cast<std::size_t>(j)];
}

const double& SampleDouble::operator()(int i, int j) const
{
    if (i < 0 || i >= m_size_x || j < 0 || j >= m_size_y)
    {
        throw std::out_of_range("SampleDouble::operator(): index out of range");
    }

    return m_data[static_cast<std::size_t>(i) * static_cast<std::size_t>(m_size_y)
                  + static_cast<std::size_t>(j)];
}

bool SampleDouble::IsSquare() const
{
    return m_size_x == m_size_y;
}

bool SampleDouble::IsZeroSize() const
{
    return m_size_x == 0 || m_size_y == 0;
}

bool SampleDouble::IsEqualSize(const SampleDouble& other) const
{
    return m_size_x == other.m_size_x && m_size_y == other.m_size_y;
}

void SampleDouble::SetValue(double value)
{
    std::fill(m_data.begin(), m_data.end(), value);
}

double* SampleDouble::GetPointer()
{
    return m_data.empty() ? nullptr : m_data.data();
}

const double* SampleDouble::GetPointer() const
{
    return m_data.empty() ? nullptr : m_data.data();
}

double SampleDouble::GetMax() const
{
    if (m_data.empty())
    {
        throw std::runtime_error("SampleDouble::GetMax: empty sample");
    }

    return *std::max_element(m_data.begin(), m_data.end());
}

SampleDouble& SampleDouble::operator+=(double value)
{
    for (auto& element : m_data)
    {
        element += value;
    }

    return *this;
}

SampleDouble& SampleDouble::operator*=(double value)
{
    for (auto& element : m_data)
    {
        element *= value;
    }

    return *this;
}

SampleDouble& SampleDouble::operator/=(double value)
{
    if (value == double{})
    {
        throw std::invalid_argument("SampleDouble::operator/=: division by zero");
    }

    for (auto& element : m_data)
    {
        element /= value;
    }

    return *this;
}

SampleDouble& SampleDouble::operator=(const SampleDouble& other)
{
    if (this != &other)
    {
        std::vector<double> data(other.m_data);
        m_data.swap(data);
        m_size_x = other.m_size_x;
        m_size_y = other.m_size_y;
    }

    return *this;
}

SampleDouble& SampleDouble::operator+=(const SampleDouble& other)
{
    if (!IsEqualSize(other))
    {
        throw std::invalid_argument("SampleDouble::operator+=: size mismatch");
    }

    for (std::size_t index = 0; index < m_data.size(); ++index)
    {
        m_data[index] += other.m_data[index];
    }

    return *this;
}

SampleDouble& SampleDouble::operator-=(const SampleDouble& other)
{
    if (!IsEqualSize(other))
    {
        throw std::invalid_argument("SampleDouble::operator-=: size mismatch");
    }

    for (std::size_t index = 0; index < m_data.size(); ++index)
    {
        m_data[index] -= other.m_data[index];
    }

    return *this;
}

SampleDouble& SampleDouble::operator*=(const SampleDouble& other)
{
    if (!IsEqualSize(other))
    {
        throw std::invalid_argument("SampleDouble::operator*=: size mismatch");
    }

    for (std::size_t index = 0; index < m_data.size(); ++index)
    {
        m_data[index] *= other.m_data[index];
    }

    return *this;
}

std::ostream& SampleDouble::Save(std::ostream& output) const
{
    const auto originalPrecision = output.precision();

    if constexpr (std::is_floating_point_v<double>)
    {
        output.precision(std::numeric_limits<double>::max_digits10);
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
        throw std::runtime_error("SampleDouble::Save: write failed");
    }

    return output;
}

std::istream& SampleDouble::Load(std::istream& input)
{
    int sizeX = 0;
    int sizeY = 0;

    if (!(input >> sizeX >> sizeY))
    {
        throw std::runtime_error("SampleDouble::Load: invalid header");
    }

    SampleDouble loaded;
    loaded.Resize(sizeX, sizeY);

    for (auto& element : loaded.m_data)
    {
        if (!(input >> element))
        {
            throw std::runtime_error("SampleDouble::Load: invalid or missing element");
        }
    }

    m_data.swap(loaded.m_data);
    m_size_x = sizeX;
    m_size_y = sizeY;
    return input;
}

void SampleDouble::CheckSquare() const
{
    if (!IsSquare())
    {
        throw std::runtime_error("SampleDouble::CheckSquare: sample is not square");
    }
}
