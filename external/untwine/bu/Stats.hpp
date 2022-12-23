#pragma once

#include <unordered_map>
#include <vector>

#include <pdal/Dimension.hpp>

#include "../untwine/Common.hpp"

namespace untwine
{

class Stats
{
public:
    enum EnumType
    {
        NoEnum,
        Enumerate,
        Count,
        Global
    };

using EnumMap = std::unordered_map<double, PointCount>;
using DataVector = std::vector<double>;

public:
    Stats(std::string name, EnumType enumerate, bool advanced = true) :
        m_name(name), m_enumerate(enumerate), m_advanced(advanced)
    { reset(); }

    // Merge another summary with this one. 'name', 'enumerate' and 'advanced' must match
    // or false is returns and no merge occurs.
    bool merge(const Stats& s);
    double minimum() const
        { return m_min; }
    double maximum() const
        { return m_max; }
    double average() const
        { return M1; }
    double populationVariance() const
        { return M2 / m_cnt; }
    double sampleVariance() const
        { return M2 / (m_cnt - 1.0); }
    double variance() const
        { return sampleVariance(); }
    double populationStddev() const
        { return std::sqrt(populationVariance()); }
    double sampleStddev() const
        { return std::sqrt(sampleVariance()); }
    double stddev() const
        { return sampleStddev(); }
    double populationSkewness() const
    {
        if (!M2 || ! m_advanced)
            return 0;
        return std::sqrt(double(m_cnt)) * M3 / std::pow(M2, 1.5);
    }
    double sampleSkewness() const
    {
        if (M2 == 0 || m_cnt <= 2 || !m_advanced)
            return 0.0;
        double c((double)m_cnt);
        return populationSkewness() * std::sqrt(c) * std::sqrt(c - 1) / (c - 2);
    }
    double skewness() const
    {
        return sampleSkewness();
    }
    double populationKurtosis() const
    {
        if (M2 == 0 || !m_advanced)
            return 0;
        return double(m_cnt) * M4 / (M2 * M2);
    }
    double populationExcessKurtosis() const
    {
        if (M2 == 0 || !m_advanced)
            return 0;
        return populationKurtosis() - 3;
    }
    double sampleKurtosis() const
    {
        if (M2 == 0 || m_cnt <= 3 || !m_advanced)
            return 0;
        double c((double)m_cnt);
        return populationKurtosis() * (c + 1) * (c - 1) / ((c - 2) * (c - 3));
    }
    double sampleExcessKurtosis() const
    {
        if (M2 == 0 || m_cnt <= 3 || !m_advanced)
            return 0;
        double c((double)m_cnt);
        return sampleKurtosis() - 3 * (c - 1) * (c - 1) / ((c - 2) * (c - 3));
    }
    double kurtosis() const
    {
        return sampleExcessKurtosis();
    }
    double median() const
        { return m_median; }
    double mad() const
        { return m_mad; }
    PointCount count() const
        { return m_cnt; }
    std::string name() const
        { return m_name; }
    const EnumMap& values() const
        { return m_values; }

    void computeGlobalStats();

    void reset()
    {
        m_max = (std::numeric_limits<double>::lowest)();
        m_min = (std::numeric_limits<double>::max)();
        m_cnt = 0;
        m_median = 0.0;
        m_mad = 0.0;
        M1 = M2 = M3 = M4 = 0.0;
    }

    void insert(double value)
    {
        m_cnt++;
        m_min = (std::min)(m_min, value);
        m_max = (std::max)(m_max, value);

        if (m_enumerate != NoEnum)
            m_values[value]++;
        if (m_enumerate == Global)
        {
            if (m_data.capacity() - m_data.size() < 10000)
                m_data.reserve(m_data.capacity() + m_cnt);
            m_data.push_back(value);
        }

        // stolen from http://www.johndcook.com/blog/skewness_kurtosis/

        PointCount n(m_cnt);

        // Difference from the mean
        double delta = value - M1;
        // Portion that this point's difference from the mean contributes
        // to the mean.
        double delta_n = delta / n;
        double term1 = delta * delta_n * (n - 1);

        // First moment - average.
        M1 += delta_n;

        if (m_advanced)
        {
            double delta_n2 = pow(delta_n, 2.0);
            // Fourth moment - kurtosis (sum part)
            M4 += term1 * delta_n2 * (n*n - 3*n + 3) +
                (6 * delta_n2 * M2) - (4 * delta_n * M3);
            // Third moment - skewness (sum part)
            M3 += term1 * delta_n * (n - 2) - 3 * delta_n * M2;
        }
        // Second moment - variance (sum part)
        M2 += term1;
    }

private:
    std::string m_name;
    EnumType m_enumerate;
    bool m_advanced;
    double m_max;
    double m_min;
    double m_mad;
    double m_median;
    EnumMap m_values;
    DataVector m_data;
    PointCount m_cnt;
    double M1, M2, M3, M4;
};

using IndexedStats = std::vector<std::pair<pdal::Dimension::Id, Stats>>;
using StatsMap = std::unordered_map<pdal::Dimension::Id, Stats>;
} // namespace untwine

namespace std
{
    template<> struct hash<pdal::Dimension::Id>
    {
        std::size_t operator()(const pdal::Dimension::Id& id) const noexcept
        {
            return std::hash<int>()((int)id);
        }
    };
}

