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

/*
 * Does not support degenerate time intervals or shrinking regions.
*/



#include <cstring>
#include <cmath>
#include <limits>

#include <spatialindex/SpatialIndex.h>

using namespace SpatialIndex;

MovingRegion::MovingRegion()
    : TimeRegion()
{
}

MovingRegion::MovingRegion(
	const double* pLow, const double* pHigh,
	const double* pVLow, const double* pVHigh,
	const IInterval& ivT, uint32_t dimension)
{
	initialize(pLow, pHigh, pVLow, pVHigh, ivT.getLowerBound(), ivT.getUpperBound(), dimension);
}

MovingRegion::MovingRegion(
	const double* pLow, const double* pHigh,
	const double* pVLow, const double* pVHigh,
	double tStart, double tEnd, uint32_t dimension)
{
	initialize(pLow, pHigh, pVLow, pVHigh, tStart, tEnd, dimension);
}

MovingRegion::MovingRegion(
	const Point& low, const Point& high,
	const Point& vlow, const Point& vhigh,
	const IInterval& ivT)
{
	if (low.m_dimension != high.m_dimension || low.m_dimension != vlow.m_dimension || vlow.m_dimension != vhigh.m_dimension)
		throw Tools::IllegalArgumentException("MovingRegion: arguments have different number of dimensions.");

	initialize(
		low.m_pCoords, high.m_pCoords, vlow.m_pCoords, vhigh.m_pCoords,
		ivT.getLowerBound(), ivT.getUpperBound(), low.m_dimension);
}

MovingRegion::MovingRegion(
	const Point& low, const Point& high,
	const Point& vlow, const Point& vhigh,
	double tStart, double tEnd)
{
	if (low.m_dimension != high.m_dimension || low.m_dimension != vlow.m_dimension || vlow.m_dimension != vhigh.m_dimension)
		throw Tools::IllegalArgumentException("MovingRegion: arguments have different number of dimensions.");

	initialize(
		low.m_pCoords, high.m_pCoords, vlow.m_pCoords, vhigh.m_pCoords,
		tStart, tEnd, low.m_dimension);
}

MovingRegion::MovingRegion(
	const Region& mbr, const Region& vbr, const IInterval& ivT)
{
	if (mbr.m_dimension != vbr.m_dimension)
		throw Tools::IllegalArgumentException("MovingRegion: arguments have different number of dimensions.");

	initialize(mbr.m_pLow, mbr.m_pHigh, vbr.m_pLow, vbr.m_pHigh, ivT.getLowerBound(), ivT.getUpperBound(), mbr.m_dimension);
}

MovingRegion::MovingRegion(
	const Region& mbr, const Region& vbr, double tStart, double tEnd)
{
	if (mbr.m_dimension != vbr.m_dimension)
		throw Tools::IllegalArgumentException("MovingRegion: arguments have different number of dimensions.");

	initialize(mbr.m_pLow, mbr.m_pHigh, vbr.m_pLow, vbr.m_pHigh, tStart, tEnd, mbr.m_dimension);
}

MovingRegion::MovingRegion(const MovingPoint& low, const MovingPoint& high)
{
	m_startTime = low.m_startTime;
	m_endTime = high.m_endTime;;
	m_dimension = low.m_dimension;
	m_pLow = nullptr; m_pHigh = nullptr;
	m_pVLow = nullptr; m_pVHigh = nullptr;

	if (m_endTime <= m_startTime) throw Tools::IllegalArgumentException("MovingRegion: Cannot support degenerate time intervals.");

	if (low.m_dimension != high.m_dimension) throw Tools::IllegalArgumentException("MovingRegion: arguments have different number of dimensions.");

	try
	{
		m_pLow = new double[m_dimension];
		m_pHigh = new double[m_dimension];
		m_pVLow = new double[m_dimension];
		m_pVHigh = new double[m_dimension];

	}
	catch (...)
	{
		delete[] m_pLow;
		delete[] m_pHigh;
		delete[] m_pVLow;
		delete[] m_pVHigh;
		throw;
	}

	memcpy(m_pLow, low.m_pCoords, m_dimension * sizeof(double));
	memcpy(m_pHigh, high.m_pCoords, m_dimension * sizeof(double));
	memcpy(m_pVLow, low.m_pVCoords, m_dimension * sizeof(double));
	memcpy(m_pVHigh, high.m_pVCoords, m_dimension * sizeof(double));
}

MovingRegion::MovingRegion(const MovingRegion& r)
{
	m_startTime = r.m_startTime;
	m_endTime = r.m_endTime;
	m_pLow = nullptr; m_pHigh = nullptr;
	m_pVLow = nullptr; m_pVHigh = nullptr;

	m_dimension = r.m_dimension;

	try
	{
		m_pLow = new double[m_dimension];
		m_pHigh = new double[m_dimension];
		m_pVLow = new double[m_dimension];
		m_pVHigh = new double[m_dimension];
	}
	catch (...)
	{
		delete[] m_pLow;
		delete[] m_pHigh;
		delete[] m_pVLow;
		delete[] m_pVHigh;
		throw;
	}

	memcpy(m_pLow, r.m_pLow, m_dimension * sizeof(double));
	memcpy(m_pHigh, r.m_pHigh, m_dimension * sizeof(double));
	memcpy(m_pVLow, r.m_pVLow, m_dimension * sizeof(double));
	memcpy(m_pVHigh, r.m_pVHigh, m_dimension * sizeof(double));
}

void MovingRegion::initialize(
	const double* pLow, const double* pHigh,
	const double* pVLow, const double* pVHigh,
	double tStart, double tEnd, uint32_t dimension)
{
	m_startTime = tStart;
	m_endTime = tEnd;
	m_dimension = dimension;
	m_pLow = nullptr; m_pHigh = nullptr;
	m_pVLow = nullptr; m_pVHigh = nullptr;

	if (m_endTime <= m_startTime) throw Tools::IllegalArgumentException("MovingRegion: Cannot support degenerate time intervals.");

	try
	{
		m_pLow = new double[m_dimension];
		m_pHigh = new double[m_dimension];
		m_pVLow = new double[m_dimension];
		m_pVHigh = new double[m_dimension];
	}
	catch (...)
	{
		delete[] m_pLow;
		delete[] m_pHigh;
		delete[] m_pVLow;
		delete[] m_pVHigh;
		throw;
	}

	// first store the point coordinates, than the point velocities.
	memcpy(m_pLow, pLow, m_dimension * sizeof(double));
	memcpy(m_pHigh, pHigh, m_dimension * sizeof(double));
	memcpy(m_pVLow, pVLow, m_dimension * sizeof(double));
	memcpy(m_pVHigh, pVHigh, m_dimension * sizeof(double));
}

MovingRegion::~MovingRegion()
{
	delete[] m_pVLow;
	delete[] m_pVHigh;
}

MovingRegion& MovingRegion::operator=(const MovingRegion& r)
{
	if(this != &r)
	{
		makeDimension(r.m_dimension);
		memcpy(m_pLow, r.m_pLow, m_dimension * sizeof(double));
		memcpy(m_pHigh, r.m_pHigh, m_dimension * sizeof(double));
		memcpy(m_pVLow, r.m_pVLow, m_dimension * sizeof(double));
		memcpy(m_pVHigh, r.m_pVHigh, m_dimension * sizeof(double));

		m_startTime = r.m_startTime;
		m_endTime = r.m_endTime;

		assert(m_startTime < m_endTime);
	}

	return *this;
}

bool MovingRegion::operator==(const MovingRegion& r) const
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
			m_pHigh[i] > r.m_pHigh[i] + std::numeric_limits<double>::epsilon() ||
			m_pVLow[i] < r.m_pVLow[i] - std::numeric_limits<double>::epsilon() ||
			m_pVLow[i] > r.m_pVLow[i] + std::numeric_limits<double>::epsilon() ||
			m_pVHigh[i] < r.m_pVHigh[i] - std::numeric_limits<double>::epsilon() ||
			m_pVHigh[i] > r.m_pVHigh[i] + std::numeric_limits<double>::epsilon())
			return false;
	}
	return true;
}

bool MovingRegion::isShrinking() const
{
	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		if (m_pVHigh[cDim] < m_pVLow[cDim]) return true;
	}
	return false;
}

// assumes that the region is not moving before and after start and end time.
double MovingRegion::getLow(uint32_t d, double t) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	if (t > m_endTime) return m_pLow[d] + m_pVLow[d] * (m_endTime - m_startTime);
	else if (t < m_startTime) return m_pLow[d];
	else return m_pLow[d] + m_pVLow[d] * (t - m_startTime);
}

// assumes that the region is not moving before and after start and end time.
double MovingRegion::getHigh(uint32_t d, double t) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	if (t > m_endTime) return m_pHigh[d] + m_pVHigh[d] * (m_endTime - m_startTime);
	else if (t < m_startTime) return m_pHigh[d];
	else return m_pHigh[d] + m_pVHigh[d] * (t - m_startTime);
}

// assuming that the region kept moving.
double MovingRegion::getExtrapolatedLow(uint32_t d, double t) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	return m_pLow[d] + m_pVLow[d] * (t - m_startTime);
}

// assuming that the region kept moving.
double MovingRegion::getExtrapolatedHigh(uint32_t d, double t) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	return m_pHigh[d] + m_pVHigh[d] * (t - m_startTime);
}

double MovingRegion::getVLow(uint32_t d) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	return m_pVLow[d];
}

double MovingRegion::getVHigh(uint32_t d) const
{
	if (d >= m_dimension) throw Tools::IndexOutOfBoundsException(d);

	return m_pVHigh[d];
}

bool MovingRegion::intersectsRegionInTime(const MovingRegion& r) const
{
	Tools::Interval ivOut;
	return intersectsRegionInTime(r, ivOut);
}

bool MovingRegion::intersectsRegionInTime(const MovingRegion& r, IInterval& ivOut) const
{
	return intersectsRegionInTime(r, r, ivOut);
}

// if tmin, tmax are infinity then this will not work correctly (everything will always intersect).
// does not work for shrinking regions.
// does not work with degenerate time-intervals.
//
// WARNING: this will return true even if one region completely contains the other, since
// their areas do intersect in that case!
//
// there are various cases here:
// 1. one region contains the other.
// 2. one boundary of one region is always contained indide the other region, while the other
//    boundary is not (so no boundaries will ever intersect).
// 3. either the upper or lower boundary of one region intersects a boundary of the other.
bool MovingRegion::intersectsRegionInTime(const IInterval& ivPeriod, const MovingRegion& r, IInterval& ivOut) const
{
	if (m_dimension != r.m_dimension) throw Tools::IllegalArgumentException("intersectsRegionInTime: MovingRegions have different number of dimensions.");

	assert(m_startTime < m_endTime);
	assert(r.m_startTime < r.m_endTime);
	assert(ivPeriod.getLowerBound() < ivPeriod.getUpperBound());
	assert(isShrinking() == false && r.isShrinking() == false);

	// this is needed, since we are assuming below that the two regions have some point of intersection
	// inside itPeriod.
	if (containsRegionInTime(ivPeriod, r) || r.containsRegionInTime(ivPeriod, *this))
	{
		ivOut = ivPeriod;
		return true;
	}

	double tmin = std::max(m_startTime, r.m_startTime);
	double tmax = std::min(m_endTime, r.m_endTime);

	// the regions do not intersect in time.
	if (tmax <= tmin) return false;

	tmin = std::max(tmin, ivPeriod.getLowerBound());
	tmax = std::min(tmax, ivPeriod.getUpperBound());

	// the regions intersecting interval does not intersect with the given time period.
	if (tmax <= tmin) return false;

	assert(tmax < std::numeric_limits<double>::max());
	assert(tmin > -std::numeric_limits<double>::max());

	// I use projected low and high because they are faster and it does not matter.
	// The are also necessary for calculating the intersection point with reference time instant 0.0.

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
	assert(
		tmin >= ivPeriod.getLowerBound() && tmax <= ivPeriod.getUpperBound() &&
		tmin >= m_startTime && tmax <= m_endTime &&
		tmin >= r.m_startTime && tmax <= r.m_endTime);

		// completely above or bellow in i-th dimension
		if (
			(r.getExtrapolatedLow(cDim, tmin) > getExtrapolatedHigh(cDim, tmin) &&
			r.getExtrapolatedLow(cDim, tmax) >= getExtrapolatedHigh(cDim, tmax)) ||
			(r.getExtrapolatedHigh(cDim, tmin) < getExtrapolatedLow(cDim, tmin) &&
			r.getExtrapolatedHigh(cDim, tmax) <= getExtrapolatedLow(cDim, tmax)))
			return false;

		// otherwise they intersect inside this interval for sure. Care needs to be taken since
		// intersection does not necessarily mean that two line segments intersect. It could be
		// that one line segment is completely above/below another, in which case there is no intersection
		// point inside tmin, tmax, even though the two region areas do intersect.

		if (r.getExtrapolatedLow(cDim, tmin) > getExtrapolatedHigh(cDim, tmin))  // r above *this at tmin
		{
			tmin = (getExtrapolatedHigh(cDim, 0.0) - r.getExtrapolatedLow(cDim, 0.0)) / (r.getVLow(cDim) - getVHigh(cDim));
		}
		else if (r.getExtrapolatedHigh(cDim, tmin) < getExtrapolatedLow(cDim, tmin)) // r below *this at tmin
		{
			tmin = (getExtrapolatedLow(cDim, 0.0) - r.getExtrapolatedHigh(cDim, 0.0)) / (r.getVHigh(cDim) - getVLow(cDim));
		}
		// else they do not intersect and the boundary might be completely contained in this region.

		if (r.getExtrapolatedLow(cDim, tmax) > getExtrapolatedHigh(cDim, tmax))  // r above *this at tmax
		{
			tmax = (getExtrapolatedHigh(cDim, 0.0) - r.getExtrapolatedLow(cDim, 0.0)) / (r.getVLow(cDim) - getVHigh(cDim));
		}
		else if (r.getExtrapolatedHigh(cDim, tmax) < getExtrapolatedLow(cDim, tmax)) // r below *this at tmax
		{
			tmax = (getExtrapolatedLow(cDim, 0.0) - r.getExtrapolatedHigh(cDim, 0.0)) / (r.getVHigh(cDim) - getVLow(cDim));
		}
		// else they do not intersect and the boundary might be completely contained in this region.

		assert(tmin <= tmax);
	}

	assert(
		tmin >= ivPeriod.getLowerBound() && tmax <= ivPeriod.getUpperBound() &&
		tmin >= m_startTime && tmax <= m_endTime &&
		tmin >= r.m_startTime && tmax <= r.m_endTime);

	ivOut.setBounds(tmin, tmax);

	return true;
}

bool MovingRegion::containsRegionInTime(const MovingRegion& r) const
{
	return containsRegionInTime(r, r);
}

// does not work for shrinking regions.
// works fine for infinite bounds (both tmin and tmax).
// does not work with degenerate time-intervals.
//
// finds if during the intersecting time-interval of r and ivPeriod, r is completely contained in *this.
bool MovingRegion::containsRegionInTime(const IInterval& ivPeriod, const MovingRegion& r) const
{
	if (m_dimension != r.m_dimension) throw Tools::IllegalArgumentException("containsRegionInTime: MovingRegions have different number of dimensions.");

	assert(isShrinking() == false && r.isShrinking() == false);

	double tmin = std::max(ivPeriod.getLowerBound(), r.m_startTime);
	double tmax = std::min(ivPeriod.getUpperBound(), r.m_endTime);

	// it should be contained in time.
	// it does not make sense if this region is not defined for any part ot [tmin, tmax].
	if (tmax <= tmin || tmin < m_startTime || tmax > m_endTime) return false;

	double intersectionTime;

	// no need to take projected coordinates here, since tmin and tmax are always contained in
	// the regions intersecting time-intervals.
	assert(
		tmin >= ivPeriod.getLowerBound() && tmax <= ivPeriod.getUpperBound() &&
		tmin >= m_startTime && tmax <= m_endTime &&
		tmin >= r.m_startTime && tmax <= r.m_endTime);

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		// it should be contained at start time.
		if (r.getExtrapolatedHigh(cDim, tmin) > getExtrapolatedHigh(cDim, tmin) ||
			r.getExtrapolatedLow(cDim, tmin) < getExtrapolatedLow(cDim, tmin)) return false;

		// this will take care of infinite bounds.
		if (r.m_pVHigh[cDim] != m_pVHigh[cDim])
		{
			intersectionTime = (getExtrapolatedHigh(cDim, 0.0) - r.getExtrapolatedHigh(cDim, 0.0)) / (r.m_pVHigh[cDim] - m_pVHigh[cDim]);
			// if they intersect during this time-interval, then it is not contained.
			if (tmin < intersectionTime && intersectionTime < tmax) return false;
			if (tmin == intersectionTime && r.m_pVHigh[cDim] > m_pVHigh[cDim]) return false;
		}

		if (r.m_pVLow[cDim] != m_pVLow[cDim])
		{
			intersectionTime = (getExtrapolatedLow(cDim, 0.0) - r.getExtrapolatedLow(cDim, 0.0)) / (r.m_pVLow[cDim] - m_pVLow[cDim]);
			// if they intersect during this time-interval, then it is not contained.
			if (tmin < intersectionTime && intersectionTime < tmax) return false;
			if (tmin == intersectionTime && r.m_pVLow[cDim] < m_pVLow[cDim]) return false;
		}
	}

	return true;
}

bool MovingRegion::containsRegionAfterTime(double t, const MovingRegion& r) const
{
	Tools::Interval ivT(t, r.m_endTime);
	return containsRegionInTime(ivT, r);
}

// Returns the area swept by the rectangle in time, in d-dimensional space (without
// including the temporal dimension, that is).
// This is what Saltenis calls Margin (which is different than what Beckmann proposes,
// where he computes only the wireframe -- instead of the surface/volume/etc. -- of the MBR in any dimension).
double MovingRegion::getProjectedSurfaceAreaInTime() const
{
	return getProjectedSurfaceAreaInTime(*this);
}

double MovingRegion::getProjectedSurfaceAreaInTime(const IInterval& ivI) const
{
	double tmin = std::max(ivI.getLowerBound(), m_startTime);
	double tmax = std::min(ivI.getUpperBound(), m_endTime);

	assert(tmin > -std::numeric_limits<double>::max());
	assert(tmax < std::numeric_limits<double>::max());
	assert(tmin <= tmax);

	if (tmin >= tmax - std::numeric_limits<double>::epsilon() &&
		tmin <= tmax + std::numeric_limits<double>::epsilon())
		return 0.0;

	double dx1, dx2, dx3;
	double dv1, dv2, dv3;
	double H = tmax - tmin;

	if (m_dimension == 3)
	{
		dx3 = getExtrapolatedHigh(2, tmin) - getExtrapolatedLow(2, tmin);
		dv3 = getVHigh(2) - getVLow(2);
		dx2 = getExtrapolatedHigh(1, tmin) - getExtrapolatedLow(1, tmin);
		dv2 = getVHigh(1) - getVLow(1);
		dx1 = getExtrapolatedHigh(0, tmin) - getExtrapolatedLow(0, tmin);
		dv1 = getVHigh(0) - getVLow(0);
		return
			H * (dx1 + dx2 + dx3 + dx1*dx2 + dx1*dx3 + dx2*dx3) +
			H*H * (dv1 + dv2 + dv3 + dx1*dv2 + dv1*dx2 + dx1*dv3 +
			dv1*dx3 + dx2*dv3 + dv2*dx3) / 2.0 +
			H*H*H * (dv1*dv2 + dv1*dv3 + dv2*dv3) / 3.0;
	}
	else if (m_dimension == 2)
	{
		dx2 = getExtrapolatedHigh(1, tmin) - getExtrapolatedLow(1, tmin);
		dv2 = getVHigh(1) - getVLow(1);
		dx1 = getExtrapolatedHigh(0, tmin) - getExtrapolatedLow(0, tmin);
		dv1 = getVHigh(0) - getVLow(0);
		return H * (dx1 + dx2) + H * H * (dv1 + dv2) / 2.0;
	}
	else if (m_dimension == 1)
	{
		// marioh: why not use the increase of the length of the interval here?
		return 0.0;
	}
	else
	{
		throw Tools::IllegalStateException("getProjectedSurfaceAreaInTime: unsupported dimensionality.");
	}
}

double MovingRegion::getCenterDistanceInTime(const MovingRegion& r) const
{

	return getCenterDistanceInTime(r, r);
}

double MovingRegion::getCenterDistanceInTime(const IInterval& ivI, const MovingRegion& r) const
{
	if (m_dimension != r.m_dimension) throw Tools::IllegalArgumentException("getCenterDistanceInTime: MovingRegions have different number of dimensions.");

	assert(m_startTime < m_endTime);
	assert(r.m_startTime < r.m_endTime);
	assert(ivI.getLowerBound() < ivI.getUpperBound());

	double tmin = std::max(m_startTime, r.m_startTime);
	double tmax = std::min(m_endTime, r.m_endTime);

	// the regions do not intersect in time.
	if (tmax <= tmin) return 0.0;

	tmin = std::max(tmin, ivI.getLowerBound());
	tmax = std::min(tmax, ivI.getUpperBound());

	// the regions intersecting interval does not intersect with the given time period.
	if (tmax <= tmin) return 0.0;

	assert(tmax < std::numeric_limits<double>::max());
	assert(tmin > -std::numeric_limits<double>::max());

	if (tmin >= tmax - std::numeric_limits<double>::epsilon() &&
		tmin <= tmax + std::numeric_limits<double>::epsilon())
		return 0.0;

	double H = tmax - tmin;

	double* dx = new double[m_dimension];
	double* dv = new double[m_dimension];
	double a = 0.0, b = 0.0, c = 0.0, f = 0.0, l = 0.0, m = 0.0, n = 0.0;

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		dx[cDim] =
			(r.getExtrapolatedLow(cDim, tmin) + r.getExtrapolatedHigh(cDim, tmin)) / 2.0 -
			(getExtrapolatedLow(cDim, tmin) + getExtrapolatedHigh(cDim, tmin)) / 2.0;
		dv[cDim] =
			(r.getVLow(cDim) + r.getVHigh(cDim)) / 2.0 -
			(getVLow(cDim) + getVHigh(cDim)) / 2.0;
	}

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		a += dv[cDim] * dv[cDim];
		b += 2.0 * dx[cDim] * dv[cDim];
		c += dx[cDim] * dx[cDim];
	}

	delete[] dx;
	delete[] dv;

	if (a == 0.0 && c == 0.0) return 0.0;
	if (a == 0.0) return H * std::sqrt(c);
	if (c == 0.0) return H * H * std::sqrt(a) / 2.0;

	f = std::sqrt(a * H * H + b * H + c);
	l = 2.0 * a * H + b;
	m = 4.0 * a * c - b * b;
	n = 2.0 * std::sqrt(a);

	return (l * f + log(l / n + f) * m / n - b * std::sqrt(c) - std::log(b / n + std::sqrt(c)) * m / n) / (4.0 * a);
}

// does not work with degenerate time-intervals.
bool MovingRegion::intersectsRegionAtTime(double t, const MovingRegion& r) const
{
	if (m_dimension != r.m_dimension) throw Tools::IllegalArgumentException("intersectsRegionAtTime: MovingRegions have different number of dimensions.");

	// do they contain the time instant?
	if (! (m_startTime <= t && t < m_endTime && r.m_startTime <= t && t < r.m_endTime)) return false;

	// do they intersect at that time instant?
	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (getExtrapolatedLow(i, t) > r.getExtrapolatedHigh(i, t) || getExtrapolatedHigh(i, t) < r.getExtrapolatedLow(i, t)) return false;
	}
	return true;
}

// does not work with degenerate time-intervals.
bool MovingRegion::containsRegionAtTime(double t, const MovingRegion& r) const
{
	if (m_dimension != r.m_dimension) throw Tools::IllegalArgumentException("containsRegionAtTime: MovingRegions have different number of dimensions.");

	// do they contain the time instant?
	if (! (m_startTime <= t && t < m_endTime && r.m_startTime <= t && t < r.m_endTime)) return false;

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		if (getExtrapolatedLow(cDim, t) > r.getExtrapolatedLow(cDim, t) || getExtrapolatedHigh(cDim, t) < r.getExtrapolatedHigh(cDim, t)) return false;
	}
	return true;
}

bool MovingRegion::intersectsPointInTime(const MovingPoint& p) const
{
	Tools::Interval ivOut;
	return intersectsPointInTime(p, ivOut);
}

bool MovingRegion::intersectsPointInTime(const MovingPoint& p, IInterval& ivOut) const
{
	return intersectsPointInTime(p, p, ivOut);
}

// if tmin, tmax are infinity then this will not work correctly (everything will always intersect).
// does not work for shrinking regions.
// does not work with degenerate time-intervals.
// FIXME: don't know what happens if tmin is negative infinity.
//
// WARNING: This will return true even if the region completely contains the point, since
// in that case the point trajectory intersects the region area!
bool MovingRegion::intersectsPointInTime(const IInterval& ivPeriod, const MovingPoint& p, IInterval& ivOut) const
{
	if (m_dimension != p.m_dimension) throw Tools::IllegalArgumentException("intersectsPointInTime: MovingPoint has different number of dimensions.");

	assert(m_startTime < m_endTime);
	assert(p.m_startTime < p.m_endTime);
	assert(ivPeriod.getLowerBound() < ivPeriod.getUpperBound());
	assert(isShrinking() == false);

	if (containsPointInTime(ivPeriod, p))
	{
		ivOut = ivPeriod;
		return true;
	}

	double tmin = std::max(m_startTime, p.m_startTime);
	double tmax = std::min(m_endTime, p.m_endTime);

	// the shapes do not intersect in time.
	if (tmax <= tmin) return false;

	tmin = std::max(tmin, ivPeriod.getLowerBound());
	tmax = std::min(tmax, ivPeriod.getUpperBound());

	// the shapes intersecting interval does not intersect with the given time period.
	if (tmax <= tmin) return false;

	assert(tmax < std::numeric_limits<double>::max());
	assert(tmin > -std::numeric_limits<double>::max());

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		assert(
			tmin >= ivPeriod.getLowerBound() && tmax <= ivPeriod.getUpperBound() &&
			tmin >= m_startTime && tmax <= m_endTime &&
			tmin >= p.m_startTime && tmax <= p.m_endTime);

		// completely above or bellow in i-th dimension
		if ((p.getProjectedCoord(cDim, tmin) > getExtrapolatedHigh(cDim, tmin) &&
			p.getProjectedCoord(cDim, tmax) >= getExtrapolatedHigh(cDim, tmax)) ||
			(p.getProjectedCoord(cDim, tmin) < getExtrapolatedLow(cDim, tmin) &&
			p.getProjectedCoord(cDim, tmax) <= getExtrapolatedLow(cDim, tmax)))
			return false;

		// otherwise they intersect inside this interval for sure, since we know that the point is not contained,
		// so there is no need to check for 0 divisors, negative values, etc...

		// adjust tmin
		if (p.getProjectedCoord(cDim, tmin) > getExtrapolatedHigh(cDim, tmin))  // p above *this at tmin
		{
			tmin = (getExtrapolatedHigh(cDim, 0.0) - p.getProjectedCoord(cDim, 0.0)) / (p.getVCoord(cDim) - getVHigh(cDim));
		}
		else if (p.getProjectedCoord(cDim, tmin) < getExtrapolatedLow(cDim, tmin)) // p below *this at tmin
		{
			tmin = (getExtrapolatedLow(cDim, 0.0) - p.getProjectedCoord(cDim, 0.0)) / (p.getVCoord(cDim) - getVLow(cDim));
		}

		// adjust tmax
		if (p.getProjectedCoord(cDim, tmax) > getExtrapolatedHigh(cDim, tmax))  // p above *this at tmax
		{
			tmax = (getExtrapolatedHigh(cDim, 0.0) - p.getProjectedCoord(cDim, 0.0)) / (p.getVCoord(cDim) - getVHigh(cDim));
		}
		else if (p.getProjectedCoord(cDim, tmax) < getExtrapolatedLow(cDim, tmax)) // p below *this at tmax
		{
			tmax = (getExtrapolatedLow(cDim, 0.0) - p.getProjectedCoord(cDim, 0.0)) / (p.getVCoord(cDim) - getVLow(cDim));
		}

		if (tmin > tmax) return false;
	}

	ivOut.setBounds(tmin, tmax);

	return true;
}

bool MovingRegion::containsPointInTime(const MovingPoint& p) const
{
	return containsPointInTime(p, p);
}

// does not work for shrinking regions.
// works fine for infinite bounds (both tmin and tmax).
// does not work with degenerate time-intervals.
//
// finds if during the intersecting time-interval of p and ivPeriod, p is completely contained in *this.
bool MovingRegion::containsPointInTime(const IInterval& ivPeriod, const MovingPoint& p) const
{
	if (m_dimension != p.m_dimension) throw Tools::IllegalArgumentException("containsPointInTime: MovingPoint has different number of dimensions.");

	assert(isShrinking() == false);

	double tmin = std::max(ivPeriod.getLowerBound(), p.m_startTime);
	double tmax = std::min(ivPeriod.getUpperBound(), p.m_endTime);

	// it should be contained in time.
	if (tmax <= tmin || tmin < m_startTime || tmax > m_endTime) return false;

	double intersectionTime;

	assert(
		tmin >= ivPeriod.getLowerBound() && tmax <= ivPeriod.getUpperBound() &&
		tmin >= m_startTime && tmax <= m_endTime &&
		tmin >= p.m_startTime && tmax <= p.m_endTime);

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		// it should be contained at start time.
		if (p.getProjectedCoord(cDim, tmin) > getExtrapolatedHigh(cDim, tmin) ||
			p.getProjectedCoord(cDim, tmin) < getExtrapolatedLow(cDim, tmin)) return false;

		if (p.m_pVCoords[cDim] != m_pVHigh[cDim])
		{
			intersectionTime = (getExtrapolatedHigh(cDim, 0.0) - p.getProjectedCoord(cDim, 0.0)) / (p.m_pVCoords[cDim] - m_pVHigh[cDim]);
			// if they intersect during this time-interval, then it is not contained.
			if (tmin < intersectionTime && intersectionTime < tmax) return false;
			if (tmin == intersectionTime && p.m_pVCoords[cDim] > m_pVHigh[cDim]) return false;
		}

		if (p.m_pVCoords[cDim] != m_pVLow[cDim])
		{
			intersectionTime = (getExtrapolatedLow(cDim, 0.0) - p.getProjectedCoord(cDim, 0.0)) / (p.m_pVCoords[cDim] - m_pVLow[cDim]);
			// if they intersect during this time-interval, then it is not contained.
			if (tmin < intersectionTime && intersectionTime < tmax) return false;
			if (tmin == intersectionTime && p.m_pVCoords[cDim] < m_pVLow[cDim]) return false;
		}
	}

	return true;
}

void MovingRegion::combineRegionInTime(const MovingRegion& r)
{
	if (m_dimension != r.m_dimension) throw Tools::IllegalArgumentException("combineRegionInTime: MovingRegions have different number of dimensions.");

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		m_pLow[cDim] = std::min(getExtrapolatedLow(cDim, m_startTime), r.getExtrapolatedLow(cDim, m_startTime));
		m_pHigh[cDim] = std::max(getExtrapolatedHigh(cDim, m_startTime), r.getExtrapolatedHigh(cDim, m_startTime));
		m_pVLow[cDim] = std::min(m_pVLow[cDim], r.m_pVLow[cDim]);
		m_pVHigh[cDim] = std::max(m_pVHigh[cDim], r.m_pVHigh[cDim]);
	}

	// m_startTime should be modified at the end, since it affects the
	// calculation of extrapolated coordinates.
	m_startTime = std::min(m_startTime, r.m_startTime);
	m_endTime = std::max(m_endTime, r.m_endTime);
}

void MovingRegion::getCombinedRegionInTime(MovingRegion& out, const MovingRegion& in) const
{
	if (m_dimension != in.m_dimension) throw Tools::IllegalArgumentException("getCombinedProjectedRegionInTime: MovingRegions have different number of dimensions.");

	out = *this;
	out.combineRegionInTime(in);
}

void MovingRegion::combineRegionAfterTime(double t, const MovingRegion& r)
{
	if (m_dimension != r.m_dimension) throw Tools::IllegalArgumentException("combineRegionInTime: MovingRegions have different number of dimensions.");

	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		m_pLow[cDim] = std::min(getExtrapolatedLow(cDim, t), r.getExtrapolatedLow(cDim, t));
		m_pHigh[cDim] = std::max(getExtrapolatedHigh(cDim, t), r.getExtrapolatedHigh(cDim, t));
		m_pVLow[cDim] = std::min(m_pVLow[cDim], r.m_pVLow[cDim]);
		m_pVHigh[cDim] = std::max(m_pVHigh[cDim], r.m_pVHigh[cDim]);
	}

	// m_startTime should be modified at the end, since it affects the
	// calculation of extrapolated coordinates.
	m_startTime = t;
	m_endTime = std::max(m_endTime, r.m_endTime);
	if (t >= m_endTime) m_endTime = std::numeric_limits<double>::max();
}

void MovingRegion::getCombinedRegionAfterTime(double t, MovingRegion& out, const MovingRegion& in) const
{
	if (m_dimension != in.m_dimension) throw Tools::IllegalArgumentException("getCombinedProjectedRegionInTime: MovingRegions have different number of dimensions.");

	out = *this;
	out.combineRegionAfterTime(t, in);
}

double MovingRegion::getIntersectingAreaInTime(const MovingRegion& r) const
{
	return getIntersectingAreaInTime(r, r);
}

double MovingRegion::getIntersectingAreaInTime(const IInterval& ivI, const MovingRegion& r) const
{
	if (m_dimension != r.m_dimension) throw Tools::IllegalArgumentException("getIntersectingAreaInTime: MovingRegions have different number of dimensions.");

	assert(m_startTime < m_endTime);
	assert(r.m_startTime < r.m_endTime);
	assert(ivI.getLowerBound() < ivI.getUpperBound());
	assert(isShrinking() == false && r.isShrinking() == false);

	double tmin = std::max(m_startTime, r.m_startTime);
	double tmax = std::min(m_endTime, r.m_endTime);

	// the regions do not intersect in time.
	if (tmax <= tmin) return 0.0;

	tmin = std::max(tmin, ivI.getLowerBound());
	tmax = std::min(tmax, ivI.getUpperBound());

	// the regions intersecting interval does not intersect with the given time period.
	if (tmax <= tmin) return 0.0;

	assert(tmax < std::numeric_limits<double>::max());
	assert(tmin > -std::numeric_limits<double>::max());

	Tools::Interval ivIn(tmin, tmax);
	Tools::Interval ivOut(ivIn);

	if (! intersectsRegionInTime(ivIn, r, ivOut)) return 0.0;

	ivIn = ivOut;
	tmin = ivIn.getLowerBound();
	tmax = ivIn.getUpperBound();
	assert(tmin <= tmax);

	assert(tmin >= ivI.getLowerBound() && tmax <= ivI.getUpperBound());

	if (containsRegionInTime(ivIn, r))
	{
		return r.getAreaInTime(ivIn);
	}
	else if (r.containsRegionInTime(ivIn, *this))
	{
		return getAreaInTime(ivIn);
	}

	MovingRegion x = *this;
	CrossPoint c;
	auto ascending = [](CrossPoint& lhs, CrossPoint& rhs) { return lhs.m_t > rhs.m_t; };
	std::priority_queue < CrossPoint, std::vector<CrossPoint>, decltype(ascending)> pq(ascending);

	// find points of intersection in all dimensions.
	for (uint32_t i = 0; i < m_dimension; ++i)
	{
		if (getLow(i, tmin) > r.getLow(i, tmin))
		{
			x.m_pLow[i] = m_pLow[i];
			x.m_pVLow[i] = m_pVLow[i];

			if (getLow(i, tmax) < r.getLow(i, tmax))
			{
				c.m_dimension = i;
				c.m_boundary = 0;
				c.m_t = (getExtrapolatedLow(i, 0.0) - r.getExtrapolatedLow(i, 0.0)) / (r.getVLow(i) - getVLow(i));
				assert(c.m_t >= tmin && c.m_t <= tmax);
				c.m_to = &r;
				pq.push(c);
			}
		}
		else
		{
			x.m_pLow[i] = r.m_pLow[i];
			x.m_pVLow[i] = r.m_pVLow[i];

			if (r.getLow(i, tmax) < getLow(i, tmax))
			{
				c.m_dimension = i;
				c.m_boundary = 0;
				c.m_t = (getExtrapolatedLow(i, 0.0) - r.getExtrapolatedLow(i, 0.0)) / (r.getVLow(i) - getVLow(i));
				assert(c.m_t >= tmin && c.m_t <= tmax);
				c.m_to = this;
				pq.push(c);
			}
		}

		if (getHigh(i, tmin) < r.getHigh(i, tmin))
		{
			x.m_pHigh[i] = m_pHigh[i];
			x.m_pVHigh[i] = m_pVHigh[i];

			if (getHigh(i, tmax) > r.getHigh(i, tmax))
			{
				c.m_dimension = i;
				c.m_boundary = 1;
				c.m_t = (getExtrapolatedHigh(i, 0.0) - r.getExtrapolatedHigh(i, 0.0)) / (r.getVHigh(i) - getVHigh(i));
				assert(c.m_t >= tmin && c.m_t <= tmax);
				c.m_to = &r;
				pq.push(c);
			}
		}
		else
		{
			x.m_pHigh[i] = r.m_pHigh[i];
			x.m_pVHigh[i] = r.m_pVHigh[i];

			if (r.getHigh(i, tmax) > getHigh(i, tmax))
			{
				c.m_dimension = i;
				c.m_boundary = 1;
				c.m_t = (getExtrapolatedHigh(i, 0.0) - r.getExtrapolatedHigh(i, 0.0)) / (r.getVHigh(i) - getVHigh(i));
				assert(c.m_t >= tmin && c.m_t <= tmax);
				c.m_to = this;
				pq.push(c);
			}
		}
	}

	// add up the total area of the intersecting pieces.
	double area = 0.0;

	while (! pq.empty())
	{
		c = pq.top(); pq.pop();

		// needed in case two consecutive points have the same intersection time.
		if (c.m_t > tmin) area += x.getAreaInTime(Tools::Interval(tmin, c.m_t));

		if (c.m_boundary == 0)
		{
			x.m_pLow[c.m_dimension] = c.m_to->m_pLow[c.m_dimension];
			x.m_pVLow[c.m_dimension] = c.m_to->m_pVLow[c.m_dimension];
		}
		else
		{
			x.m_pHigh[c.m_dimension] = c.m_to->m_pHigh[c.m_dimension];
			x.m_pVHigh[c.m_dimension] = c.m_to->m_pVHigh[c.m_dimension];
		}

      	tmin = c.m_t;
   }

   // ... and the last piece
   if (tmax > tmin) area += x.getAreaInTime(Tools::Interval(tmin, tmax));

   return area;
}

//
// IObject interface
//
MovingRegion* MovingRegion::clone()
{
	return new MovingRegion(*this);
}

//
// ISerializable interface
//
uint32_t MovingRegion::getByteArraySize()
{
	return (sizeof(uint32_t) + 2 * sizeof(double) + 4 * m_dimension * sizeof(double));
}

void MovingRegion::loadFromByteArray(const uint8_t* ptr)
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
	ptr += m_dimension * sizeof(double);
	memcpy(m_pVLow, ptr, m_dimension * sizeof(double));
	ptr += m_dimension * sizeof(double);
	memcpy(m_pVHigh, ptr, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

void MovingRegion::storeToByteArray(uint8_t** data, uint32_t& len)
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
	ptr += m_dimension * sizeof(double);
	memcpy(ptr, m_pVLow, m_dimension * sizeof(double));
	ptr += m_dimension * sizeof(double);
	memcpy(ptr, m_pVHigh, m_dimension * sizeof(double));
	//ptr += m_dimension * sizeof(double);
}

//
// IEvolvingShape interface
//
void MovingRegion::getVMBR(Region& out) const
{
	out.makeDimension(m_dimension);
	memcpy(out.m_pLow, m_pVLow, m_dimension * sizeof(double));
	memcpy(out.m_pHigh, m_pVHigh, m_dimension * sizeof(double));
}

void MovingRegion::getMBRAtTime(double t, Region& out) const
{
	out.makeDimension(m_dimension);
	for (uint32_t cDim = 0; cDim < m_dimension; ++cDim)
	{
		out.m_pLow[cDim] = getLow(cDim, t);
		out.m_pHigh[cDim] = getHigh(cDim, t);
	}
}

//
// ITimeShape interface
//
double MovingRegion::getAreaInTime() const
{
	return getAreaInTime(*this);
}

// this computes the area/volume/etc. swept by the Region in time.
double MovingRegion::getAreaInTime(const IInterval& ivI) const
{
	double tmin = std::max(ivI.getLowerBound(), m_startTime);
	double tmax = std::min(ivI.getUpperBound(), m_endTime);

	assert(tmin > -std::numeric_limits<double>::max());
	assert(tmax < std::numeric_limits<double>::max());
	assert(tmin <= tmax);

	if (tmin >= tmax - std::numeric_limits<double>::epsilon() &&
		tmin <= tmax + std::numeric_limits<double>::epsilon())
		return 0.0;

	double dx1, dx2, dx3;
	double dv1, dv2, dv3;
	double H = tmax - tmin;

	if (m_dimension == 3)
	{
		dx3 = getExtrapolatedHigh(2, tmin) - getExtrapolatedLow(2, tmin);
		dv3 = getVHigh(2) - getVLow(2);
		dx2 = getExtrapolatedHigh(1, tmin) - getExtrapolatedLow(1, tmin);
		dv2 = getVHigh(1) - getVLow(1);
		dx1 = getExtrapolatedHigh(0, tmin) - getExtrapolatedLow(0, tmin);
		dv1 = getVHigh(0) - getVLow(0);
		return
			H * dx1 * dx2 * dx3 + H * H * (dx1 * dx2 * dv3 + (dx1 * dv2 + dv1 * dx2) * dx3) / 2.0 +
			H * H * H * ((dx1 * dv2 + dv1 * dx2) * dv3 + dv1 * dv2 * dx3) / 3.0 + H * H * H * H * dv1 * dv2 * dv3 / 4.0;
	}
	else if (m_dimension == 2)
	{
		dx2 = getExtrapolatedHigh(1, tmin) - getExtrapolatedLow(1, tmin);
		dv2 = getVHigh(1) - getVLow(1);
		dx1 = getExtrapolatedHigh(0, tmin) - getExtrapolatedLow(0, tmin);
		dv1 = getVHigh(0) - getVLow(0);
		return H * dx1 * dx2 + H * H * (dx1 * dv2 + dv1 * dx2) / 2.0 + H * H * H * dv1 * dv2 / 3.0;
	}
	else if (m_dimension == 1)
	{
		dx1 = getExtrapolatedHigh(0, tmin) - getExtrapolatedLow(0, tmin);
		dv1 = getVHigh(0) - getVLow(0);
		return H * dx1 + H * H * dv1 / 2.0;
	}
	else
	{
		throw Tools::NotSupportedException("getAreaInTime: unsupported dimensionality.");
	}
}

double MovingRegion::getIntersectingAreaInTime(const ITimeShape& r) const
{
	return getIntersectingAreaInTime(r, r);
}

double MovingRegion::getIntersectingAreaInTime(const IInterval&, const ITimeShape& in) const
{
	const MovingRegion* pr = dynamic_cast<const MovingRegion*>(&in);
	if (pr != nullptr) return getIntersectingAreaInTime(*pr);

	throw Tools::IllegalStateException("getIntersectingAreaInTime: Not implemented yet!");
}

void MovingRegion::makeInfinite(uint32_t dimension)
{
	makeDimension(dimension);
	for (uint32_t cIndex = 0; cIndex < m_dimension; ++cIndex)
	{
		m_pLow[cIndex] = std::numeric_limits<double>::max();
		m_pHigh[cIndex] = -std::numeric_limits<double>::max();
		m_pVLow[cIndex] = std::numeric_limits<double>::max();
		m_pVHigh[cIndex] = -std::numeric_limits<double>::max();
	}

	m_startTime = -std::numeric_limits<double>::max();
	m_endTime = std::numeric_limits<double>::max();
}

void MovingRegion::makeDimension(uint32_t dimension)
{
	if (m_dimension != dimension)
	{
		delete[] m_pLow;
		delete[] m_pHigh;
		delete[] m_pVLow;
		delete[] m_pVHigh;
		m_pLow = nullptr; m_pHigh = nullptr;
		m_pVLow = nullptr; m_pVHigh = nullptr;

		m_dimension = dimension;
		m_pLow = new double[m_dimension];
		m_pHigh = new double[m_dimension];
		m_pVLow = new double[m_dimension];
		m_pVHigh = new double[m_dimension];
	}
}

std::ostream& SpatialIndex::operator<<(std::ostream& os, const MovingRegion& r)
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

	os << "VLow: ";
	for (i = 0; i < r.m_dimension; ++i)
	{
		os << r.m_pVLow[i] << " ";
	}

	os << ", VHigh: ";

	for (i = 0; i < r.m_dimension; ++i)
	{
		os << r.m_pVHigh[i] << " ";
	}

	os << ", Start: " << r.m_startTime << ", End: " << r.m_endTime;

	return os;
}
