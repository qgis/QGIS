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

LineSegment::LineSegment()
= default;

LineSegment::LineSegment(const double* pStartPoint, const double* pEndPoint, uint32_t dimension)
	: m_dimension(dimension)
{
	// no need to initialize arrays to 0 since if a bad_alloc is raised the destructor will not be called.

	m_pStartPoint = new double[m_dimension];
	m_pEndPoint = new double[m_dimension];
	memcpy(m_pStartPoint, pStartPoint, m_dimension * sizeof(double));
	memcpy(m_pEndPoint, pEndPoint, m_dimension * sizeof(double));
}

LineSegment::LineSegment(const Point& startPoint, const Point& endPoint)
	: m_dimension(startPoint.m_dimension)
{
	if (startPoint.m_dimension != endPoint.m_dimension)
		throw Tools::IllegalArgumentException(
			"LineSegment::LineSegment: Points have different dimensionalities."
		);

	// no need to initialize arrays to 0 since if a bad_alloc is raised the destructor will not be called.

	m_pStartPoint = new double[m_dimension];
	m_pEndPoint = new double[m_dimension];
	memcpy(m_pStartPoint, startPoint.m_pCoords, m_dimension * sizeof(double));
	memcpy(m_pEndPoint, endPoint.m_pCoords, m_dimension * sizeof(double));
}

LineSegment::LineSegment(const LineSegment& l)
	: m_dimension(l.m_dimension)
{
	// no need to initialize arrays to 0 since if a bad_alloc is raised the destructor will not be called.

	m_pStartPoint = new double[m_dimension];
	m_pEndPoint = new double[m_dimension];
	memcpy(m_pStartPoint, l.m_pStartPoint, m_dimension * sizeof(double));
	memcpy(m_pEndPoint, l.m_pEndPoint, m_dimension * sizeof(double));
}

LineSegment::~LineSegment()
{
	delete[] m_pStartPoint;
	delete[] m_pEndPoint;
}

LineSegment& LineSegment::operator=(const LineSegment& l)
{
	if (this != &l)
	{
		makeDimension(l.m_dimension);
		memcpy(m_pStartPoint, l.m_pStartPoint, m_dimension * sizeof(double));
		memcpy(m_pEndPoint, l.m_pEndPoint, m_dimension * sizeof(double));
	}

	return *this;
}

bool LineSegment::operator==(const LineSegment& l) const
{
	if (m_dimension != l.m_dimension)
		throw Tools::IllegalArgumentException(
			"LineSegment::operator==: LineSegments have different number of dimensions."
		);

	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (
			m_pStartPoint[i] < l.m_pStartPoint[i] - std::numeric_limits<double>::epsilon() ||
			m_pStartPoint[i] > l.m_pStartPoint[i] + std::numeric_limits<double>::epsilon())  return false;

		if (
			m_pEndPoint[i] < l.m_pEndPoint[i] - std::numeric_limits<double>::epsilon() ||
			m_pEndPoint[i] > l.m_pEndPoint[i] + std::numeric_limits<double>::epsilon())  return false;
	}

	return true;
}

//
// IObject interface
//
LineSegment* LineSegment::clone()
{
	return new LineSegment(*this);
}

//
// ISerializable interface
//
uint32_t LineSegment::getByteArraySize()
{
	return (sizeof(uint32_t) + m_dimension * sizeof(double) * 2);
}

void LineSegment::loadFromByteArray(const uint8_t* ptr)
{
	uint32_t dimension;
	memcpy(&dimension, ptr, sizeof(uint32_t));
	ptr += sizeof(uint32_t);

	makeDimension(dimension);
	memcpy(m_pStartPoint, ptr, m_dimension * sizeof(double));
	ptr += m_dimension * sizeof(double);
	memcpy(m_pEndPoint, ptr, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

void LineSegment::storeToByteArray(uint8_t** data, uint32_t& len)
{
	len = getByteArraySize();
	*data = new uint8_t[len];
	uint8_t* ptr = *data;

	memcpy(ptr, &m_dimension, sizeof(uint32_t));
	ptr += sizeof(uint32_t);
	memcpy(ptr, m_pStartPoint, m_dimension * sizeof(double));
	ptr += m_dimension * sizeof(double);
	memcpy(ptr, m_pEndPoint, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

//
// IShape interface
//
bool LineSegment::intersectsShape(const IShape& s) const
{
	const LineSegment* ps = dynamic_cast<const LineSegment*>(&s);
	if (ps != nullptr) return intersectsLineSegment(*ps);

	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != nullptr) return intersectsRegion(*pr);

	throw Tools::IllegalStateException(
		"LineSegment::intersectsShape: Not implemented yet!"
	);
}

bool LineSegment::containsShape(const IShape&) const
{
	return false;
}

bool LineSegment::touchesShape(const IShape&) const
{
	throw Tools::IllegalStateException(
		"LineSegment::touchesShape: Not implemented yet!"
	);
}

void LineSegment::getCenter(Point& out) const
{
	double* coords = new double[m_dimension];
	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		coords[cDim] =
			(std::abs(m_pStartPoint[cDim] - m_pEndPoint[cDim]) / 2.0) +
			std::min(m_pStartPoint[cDim], m_pEndPoint[cDim]);
	}

	out = Point(coords, m_dimension);
	delete[] coords;
}

uint32_t LineSegment::getDimension() const
{
	return m_dimension;
}

void LineSegment::getMBR(Region& out) const
{
	double* low = new double[m_dimension];
	double* high = new double[m_dimension];
	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		low[cDim] = std::min(m_pStartPoint[cDim], m_pEndPoint[cDim]);
		high[cDim] = std::max(m_pStartPoint[cDim], m_pEndPoint[cDim]);
	}

	out = Region(low, high, m_dimension);
	delete[] low;
	delete[] high;
}

double LineSegment::getArea() const
{
	return 0.0;
}

double LineSegment::getMinimumDistance(const IShape& s) const
{
	const Point* ppt = dynamic_cast<const Point*>(&s);
	if (ppt != nullptr)
	{
		return getMinimumDistance(*ppt);
	}

/*
	const Region* pr = dynamic_cast<const Region*>(&s);
	if (pr != 0)
	{
		return pr->getMinimumDistance(*this);
	}
*/

	throw Tools::IllegalStateException(
		"LineSegment::getMinimumDistance: Not implemented yet!"
	);
}

double LineSegment::getMinimumDistance(const Point& p) const
{
	if (m_dimension == 1)
		throw Tools::NotSupportedException(
			"LineSegment::getMinimumDistance: Use an Interval instead."
		);

	if (m_dimension != 2)
		throw Tools::NotSupportedException(
			"LineSegment::getMinimumDistance: Distance for high dimensional spaces not supported!"
		);

	if (m_pEndPoint[0] >= m_pStartPoint[0] - std::numeric_limits<double>::epsilon() &&
		m_pEndPoint[0] <= m_pStartPoint[0] + std::numeric_limits<double>::epsilon()) return std::abs(p.m_pCoords[0] - m_pStartPoint[0]);

	if (m_pEndPoint[1] >= m_pStartPoint[1] - std::numeric_limits<double>::epsilon() &&
		m_pEndPoint[1] <= m_pStartPoint[1] + std::numeric_limits<double>::epsilon()) return std::abs(p.m_pCoords[1] - m_pStartPoint[1]);

	double x1 = m_pStartPoint[0];
	double x2 = m_pEndPoint[0];
	double x0 = p.m_pCoords[0];
	double y1 = m_pStartPoint[1];
	double y2 = m_pEndPoint[1];
	double y0 = p.m_pCoords[1];

	return std::abs((x2 - x1) * (y1 - y0) - (x1 - x0) * (y2 - y1)) / (std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)));
}

bool LineSegment::intersectsRegion(const Region& r) const
{
	if (m_dimension != 2)
		throw Tools::NotSupportedException(
			"LineSegment::intersectsRegion: only supported for 2 dimensions"
		);

	if (m_dimension != r.m_dimension)
		throw Tools::IllegalArgumentException(
			"LineSegment::intersectsRegion: LineSegment and Region have different number of dimensions."
		);

    return r.intersectsLineSegment((*this));
}

bool LineSegment::intersectsLineSegment(const LineSegment& l) const
{
	if (m_dimension != 2)
		throw Tools::NotSupportedException(
			"LineSegment::intersectsLineSegment: only supported for 2 dimensions"
		);

	if (m_dimension != l.m_dimension)
		throw Tools::IllegalArgumentException(
			"LineSegment::intersectsLineSegment: LineSegments have different number of dimensions."
		);

    // use Geometry::intersects
    Point p1, p2, p3, p4;
    p1 = Point(m_pStartPoint, 2);
    p2 = Point(m_pEndPoint, 2);
    p3 = Point(l.m_pStartPoint, 2);
    p4 = Point(l.m_pEndPoint, 2);
    return intersects(p1, p2, p3, p4);
}

// assuming moving from start to end, positive distance is from right hand side.
double LineSegment::getRelativeMinimumDistance(const Point& p) const
{
	if (m_dimension == 1)
		throw Tools::NotSupportedException(
			"LineSegment::getRelativeMinimumDistance: Use an Interval instead."
		);

	if (m_dimension != 2)
		throw Tools::NotSupportedException(
			"LineSegment::getRelativeMinimumDistance: Distance for high dimensional spaces not supported!"
		);

	if (m_pEndPoint[0] >= m_pStartPoint[0] - std::numeric_limits<double>::epsilon() &&
		m_pEndPoint[0] <= m_pStartPoint[0] + std::numeric_limits<double>::epsilon())
	{
		if (m_pStartPoint[1] < m_pEndPoint[1]) return m_pStartPoint[0] - p.m_pCoords[0];
		if (m_pStartPoint[1] >= m_pEndPoint[1]) return p.m_pCoords[0] - m_pStartPoint[0];
	}

	if (m_pEndPoint[1] >= m_pStartPoint[1] - std::numeric_limits<double>::epsilon() &&
		m_pEndPoint[1] <= m_pStartPoint[1] + std::numeric_limits<double>::epsilon())
	{
		if (m_pStartPoint[0] < m_pEndPoint[0]) return p.m_pCoords[1] - m_pStartPoint[1];
		if (m_pStartPoint[0] >= m_pEndPoint[0]) return m_pStartPoint[1] - p.m_pCoords[1];
	}

	double x1 = m_pStartPoint[0];
	double x2 = m_pEndPoint[0];
	double x0 = p.m_pCoords[0];
	double y1 = m_pStartPoint[1];
	double y2 = m_pEndPoint[1];
	double y0 = p.m_pCoords[1];

	return ((x1 - x0) * (y2 - y1) - (x2 - x1) * (y1 - y0)) / (std::sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)));
}

double LineSegment::getRelativeMaximumDistance(const Region& r) const
{
	if (m_dimension == 1)
		throw Tools::NotSupportedException(
			"LineSegment::getRelativeMaximumDistance: Use an Interval instead."
		);

	if (m_dimension != 2)
		throw Tools::NotSupportedException(
			"LineSegment::getRelativeMaximumDistance: Distance for high dimensional spaces not supported!"
		);

	// clockwise.
	double d1 = getRelativeMinimumDistance(Point(r.m_pLow, 2));

	double coords[2];
	coords[0] = r.m_pLow[0];
	coords[1] = r.m_pHigh[1];
	double d2 = getRelativeMinimumDistance(Point(coords, 2));

	double d3 = getRelativeMinimumDistance(Point(r.m_pHigh, 2));

	coords[0] = r.m_pHigh[0];
	coords[1] = r.m_pLow[1];
	double d4 = getRelativeMinimumDistance(Point(coords, 2));

	return std::max(d1, std::max(d2, std::max(d3, d4)));
}

double LineSegment::getAngleOfPerpendicularRay()
{
	if (m_dimension == 1)
		throw Tools::NotSupportedException(
			"LineSegment::getAngleOfPerpendicularRay: Use an Interval instead."
		);

	if (m_dimension != 2)
		throw Tools::NotSupportedException(
			"LineSegment::getAngleOfPerpendicularRay: Distance for high dimensional spaces not supported!"
		);

	if (m_pStartPoint[0] >= m_pEndPoint[0] - std::numeric_limits<double>::epsilon() &&
		m_pStartPoint[0] <= m_pEndPoint[0] + std::numeric_limits<double>::epsilon()) return 0.0;

	if (m_pStartPoint[1] >= m_pEndPoint[1] - std::numeric_limits<double>::epsilon() &&
		m_pStartPoint[1] <= m_pEndPoint[1] + std::numeric_limits<double>::epsilon()) return M_PI_2;

	return std::atan(-(m_pStartPoint[0] - m_pEndPoint[0]) / (m_pStartPoint[1] - m_pEndPoint[1]));
}

void LineSegment::makeInfinite(uint32_t dimension)
{
	makeDimension(dimension);
	for (uint32_t cIndex = 0; cIndex < m_dimension; ++cIndex)
	{
		m_pStartPoint[cIndex] = std::numeric_limits<double>::max();
		m_pEndPoint[cIndex] = std::numeric_limits<double>::max();
	}
}

void LineSegment::makeDimension(uint32_t dimension)
{
	if (m_dimension != dimension)
	{
		delete[] m_pStartPoint;
		delete[] m_pEndPoint;

		// remember that this is not a constructor. The object will be destructed normally if
		// something goes wrong (bad_alloc), so we must take care not to leave the object at an intermediate state.
		m_pStartPoint = nullptr;
		m_pEndPoint = nullptr;

		m_dimension = dimension;
		m_pStartPoint = new double[m_dimension];
		m_pEndPoint = new double[m_dimension];
	}
}

// compute double the area of the triangle created by points a, b and c (only for 2 dimensional points)
double LineSegment::doubleAreaTriangle(const SpatialIndex::Point &a, const SpatialIndex::Point &b, const SpatialIndex::Point &c) {
    double *pA, *pB, *pC;
    pA = a.m_pCoords; pB = b.m_pCoords; pC = c.m_pCoords;
    return (((pB[0] - pA[0]) * (pC[1] - pA[1])) - ((pC[0] - pA[0]) * (pB[1] - pA[1])));
}

// determine whether point c is to the left of the segment comprised of points a & b (2-d only)
bool LineSegment::leftOf(const SpatialIndex::Point &a, const SpatialIndex::Point &b, const SpatialIndex::Point &c) {
    return (doubleAreaTriangle(a, b, c) > 0);
}

// determine whether all 3 points are on the same line
bool LineSegment::collinear(const SpatialIndex::Point &a, const SpatialIndex::Point &b, const SpatialIndex::Point &c) {
    return (doubleAreaTriangle(a, b, c) == 0);
}

// determine whether the segment comprised of a, b and segment of c, d intersect (exclusive of their endpoints..hence the "Proper")
bool LineSegment::intersectsProper(const SpatialIndex::Point &a, const SpatialIndex::Point &b, const SpatialIndex::Point &c, const SpatialIndex::Point &d) {
    if ( collinear(a, b, c) || collinear(a, b, d) ||
         collinear(c, d, a) || collinear(c, d, b)) {
        return false;
    }
    return ((leftOf(a, b, c) ^ leftOf(a, b, d)) &&
            (leftOf(c, d, a) ^ leftOf(c, d, b)));
}

// if the points are collinear, is c between a & b
bool LineSegment::between(const SpatialIndex::Point &a, const SpatialIndex::Point &b, const SpatialIndex::Point &c) {
    if ( !collinear(a, b, c) ) {
        return false;
    }
    double *pA, *pB, *pC;
    pA = a.m_pCoords; pB = b.m_pCoords; pC = c.m_pCoords;
    if ( pA[0] != pB[0] ) { // a & b are not on the same vertical, compare on x axis
        return  between(pA[0], pB[0], pC[0]);
    } else { // a & b are a vertical segment, we need to compare on y axis
        return between(pA[1], pB[1], pC[1]);
    }
}

bool LineSegment::between(double a, double b, double c) {
    return ( ((a <= c) && (c <= b)) || ((a >= c) && (c >= b)) );
}

// intersection test, including endpoints
bool LineSegment::intersects(const SpatialIndex::Point &a, const SpatialIndex::Point &b, const SpatialIndex::Point &c, const SpatialIndex::Point &d) {
    if (intersectsProper(a, b, c, d)) {
        return true;
    } 
    else if ( between(a, b, c) || between(a, b, d) ||
              between(c, d, a) || between(c, d, b) ) { 
        return true;
    }
    else { 
        return false;
    }
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const LineSegment& l)
{
	for (uint32_t cDim = 0; cDim < l.m_dimension; ++cDim)
	{
		os << l.m_pStartPoint[cDim] << ", " << l.m_pEndPoint[cDim] << " ";
	}

	return os;
}
