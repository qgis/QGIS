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
#include <cmath>
#include <limits>

#include <spatialindex/SpatialIndex.h>

using namespace SpatialIndex;

MovingPoint::MovingPoint()
= default;

MovingPoint::MovingPoint(	const double* pCoords,
							const double* pVCoords,
							const IInterval& ti,
							uint32_t dimension)
{
	initialize(	pCoords,
				pVCoords,
				ti.getLowerBound(),
				ti.getUpperBound(),
				dimension);
}

MovingPoint::MovingPoint(	const double* pCoords,
							const double* pVCoords,
							double tStart,
							double tEnd,
							uint32_t dimension)
{
	initialize(pCoords, pVCoords, tStart, tEnd, dimension);
}

MovingPoint::MovingPoint(	const Point& p,
							const Point& vp,
							const IInterval& ti)
{
	if (p.m_dimension != vp.m_dimension)
		throw Tools::IllegalArgumentException("MovingPoint: Points have different number of dimensions.");

	initialize(	p.m_pCoords,
				vp.m_pCoords,
				ti.getLowerBound(),
				ti.getUpperBound(),
				p.m_dimension);
}

MovingPoint::MovingPoint(const Point& p, const Point& vp, double tStart, double tEnd)
{
	if (p.m_dimension != vp.m_dimension)
		throw Tools::IllegalArgumentException("MovingPoint: Points have different number of dimensions.");

	initialize(	p.m_pCoords,
				vp.m_pCoords,
				tStart,
				tEnd,
				p.m_dimension);
}

MovingPoint::MovingPoint(const MovingPoint& p)
{
	m_startTime = p.m_startTime;
	m_endTime = p.m_endTime;
	m_pCoords = nullptr;

	m_dimension = p.m_dimension;

	try
	{
		m_pCoords = new double[m_dimension];
		m_pVCoords = new double[m_dimension];
	}
	catch (...)
	{
		delete[] m_pCoords;
		throw;
	}

	memcpy(m_pCoords, p.m_pCoords, m_dimension * sizeof(double));
	memcpy(m_pVCoords, p.m_pVCoords, m_dimension * sizeof(double));
}

MovingPoint::~MovingPoint()
{
	delete[] m_pVCoords;
}

void MovingPoint::initialize(
	const double* pCoords, const double* pVCoords,
	double tStart, double tEnd, uint32_t dimension)
{
	m_dimension = dimension;
	m_startTime = tStart;
	m_endTime = tEnd;
	m_pCoords = nullptr;

	if (m_endTime <= m_startTime)
		throw Tools::IllegalArgumentException("MovingPoint: Cannot support degenerate time intervals.");

	try
	{
		m_pCoords = new double[m_dimension];
		m_pVCoords = new double[m_dimension];
	}
	catch (...)
	{
		delete[] m_pCoords;
		throw;
	}

	// first store the point coordinates, than the point velocities.
	memcpy(m_pCoords, pCoords, m_dimension * sizeof(double));
	memcpy(m_pVCoords, pVCoords, m_dimension * sizeof(double));
}

MovingPoint& MovingPoint::operator=(const MovingPoint& p)
{
	if (this != &p)
	{
		makeDimension(p.m_dimension);
		memcpy(m_pCoords, p.m_pCoords, m_dimension * sizeof(double));
		memcpy(m_pVCoords, p.m_pVCoords, m_dimension * sizeof(double));

		m_startTime = p.m_startTime;
		m_endTime = p.m_endTime;
	}

	return *this;
}

bool MovingPoint::operator==(const MovingPoint& p) const
{
	if (
		m_startTime < p.m_startTime - std::numeric_limits<double>::epsilon() ||
		m_startTime > p.m_startTime + std::numeric_limits<double>::epsilon() ||
		m_endTime < p.m_endTime - std::numeric_limits<double>::epsilon() ||
		m_endTime > p.m_endTime + std::numeric_limits<double>::epsilon())
		return false;

	for (uint32_t cDim = 0; cDim < 2 * m_dimension; ++cDim)
	{
		if (
			m_pCoords[cDim] < p.m_pCoords[cDim] - std::numeric_limits<double>::epsilon() ||
			m_pCoords[cDim] > p.m_pCoords[cDim] + std::numeric_limits<double>::epsilon() ||
			m_pVCoords[cDim] < p.m_pVCoords[cDim] - std::numeric_limits<double>::epsilon() ||
			m_pVCoords[cDim] > p.m_pVCoords[cDim] + std::numeric_limits<double>::epsilon())
			return false;
	}

	return true;
}


double MovingPoint::getCoord(uint32_t d, double t) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	if (t >= m_endTime) return m_pCoords[d] + m_pVCoords[d] * (m_endTime - m_startTime);
	else if (t <= m_startTime) return m_pCoords[d];
	else return m_pCoords[d] + m_pVCoords[d] * (t - m_startTime);
}

double MovingPoint::getProjectedCoord(uint32_t d, double t) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	return m_pCoords[d] + m_pVCoords[d] * (t - m_startTime);
}

double MovingPoint::getVCoord(uint32_t d) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	return m_pVCoords[d];
}

void MovingPoint::getPointAtTime(double t, Point& out) const
{
	out.makeDimension(m_dimension);
	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		out.m_pCoords[cDim] = getCoord(cDim, t);
	}
}

//
// IObject interface
//
MovingPoint* MovingPoint::clone()
{
	return new MovingPoint(*this);
}

//
// ISerializable interface
//
uint32_t MovingPoint::getByteArraySize()
{
	return (sizeof(uint32_t) + 2 * sizeof(double) + 2 * m_dimension * sizeof(double));
}

void MovingPoint::loadFromByteArray(const uint8_t* ptr)
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
	ptr += m_dimension * sizeof(double);
	memcpy(m_pVCoords, ptr, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

void MovingPoint::storeToByteArray(uint8_t** data, uint32_t& len)
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
	ptr += m_dimension * sizeof(double);
	memcpy(ptr, m_pVCoords, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

//
// IEvolvingShape interface
//
void MovingPoint::getVMBR(Region& out) const
{
	out.makeDimension(m_dimension);
	memcpy(out.m_pLow, m_pVCoords, m_dimension * sizeof(double));
	memcpy(out.m_pHigh, m_pVCoords, m_dimension * sizeof(double));
}

void MovingPoint::getMBRAtTime(double t, Region& out) const
{
	out.makeDimension(m_dimension);
	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		out.m_pLow[cDim] = getCoord(cDim, t);
		out.m_pHigh[cDim] = getCoord(cDim, t);
	}
}

void MovingPoint::makeInfinite(uint32_t dimension)
{
	makeDimension(dimension);
	for (uint32_t cIndex = 0; cIndex < m_dimension; ++cIndex)
	{
		m_pCoords[cIndex] = std::numeric_limits<double>::max();
		m_pVCoords[cIndex] = -std::numeric_limits<double>::max();
	}

	m_startTime = std::numeric_limits<double>::max();
	m_endTime = -std::numeric_limits<double>::max();
}

void MovingPoint::makeDimension(uint32_t dimension)
{
	if (m_dimension != dimension)
	{
		delete[] m_pCoords;
		delete[] m_pVCoords;
		m_pCoords = nullptr; m_pVCoords = nullptr;

		m_dimension = dimension;
		m_pCoords = new double[m_dimension];
		m_pVCoords = new double[m_dimension];
	}
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const MovingPoint& pt)
{
	uint32_t i;

	os << "Coords: ";
	for (i = 0; i < pt.m_dimension; ++i)
	{
		os << pt.m_pCoords[i] << " ";
	}

	os << "VCoords: ";
	for (i = 0; i < pt.m_dimension; ++i)
	{
		os << pt.m_pVCoords[i] << " ";
	}

	os << ", Start: " << pt.m_startTime << ", End: " << pt.m_endTime;

	return os;
}
