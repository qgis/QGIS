/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2004, Marios Hadjieleftheriou
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

Point::Point()
= default;

Point::Point(const double* pCoords, uint32_t dimension)
	: m_dimension(dimension)
{
	// no need to initialize m_pCoords to 0 since if a bad_alloc is raised the destructor will not be called.

	m_pCoords = new double[m_dimension];
	memcpy(m_pCoords, pCoords, m_dimension * sizeof(double));
}

Point::Point(const Point& p)
	: m_dimension(p.m_dimension)
{
	// no need to initialize m_pCoords to 0 since if a bad_alloc is raised the destructor will not be called.

	m_pCoords = new double[m_dimension];
	memcpy(m_pCoords, p.m_pCoords, m_dimension * sizeof(double));
}

Point::~Point()
{
	delete[] m_pCoords;
}

Point& Point::operator=(const Point& p)
{
	if (this != &p)
	{
		makeDimension(p.m_dimension);
		memcpy(m_pCoords, p.m_pCoords, m_dimension * sizeof(double));
	}

	return *this;
}

bool Point::operator==(const Point& p) const
{
	if (m_dimension != p.m_dimension)
		throw Tools::IllegalArgumentException(
			"Point::operator==: Points have different number of dimensions."
		);

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (
			m_pCoords[i] < p.m_pCoords[i] - std::numeric_limits<double>::epsilon() ||
			m_pCoords[i] > p.m_pCoords[i] + std::numeric_limits<double>::epsilon())  return false;
	}

	return true;
}

//
// IObject interface
//
Point* Point::clone()
{
	return new Point(*this);
}

//
// ISerializable interface
//
uint32_t Point::getByteArraySize()
{
	return (sizeof(uint32_t) + m_dimension * sizeof(double));
}

void Point::loadFromByteArray(const uint8_t* ptr)
{
	uint32_t dimension;
	memcpy(&dimension, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	makeDimension(dimension);
	memcpy(m_pCoords, ptr, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

void Point::storeToByteArray(uint8_t** data, uint32_t& len)
{
	len = getByteArraySize();
	*data = new uint8_t[len];
	uint8_t* ptr = *data;

	memcpy(ptr, &m_dimension, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, m_pCoords, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

//
// IShape interface
//
bool Point::intersectsShape(const IShape& s) const
{
	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != nullptr)
	{
		return pr->containsPoint(*this);
	}

	throw Tools::IllegalStateException(
		"Point::intersectsShape: Not implemented yet!"
	);
}

bool Point::containsShape(const IShape&) const
{
	return false;
}

bool Point::touchesShape(const IShape& s) const
{
	const Point* ppt = dynamic_cast<const Point*>(&s);
	if (ppt != nullptr)
	{
		if (*this == *ppt) return true;
		return false;
	}

	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != nullptr)
	{
		return pr->touchesPoint(*this);
	}

	throw Tools::IllegalStateException(
		"Point::touchesShape: Not implemented yet!"
	);
}

void Point::getCenter(Point& out) const
{
	out = *this;
}

uint32_t Point::getDimension() const
{
	return m_dimension;
}

void Point::getMBR(Region& out) const
{
	out = Region(m_pCoords, m_pCoords, m_dimension);
}

double Point::getArea() const
{
	return 0.0;
}

double Point::getMinimumDistance(const IShape& s) const
{
	const Point* ppt = dynamic_cast<const Point*>(&s);
	if (ppt != nullptr)
	{
		return getMinimumDistance(*ppt);
	}

	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != nullptr)
	{
		return pr->getMinimumDistance(*this);
	}

	throw Tools::IllegalStateException(
		"Point::getMinimumDistance: Not implemented yet!"
	);
}

double Point::getMinimumDistance(const Point& p) const
{
	if (m_dimension != p.m_dimension)
		throw Tools::IllegalArgumentException(
			"Point::getMinimumDistance: Shapes have different number of dimensions."
		);

	double ret = 0.0;

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		ret += std::pow(m_pCoords[cDim] - p.m_pCoords[cDim], 2.0);
	}

	return std::sqrt(ret);
}

double Point::getCoordinate(uint32_t index) const
{
	if (index >= m_dimension)
		throw Tools::IndexOutOfBoundsException(index);

	return m_pCoords[index];
}

void Point::makeInfinite(uint32_t dimension)
{
	makeDimension(dimension);
	for (uint32_t cIndex = 0; cIndex < m_dimension; ++cIndex)
	{
		m_pCoords[cIndex] = std::numeric_limits<double>::max();
	}
}

void Point::makeDimension(uint32_t dimension)
{
	if (m_dimension != dimension)
	{
		delete[] m_pCoords;

		// remember that this is not a constructor. The object will be destructed normally if
		// something goes wrong (bad_alloc), so we must take care not to leave the object at an intermediate state.
		m_pCoords = nullptr;

		m_dimension = dimension;
		m_pCoords = new double[m_dimension];
	}
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const Point& pt)
{
	for (uint32_t cDim = 0; cDim < pt.m_dimension; ++cDim)
	{
		os << pt.m_pCoords[cDim] << " ";
	}

	return os;
}
