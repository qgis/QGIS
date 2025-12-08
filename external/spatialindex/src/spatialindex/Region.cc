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

#include <spatialindex/SpatialIndex.h>

#include <cstring>
#include <cmath>
#include <limits>

using namespace SpatialIndex;

Region::Region()
= default;

Region::Region(const double* pLow, const double* pHigh, uint32_t dimension)
{
	initialize(pLow, pHigh, dimension);
}

Region::Region(const Point& low, const Point& high)
{
	if (low.m_dimension != high.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::Region: arguments have different number of dimensions."
		);

	initialize(low.m_pCoords, high.m_pCoords, low.m_dimension);
}

Region::Region(const Region& r)
{
	initialize(r.m_pLow, r.m_pHigh, r.m_dimension);
}

void Region::initialize(const double* pLow, const double* pHigh, uint32_t dimension)
{
	m_pLow = nullptr;
	m_dimension = dimension;

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

	memcpy(m_pLow, pLow, m_dimension * sizeof(double));
	memcpy(m_pHigh, pHigh, m_dimension * sizeof(double));
}

Region::~Region()
{
	delete[] m_pLow;
	delete[] m_pHigh;
}

Region& Region::operator=(const Region& r)
{
	if(this != &r)
	{
		makeDimension(r.m_dimension);
		memcpy(m_pLow, r.m_pLow, m_dimension * sizeof(double));
		memcpy(m_pHigh, r.m_pHigh, m_dimension * sizeof(double));
	}

	return *this;
}

bool Region::operator==(const Region& r) const
{
	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::operator==: Regions have different number of dimensions."
		);

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

//
// IObject interface
//
Region* Region::clone()
{
	return new Region(*this);
}

//
// ISerializable interface
//
uint32_t Region::getByteArraySize()
{
	return (sizeof(uint32_t) + 2 * m_dimension * sizeof(double));
}

void Region::loadFromByteArray(const uint8_t* ptr)
{
	uint32_t dimension;
	memcpy(&dimension, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	makeDimension(dimension);
	memcpy(m_pLow, ptr, m_dimension * sizeof(double));
	ptr += m_dimension * sizeof(double);
	memcpy(m_pHigh, ptr, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

void Region::storeToByteArray(uint8_t** data, uint32_t& len)
{
	len = getByteArraySize();
	*data = new uint8_t[len];
	uint8_t* ptr = *data;

	memcpy(ptr, &m_dimension, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, m_pLow, m_dimension * sizeof(double));
	ptr += m_dimension * sizeof(double);
	memcpy(ptr, m_pHigh, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

//
// IShape interface
//
bool Region::intersectsShape(const IShape& s) const
{
	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != nullptr) return intersectsRegion(*pr);

	const LineSegment* pls = dynamic_cast<const LineSegment*>(&s);
	if (pls != nullptr) return intersectsLineSegment(*pls);

	const Point* ppt = dynamic_cast<const Point*>(&s);
	if (ppt != nullptr) return containsPoint(*ppt);

	throw Tools::IllegalStateException(
		"Region::intersectsShape: Not implemented yet!"
	);
}

bool Region::containsShape(const IShape& s) const
{
	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != nullptr) return containsRegion(*pr);

	const Point* ppt = dynamic_cast<const Point*>(&s);
	if (ppt != nullptr) return containsPoint(*ppt);

	throw Tools::IllegalStateException(
		"Region::containsShape: Not implemented yet!"
	);
}

bool Region::touchesShape(const IShape& s) const
{
	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != nullptr) return touchesRegion(*pr);

	const Point* ppt = dynamic_cast<const Point*>(&s);
	if (ppt != nullptr) return touchesPoint(*ppt);

	throw Tools::IllegalStateException(
		"Region::touchesShape: Not implemented yet!"
	);
}

void Region::getCenter(Point& out) const
{
	out.makeDimension(m_dimension);
	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		out.m_pCoords[i] = (m_pLow[i] + m_pHigh[i]) / 2.0;
	}
}

uint32_t Region::getDimension() const
{
	return m_dimension;
}

void Region::getMBR(Region& out) const
{
	out = *this;
}

double Region::getArea() const
{
	double area = 1.0;

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		area *= m_pHigh[i] - m_pLow[i];
	}

	return area;
}

double Region::getMinimumDistance(const IShape& s) const
{
	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != nullptr) return getMinimumDistance(*pr);

	const Point* ppt = dynamic_cast<const Point*>(&s);
	if (ppt != nullptr) return getMinimumDistance(*ppt);

	throw Tools::IllegalStateException(
		"Region::getMinimumDistance: Not implemented yet!"
	);
}

bool Region::intersectsRegion(const Region& r) const
{
	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::intersectsRegion: Regions have different number of dimensions."
		);

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (m_pLow[i] > r.m_pHigh[i] || m_pHigh[i] < r.m_pLow[i]) return false;
	}
	return true;
}

bool Region::containsRegion(const Region& r) const
{
	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::containsRegion: Regions have different number of dimensions."
		);

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (m_pLow[i] > r.m_pLow[i] || m_pHigh[i] < r.m_pHigh[i]) return false;
	}
	return true;
}

bool Region::touchesRegion(const Region& r) const
{
	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::touchesRegion: Regions have different number of dimensions."
		);

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (
			(m_pLow[i] >= r.m_pLow[i] - std::numeric_limits<double>::epsilon() &&
			m_pLow[i] <= r.m_pLow[i] + std::numeric_limits<double>::epsilon()) ||
			(m_pHigh[i] >= r.m_pHigh[i] - std::numeric_limits<double>::epsilon() &&
			m_pHigh[i] <= r.m_pHigh[i] + std::numeric_limits<double>::epsilon()))
			return true;
	}
	return false;
}


double Region::getMinimumDistance(const Region& r) const
{
	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::getMinimumDistance: Regions have different number of dimensions."
		);

	double ret = 0.0;

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		double x = 0.0;

		if (r.m_pHigh[i] < m_pLow[i])
		{
			x = std::abs(r.m_pHigh[i] - m_pLow[i]);
		}
		else if (m_pHigh[i] < r.m_pLow[i])
		{
			x = std::abs(r.m_pLow[i] - m_pHigh[i]);
		}

		ret += x * x;
	}

	return std::sqrt(ret);
}

bool Region::intersectsLineSegment(const LineSegment& in) const
{
	if (m_dimension != 2)
		throw Tools::NotSupportedException(
			"Region::intersectsLineSegment: only supported for 2 dimensions"
		);

	if (m_dimension != in.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::intersectsRegion: Region and LineSegment have different number of dimensions."
		);

    // there may be a more efficient method, but this suffices for now
    Point ll = Point(m_pLow, 2);
    Point ur = Point(m_pHigh, 2);
    // fabricate ul and lr coordinates and points
    double c_ul[2] = {m_pLow[0], m_pHigh[1]};
    double c_lr[2] = {m_pHigh[0], m_pLow[1]};
    Point ul = Point(&c_ul[0], 2);
    Point lr = Point(&c_lr[0], 2);

    // Points/LineSegment for the segment
    Point p1 = Point(in.m_pStartPoint, 2);
    Point p2 = Point(in.m_pEndPoint, 2);


    //Check whether either or both the endpoints are within the region OR
    //whether any of the bounding segments of the Region intersect the segment
    return (containsPoint(p1) || containsPoint(p2) ||
            in.intersectsShape(LineSegment(ll, ul)) || in.intersectsShape(LineSegment(ul, ur)) ||
            in.intersectsShape(LineSegment(ur, lr)) || in.intersectsShape(LineSegment(lr, ll)));

}

bool Region::containsPoint(const Point& p) const
{
	if (m_dimension != p.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::containsPoint: Point has different number of dimensions."
		);

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (m_pLow[i] > p.getCoordinate(i) || m_pHigh[i] < p.getCoordinate(i)) return false;
	}
	return true;
}

bool Region::touchesPoint(const Point& p) const
{
	if (m_dimension != p.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::touchesPoint: Point has different number of dimensions."
		);

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (
			(m_pLow[i] >= p.getCoordinate(i) - std::numeric_limits<double>::epsilon() &&
			 m_pLow[i] <= p.getCoordinate(i) + std::numeric_limits<double>::epsilon()) ||
			(m_pHigh[i] >= p.getCoordinate(i) - std::numeric_limits<double>::epsilon() &&
			 m_pHigh[i] <= p.getCoordinate(i) + std::numeric_limits<double>::epsilon()))
			return true;
	}
	return false;
}

double Region::getMinimumDistance(const Point& p) const
{
	if (m_dimension != p.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::getMinimumDistance: Point has different number of dimensions."
		);

	double ret = 0.0;

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (p.getCoordinate(i) < m_pLow[i])
		{
			ret += std::pow(m_pLow[i] - p.getCoordinate(i), 2.0);
		}
		else if (p.getCoordinate(i) > m_pHigh[i])
		{
			ret += std::pow(p.getCoordinate(i) - m_pHigh[i], 2.0);
		}
	}

	return std::sqrt(ret);
}

Region Region::getIntersectingRegion(const Region& r) const
{
	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::getIntersectingRegion: Regions have different number of dimensions."
		);

	Region ret;
	ret.makeInfinite(m_dimension);

	// check for intersection.
	// marioh: avoid function call since this is called billions of times.
	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		if (m_pLow[cDim] > r.m_pHigh[cDim] || m_pHigh[cDim] < r.m_pLow[cDim]) return ret;
	}

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		ret.m_pLow[cDim] = std::max(m_pLow[cDim], r.m_pLow[cDim]);
		ret.m_pHigh[cDim] = std::min(m_pHigh[cDim], r.m_pHigh[cDim]);
	}

	return ret;
}

double Region::getIntersectingArea(const Region& r) const
{
	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::getIntersectingArea: Regions have different number of dimensions."
		);

	double ret = 1.0;
	double f1, f2;

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		if (m_pLow[cDim] > r.m_pHigh[cDim] || m_pHigh[cDim] < r.m_pLow[cDim]) return 0.0;

		f1 = std::max(m_pLow[cDim], r.m_pLow[cDim]);
		f2 = std::min(m_pHigh[cDim], r.m_pHigh[cDim]);
		ret *= f2 - f1;
	}

	return ret;
}

/*
 * Returns the margin of a region. It is calcuated as the sum of  2^(d-1) * width, in each dimension.
 * It is actually the sum of all edges, no matter what the dimensionality is.
*/
double Region::getMargin() const
{
	double mul = std::pow(2.0, static_cast<double>(m_dimension) - 1.0);
	double margin = 0.0;

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		margin += (m_pHigh[i] - m_pLow[i]) * mul;
	}

	return margin;
}

void Region::combineRegion(const Region& r)
{
	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::combineRegion: Region has different number of dimensions."
		);

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		m_pLow[cDim] = std::min(m_pLow[cDim], r.m_pLow[cDim]);
		m_pHigh[cDim] = std::max(m_pHigh[cDim], r.m_pHigh[cDim]);
	}
}

void Region::combinePoint(const Point& p)
{
	if (m_dimension != p.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::combinePoint: Point has different number of dimensions."
		);

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		m_pLow[cDim] = std::min(m_pLow[cDim], p.m_pCoords[cDim]);
		m_pHigh[cDim] = std::max(m_pHigh[cDim], p.m_pCoords[cDim]);
	}
}

void Region::getCombinedRegion(Region& out, const Region& in) const
{
	if (m_dimension != in.m_dimension)
		throw Tools::IllegalArgumentException(
			"Region::getCombinedRegion: Regions have different number of dimensions."
		);

	out = *this;
	out.combineRegion(in);
}

double Region::getLow(uint32_t index) const
{
	if (index >= m_dimension)
		throw Tools::IndexOutOfBoundsException(index);

	return m_pLow[index];
}

double Region::getHigh(uint32_t index) const
{
	if (index >= m_dimension)
		throw Tools::IndexOutOfBoundsException(index);

	return m_pHigh[index];
}

void Region::makeInfinite(uint32_t dimension)
{
	makeDimension(dimension);
	for (uint32_t cIndex = 0; cIndex < m_dimension; ++cIndex)
	{
		m_pLow[cIndex] = std::numeric_limits<double>::max();
		m_pHigh[cIndex] = -std::numeric_limits<double>::max();
	}
}

void Region::makeDimension(uint32_t dimension)
{
	if (m_dimension != dimension)
	{
		delete[] m_pLow;
		delete[] m_pHigh;

		// remember that this is not a constructor. The object will be destructed normally if
		// something goes wrong (bad_alloc), so we must take care not to leave the object at an intermediate state.
		m_pLow = nullptr; m_pHigh = nullptr;

		m_dimension = dimension;
		m_pLow = new double[m_dimension];
		m_pHigh = new double[m_dimension];
	}
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const Region& r)
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

	return os;
}
