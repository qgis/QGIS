
#include "Stats.hpp"

#include <cmath>

namespace untwine
{

void Stats::computeGlobalStats()
{
    auto compute_median = [](std::vector<double> vals)
    {
        std::nth_element(vals.begin(), vals.begin() + vals.size() / 2, vals.end());
        return *(vals.begin() + vals.size() / 2);
    };

    // TODO add quantiles
    m_median = compute_median(m_data);
    std::transform(m_data.begin(), m_data.end(), m_data.begin(),
       [this](double v) { return std::fabs(v - this->m_median); });
    m_mad = compute_median(m_data);
}

// Math comes from https://prod.sandia.gov/techlib-noauth/access-control.cgi/2008/086212.pdf
// (Pebay paper from Sandia labs, 2008)
bool Stats::merge(const Stats& s)
{
    if ((m_name != s.m_name) || (m_enumerate != s.m_enumerate) || (m_advanced != s.m_advanced))
        return false;

    double n1 = (double)m_cnt;
    double n2 = (double)s.m_cnt;
    double n = n1 + n2;
    double nsq = n * n;
    double n1n2 = (double)m_cnt * s.m_cnt;
    double n1sq = n1 * n1;
    double n2sq = n2 * n2;
    double ncube = n * n * n;
    double deltaMean = s.M1 - M1;

    if (n == 0)
        return true;

    double m1 = M1 + s.m_cnt * deltaMean / n;
    double m2 = M2 + s.M2 + n1n2 * std::pow(deltaMean, 2) / n;
    double m3 = M3 + s.M3 + n1n2 * (n1 - n2) * std::pow(deltaMean, 3) / nsq +
        3 * (n1 * s.M2 - n2 * M2) * deltaMean / n;
    double m4 = M4 + s.M4 +
        n1n2 * (n1sq - n1n2 + n2sq) * std::pow(deltaMean, 4) / ncube +
        6 * (n1sq * s.M2 + n2sq * M2) * std::pow(deltaMean, 2) / nsq +
        4 * (n1 * s.M3 - n2 * M3) * deltaMean / n;

    M1 = m1;
    M2 = m2;
    M3 = m3;
    M4 = m4;
    m_min = (std::min)(m_min, s.m_min);
    m_max = (std::max)(m_max, s.m_max);
    m_cnt = s.m_cnt + m_cnt;
    m_data.insert(m_data.begin(), s.m_data.begin(), s.m_data.end());
    for (auto p : s.m_values)
        m_values[p.first] += p.second;

    return true;
}

} // namespace untwine
