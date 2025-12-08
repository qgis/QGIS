/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2002, Marios Hadjieleftheriou
 *
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

#include <spatialindex/SpatialIndex.h>

#include <cstring>
#include <limits>

using namespace SpatialIndex;

TimeRegion::TimeRegion()
	: Region(), m_startTime(-std::numeric_limits<double>::max()), m_endTime(std::numeric_limits<double>::max())
{
}

TimeRegion::TimeRegion(const double* pLow, const double* pHigh, const IInterval& ti, uint32_t dimension)
	: Region(pLow, pHigh, dimension), m_startTime(ti.getLowerBound()), m_endTime(ti.getUpperBound())
{
}

TimeRegion::TimeRegion(const double* pLow, const double* pHigh, double tStart, double tEnd, uint32_t dimension)
	: Region(pLow, pHigh, dimension), m_startTime(tStart), m_endTime(tEnd)
{
}

TimeRegion::TimeRegion(const Point& low, const Point& high, const IInterval& ti)
	: Region(low, high), m_startTime(ti.getLowerBound()), m_endTime(ti.getUpperBound())
{
}

TimeRegion::TimeRegion(const Point& low, const Point& high, double tStart, double tEnd)
	: Region(low, high), m_startTime(tStart), m_endTime(tEnd)
{
}

TimeRegion::TimeRegion(const Region& r, const IInterval& ti)
	: Region(r), m_startTime(ti.getLowerBound()), m_endTime(ti.getUpperBound())
{
}

TimeRegion::TimeRegion(const Region& r, double tStart, double tEnd)
	: Region(r), m_startTime(tStart), m_endTime(tEnd)
{
}

TimeRegion::TimeRegion(const TimePoint& low, const TimePoint& high)
    : Region( low, high), m_startTime(low.m_startTime), m_endTime(high.m_endTime)
{
}

TimeRegion::TimeRegion(const TimeRegion& r)
	: m_startTime(r.m_startTime), m_endTime(r.m_endTime)
{
	m_dimension = r.m_dimension;
	m_pLow = nullptr;

	try
	{
		m_pLow = new double[m_dimension];
		m_pHigh = new double[m_dimension];
	}
	catch (...)
	{
		delete[] m_pLow;
		throw;
	}

	memcpy(m_pLow, r.m_pLow, m_dimension * sizeof(double));
	memcpy(m_pHigh, r.m_pHigh, m_dimension * sizeof(double));
}

TimeRegion::~TimeRegion()
= default;

TimeRegion& TimeRegion::operator=(const TimeRegion& r)
{
	if(this != &r)
	{
		makeDimension(r.m_dimension);
		memcpy(m_pLow, r.m_pLow, m_dimension * sizeof(double));
		memcpy(m_pHigh, r.m_pHigh, m_dimension * sizeof(double));

		m_startTime = r.m_startTime;
		m_endTime = r.m_endTime;
	}

	return *this;
}

bool TimeRegion::operator==(const TimeRegion& r) const
{
	if (m_startTime < r.m_startTime - std::numeric_limits<double>::epsilon() ||
		m_startTime > r.m_startTime + std::numeric_limits<double>::epsilon() ||
		m_endTime < r.m_endTime - std::numeric_limits<double>::epsilon() ||
		m_endTime > r.m_endTime + std::numeric_limits<double>::epsilon())
		return false;

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (
			m_pLow[i] < r.m_pLow[i] - std::numeric_limits<double>::epsilon() ||
			m_pLow[i] > r.m_pLow[i] + std::numeric_limits<double>::epsilon() ||
			m_pHigh[i] < r.m_pHigh[i] - std::numeric_limits<double>::epsilon() ||
			m_pHigh[i] > r.m_pHigh[i] + std::numeric_limits<double>::epsilon())
			return false;
	}
	return true;
}

bool TimeRegion::intersectsRegionInTime(const TimeRegion& r) const
{
	// they should just intersect in time.
	// check if they intersect in time first.
	// the first check is needed for the following case:
	// m_endTime == m_startTime == r.m_startTime.
	// For open ended intervals this should be considered as an intersection
	// (takes care of degenarate intervals)
	//if (m_startTime != r.m_startTime &&
	//	(m_startTime >= r.m_endTime || m_endTime <= r.m_startTime))
	if (! intersectsInterval(r)) return false;
	return Region::intersectsRegion(r);
}

bool TimeRegion::containsRegionInTime(const TimeRegion& r) const
{
	// it should be contained in time.
	if (! containsInterval(r)) return false;
	return Region::containsRegion(r);
}

bool TimeRegion::touchesRegionInTime(const TimeRegion& r) const
{
	// they should just intersect in time.
	//if (m_startTime != r.m_startTime &&
	//	(m_startTime >= r.m_endTime || m_endTime <= r.m_startTime))
	if (!intersectsInterval(r)) return false;
	return Region::touchesRegion(r);
}

bool TimeRegion::containsPointInTime(const TimePoint& p) const
{
	// it should be contained in time.
	//if (p.m_startTime < m_startTime || p.m_endTime > m_endTime) return false;
	if (containsInterval(p)) return false;
	return Region::containsPoint(p);
}

bool TimeRegion::touchesPointInTime(const TimePoint& p) const
{
	// they should just intersect in time.
	//if (m_startTime != p.m_startTime &&
	//	(m_startTime >= p.m_endTime || m_endTime <= p.m_startTime))
	if (intersectsInterval(p)) return false;
	return Region::touchesPoint(p);
}

void TimeRegion::combineRegionInTime(const TimeRegion& r)
{
	Region::combineRegion(r);

	m_startTime = std::min(m_startTime, r.m_startTime);
	m_endTime = std::max(m_endTime, r.m_endTime);
}

void TimeRegion::getCombinedRegionInTime(TimeRegion& out, const TimeRegion& in) const
{
	Region::getCombinedRegion(out, in);

	out.m_startTime = std::min(m_startTime, in.m_startTime);
	out.m_endTime = std::max(m_endTime, in.m_endTime);
}

//
// IObject interface
//
TimeRegion* TimeRegion::clone()
{
	return new TimeRegion(*this);
}

//
// ISerializable interface
//
uint32_t TimeRegion::getByteArraySize()
{
	return (sizeof(uint32_t) + 2 * sizeof(double) + 2 * m_dimension * sizeof(double));
}

void TimeRegion::loadFromByteArray(const uint8_t* ptr)
{
	uint32_t dimension;

	memcpy(&dimension, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&m_startTime, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_endTime, ptr, sizeof(double));
	ptr += sizeof(double);

	makeDimension(dimension);
	memcpy(m_pLow, ptr, m_dimension * sizeof(double));
	ptr += m_dimension * sizeof(double);
	memcpy(m_pHigh, ptr, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

void TimeRegion::storeToByteArray(uint8_t** data, uint32_t& len)
{
	len = getByteArraySize();
	*data = new uint8_t[len];
	uint8_t* ptr = *data;

	memcpy(ptr, &m_dimension, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, &m_startTime, sizeof(double));
	ptr += sizeof(double);
	memcpy(ptr, &m_endTime, sizeof(double));
	ptr += sizeof(double);

	memcpy(ptr, m_pLow, m_dimension * sizeof(double));
	ptr += m_dimension * sizeof(double);
	memcpy(ptr, m_pHigh, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

//
// ITimeShape interface
//
bool TimeRegion::intersectsShapeInTime(const ITimeShape& in) const
{
	const TimeRegion* pr = dynamic_cast<const TimeRegion*>(&in);
	if (pr != nullptr) return intersectsRegionInTime(*pr);

	const TimePoint* ppt = dynamic_cast<const TimePoint*>(&in);
	if (ppt != nullptr) return containsPointInTime(*ppt);

	throw Tools::IllegalStateException("intersectsShapeInTime: Not implemented yet!");
}

bool TimeRegion::intersectsShapeInTime(const IInterval&, const ITimeShape&) const
{
	throw Tools::IllegalStateException("intersectsShapeInTime: Not implemented yet!");
}

bool TimeRegion::containsShapeInTime(const ITimeShape& in) const
{
	const TimeRegion* pr = dynamic_cast<const TimeRegion*>(&in);
	if (pr != nullptr) return containsRegionInTime(*pr);

	const TimePoint* ppt = dynamic_cast<const TimePoint*>(&in);
	if (ppt != nullptr) return containsPointInTime(*ppt);

	throw Tools::IllegalStateException("containsShapeInTime: Not implemented yet!");
}

bool TimeRegion::containsShapeInTime(const IInterval&, const ITimeShape&) const
{
	throw Tools::IllegalStateException("containsShapeInTime: Not implemented yet!");
}

bool TimeRegion::touchesShapeInTime(const ITimeShape& in) const
{
	const TimeRegion* pr = dynamic_cast<const TimeRegion*>(&in);
	if (pr != nullptr) return touchesRegionInTime(*pr);

	throw Tools::IllegalStateException("touchesShapeInTime: Not implemented yet!");
}

bool TimeRegion::touchesShapeInTime(const IInterval&, const ITimeShape&) const
{
	throw Tools::IllegalStateException("touchesShapeInTime: Not implemented yet!");
}

double TimeRegion::getAreaInTime() const
{
	throw Tools::IllegalStateException("getAreaInTime: Not implemented yet!");
}

double TimeRegion::getAreaInTime(const IInterval&) const
{
	throw Tools::IllegalStateException("getAreaInTime: Not implemented yet!");
}

double TimeRegion::getIntersectingAreaInTime(const ITimeShape&) const
{
	throw Tools::IllegalStateException("getIntersectingAreaInTime: Not implemented yet!");
}

double TimeRegion::getIntersectingAreaInTime(const IInterval&, const ITimeShape&) const
{
	throw Tools::IllegalStateException("getIntersectingAreaInTime: Not implemented yet!");
}

//
// IInterval interface
//
Tools::IInterval& TimeRegion::operator=(const Tools::IInterval& i)
{
	if (this != &i)
	{
		m_startTime = i.getLowerBound();
		m_endTime = i.getUpperBound();
	}

	return *this;
}

double TimeRegion::getLowerBound() const
{
	return m_startTime;
}

double TimeRegion::getUpperBound() const
{
	return m_endTime;
}

void TimeRegion::setBounds(double l, double h)
{
	assert(m_startTime <= m_endTime);

	m_startTime = l;
	m_endTime = h;
}

bool TimeRegion::intersectsInterval(const IInterval& ti) const
{
	return intersectsInterval(ti.getIntervalType(), ti.getLowerBound(), ti.getUpperBound());
}

bool TimeRegion::intersectsInterval(Tools::IntervalType, const double start, const double end) const
{
	//if (m_startTime != start &&
	//		(m_startTime >= end || m_endTime <= start)) return false;
	// this will not work for degenarate intervals.
	if (m_startTime >= end || m_endTime <= start) return false;

	return true;
}

bool TimeRegion::containsInterval(const IInterval& ti) const
{
	if (m_startTime <= ti.getLowerBound() && m_endTime >= ti.getUpperBound()) return true;
	return false;
}

Tools::IntervalType TimeRegion::getIntervalType() const
{
	return Tools::IT_RIGHTOPEN;
}

void TimeRegion::makeInfinite(uint32_t dimension)
{
	makeDimension(dimension);
	for (uint32_t cIndex = 0; cIndex < m_dimension; ++cIndex)
	{
		m_pLow[cIndex] = std::numeric_limits<double>::max();
		m_pHigh[cIndex] = -std::numeric_limits<double>::max();
	}

	m_startTime = std::numeric_limits<double>::max();
	m_endTime = -std::numeric_limits<double>::max();
}

void TimeRegion::makeDimension(uint32_t dimension)
{
	if (m_dimension != dimension)
	{
		m_dimension = dimension;

		delete[] m_pLow;
		delete[] m_pHigh;
		m_pLow = nullptr; m_pHigh = nullptr;

		m_pLow = new double[m_dimension];
		m_pHigh = new double[m_dimension];
	}
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const TimeRegion& r)
{
	uint32_t i;

	os << "Low: ";
	for (i = 0; i < r.m_dimension; ++i)
	{
		os << r.m_pLow[i] << " ";
	}

	os << ", High: ";

	for (i = 0; i < r.m_dimension; ++i)
	{
		os << r.m_pHigh[i] << " ";
	}

	os << ", Start: " << r.m_startTime << ", End: " << r.m_endTime;

	return os;
}
