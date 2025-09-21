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

#include <cstring>
#include <limits>

#include <spatialindex/SpatialIndex.h>

using namespace SpatialIndex;

TimePoint::TimePoint()
	: Point(), m_startTime(-std::numeric_limits<double>::max()), m_endTime(std::numeric_limits<double>::max())
{
}

TimePoint::TimePoint(const double* pCoords, const IInterval& ti, uint32_t dimension)
	: Point(pCoords, dimension), m_startTime(ti.getLowerBound()), m_endTime(ti.getUpperBound())
{
}

TimePoint::TimePoint(const double* pCoords, double tStart, double tEnd, uint32_t dimension)
	: Point(pCoords, dimension), m_startTime(tStart), m_endTime(tEnd)
{
}

TimePoint::TimePoint(const Point& p, const IInterval& ti)
	: Point(p), m_startTime(ti.getLowerBound()), m_endTime(ti.getUpperBound())
{
}

TimePoint::TimePoint(const Point& p, double tStart, double tEnd)
	: Point(p), m_startTime(tStart), m_endTime(tEnd)
{
}

TimePoint::TimePoint(const TimePoint& p)
	: m_startTime(p.m_startTime), m_endTime(p.m_endTime)
{
	m_dimension = p.m_dimension;

	m_pCoords = new double[m_dimension];
	memcpy(m_pCoords, p.m_pCoords, m_dimension * sizeof(double));
}

TimePoint::~TimePoint()
= default;

TimePoint& TimePoint::operator=(const TimePoint& p)
{
	if (this != &p)
	{
		makeDimension(p.m_dimension);
		memcpy(m_pCoords, p.m_pCoords, m_dimension * sizeof(double));
		m_startTime = p.m_startTime;
		m_endTime = p.m_endTime;
	}

	return *this;
}

bool TimePoint::operator==(const TimePoint& p) const
{
	if (
		m_startTime < p.m_startTime - std::numeric_limits<double>::epsilon() ||
		m_startTime > p.m_startTime + std::numeric_limits<double>::epsilon() ||
		m_endTime < p.m_endTime - std::numeric_limits<double>::epsilon() ||
		m_endTime > p.m_endTime + std::numeric_limits<double>::epsilon())
		return false;

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		if (
			m_pCoords[cDim] < p.m_pCoords[cDim] - std::numeric_limits<double>::epsilon() ||
			m_pCoords[cDim] > p.m_pCoords[cDim] + std::numeric_limits<double>::epsilon()) 
			return false;
	}

	return true;
}

//
// IObject interface
//
TimePoint* TimePoint::clone()
{
	return new TimePoint(*this);
}

//
// ISerializable interface
//
uint32_t TimePoint::getByteArraySize()
{
	return (sizeof(uint32_t) + 2 * sizeof(double) + m_dimension * sizeof(double));
}

void TimePoint::loadFromByteArray(const uint8_t* ptr)
{
	uint32_t dimension;
	memcpy(&dimension, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(&m_startTime, ptr, sizeof(double));
	ptr += sizeof(double);
	memcpy(&m_endTime, ptr, sizeof(double));
	ptr += sizeof(double);

	makeDimension(dimension);
	memcpy(m_pCoords, ptr, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

void TimePoint::storeToByteArray(uint8_t** data, uint32_t& len)
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
	memcpy(ptr, m_pCoords, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

//
// ITimeShape interface
//
bool TimePoint::intersectsShapeInTime(const ITimeShape& in) const
{
	const TimeRegion* pr = dynamic_cast<const TimeRegion*>(&in);
	if (pr != nullptr) return pr->containsPointInTime(*this);

	throw Tools::IllegalStateException("intersectsShapeInTime: Not implemented yet!");
}

bool TimePoint::intersectsShapeInTime(const IInterval&, const ITimeShape&) const
{
	throw Tools::IllegalStateException("intersectsShapeInTime: Not implemented yet!");
}

bool TimePoint::containsShapeInTime(const ITimeShape&) const
{
	return false;
}

bool TimePoint::containsShapeInTime(const IInterval&, const ITimeShape&) const
{
	return false;
}

bool TimePoint::touchesShapeInTime(const ITimeShape&) const
{
	throw Tools::IllegalStateException("touchesShapeInTime: Not implemented yet!");
}

bool TimePoint::touchesShapeInTime(const IInterval&, const ITimeShape&) const
{
	throw Tools::IllegalStateException("touchesShapeInTime: Not implemented yet!");
}

double TimePoint::getAreaInTime() const
{
	return 0.0;
}

double TimePoint::getAreaInTime(const IInterval&) const
{
	return 0.0;
}

double TimePoint::getIntersectingAreaInTime(const ITimeShape&) const
{
	return 0.0;
}

double TimePoint::getIntersectingAreaInTime(const IInterval&, const ITimeShape&) const
{
	return 0.0;
}

//
// IInterval interface
//
Tools::IInterval& TimePoint::operator=(const Tools::IInterval& i)
{
	if (this != &i)
	{
		m_startTime = i.getLowerBound();
		m_endTime = i.getUpperBound();
	}

	return *this;
}

double TimePoint::getLowerBound() const
{
	return m_startTime;
}

double TimePoint::getUpperBound() const
{
	return m_endTime;
}

void TimePoint::setBounds(double l, double h)
{
	assert(l <= h);

	m_startTime = l;
	m_endTime = h;
}

bool TimePoint::intersectsInterval(const IInterval& ti) const
{
	return intersectsInterval(ti.getIntervalType(), ti.getLowerBound(), ti.getUpperBound());
}

bool TimePoint::intersectsInterval(Tools::IntervalType, const double start, const double end) const
{
	//if (m_startTime != start &&
	//		(m_startTime >= end || m_endTime <= start)) return false;
	if (m_startTime >= end || m_endTime <= start) return false;

	return true;
}

bool TimePoint::containsInterval(const IInterval& ti) const
{
	if (m_startTime <= ti.getLowerBound() && m_endTime >= ti.getUpperBound()) return true;
	return false;
}

Tools::IntervalType TimePoint::getIntervalType() const
{
	return Tools::IT_RIGHTOPEN;
}

void TimePoint::makeInfinite(uint32_t dimension)
{
	makeDimension(dimension);
	for (uint32_t cIndex = 0; cIndex < m_dimension; ++cIndex)
	{
		m_pCoords[cIndex] = std::numeric_limits<double>::max();
	}

	m_startTime = std::numeric_limits<double>::max();
	m_endTime = -std::numeric_limits<double>::max();
}

void TimePoint::makeDimension(uint32_t dimension)
{
	if (m_dimension != dimension)
	{
		m_dimension = dimension;

		delete[] m_pCoords;
		m_pCoords = nullptr;

		m_pCoords = new double[m_dimension];
	}
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const TimePoint& pt)
{
	uint32_t i;

	for (i = 0; i < pt.m_dimension; ++i)
	{
		os << pt.m_pCoords[i] << " ";
	}

	os << ", Start: " << pt.m_startTime << ", End: " << pt.m_endTime;

	return os;
}
