/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Peter Labadorf - plaba3.1415@gmail.com
 ******************************************************************************
 * Copyright (c) 2023, Peter Labadorf
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

#include <cmath>
#include <cstring>
#include <limits>

#include <spatialindex/SpatialIndex.h>

using namespace SpatialIndex;

Ball::Ball()
= default;

Ball::Ball(const Ball &b)
{
	m_centerPoint = b.m_centerPoint;
	m_radius = b.m_radius;
}

Ball::Ball(double radius, const Point &center)
{
	m_centerPoint = center;
	m_radius = radius;
}

Ball::Ball(double radius, const double *pCoords, uint32_t dimension)
{
	m_centerPoint = Point(pCoords, dimension);
	m_radius = radius;
}

Ball::~Ball()
= default;

Ball &Ball::operator=(const Ball &b)
{
	if(this != &b)
	{
		m_radius = b.m_radius;
		m_centerPoint = b.m_centerPoint;
	}
	return *this;
}

bool Ball::operator==(const Ball &b) const
{
	return fabs(m_radius - b.m_radius) <= std::numeric_limits<double>::epsilon()
		&& m_centerPoint == b.m_centerPoint;
}

//
// IObject interface
//
Ball *Ball::clone()
{
	return new Ball(*this);
}

//
// ISerializable interface
//
uint32_t Ball::getByteArraySize()
{
	return m_centerPoint.getByteArraySize() + sizeof(double);
}

void Ball::loadFromByteArray(const uint8_t *data)
{
	m_centerPoint.loadFromByteArray(data);
	data += m_centerPoint.getByteArraySize();
	memcpy(&m_radius, data, sizeof(double));
}

void Ball::storeToByteArray(uint8_t **data, uint32_t &length)
{
	uint32_t center_point_size;
	length = getByteArraySize();

	*data = new uint8_t[length];
	uint8_t *ptr = *data;

	m_centerPoint.storeToByteArray(&ptr, center_point_size);

	ptr += center_point_size;
	memcpy(ptr, &m_radius, sizeof(double));
}

//
// IShape interface
//
bool Ball::intersectsShape(const IShape &in) const
{
	return in.getMinimumDistance(m_centerPoint) <= m_radius;
}

bool Ball::containsShape(const IShape &in) const
{
	if (in.getDimension() != m_centerPoint.m_dimension)
		throw Tools::IllegalArgumentException(
			"Ball::containsShape: Shape has the wrong number of dimensions."
		);

	const Point *point = dynamic_cast<const Point*>(&in);
	if (point != nullptr) return containsPoint(point);

	const LineSegment *line = dynamic_cast<const LineSegment*>(&in);
	if (line != nullptr) return containsLineSegment(line);

	const Region *region = dynamic_cast<const Region*>(&in);
	if (region != nullptr) return containsRegion(region);

	const Ball *ball = dynamic_cast<const Ball*>(&in);
	if (ball != nullptr) return containsBall(ball);

	throw Tools::IllegalStateException(
		"Ball::intersectsShape: Not implemented yet!"
	);
}

bool Ball::touchesShape(const IShape &in) const
{
	return fabs(in.getMinimumDistance(m_centerPoint) - m_radius) <=
		std::numeric_limits<double>::epsilon();
}

void Ball::getCenter(Point &out) const
{
	out = m_centerPoint;
}

uint32_t Ball::getDimension() const
{
	return m_centerPoint.m_dimension;
}

void Ball::getMBR(Region &out) const
{
	out = Region(m_centerPoint, m_centerPoint);
	for (uint16_t i = 0; i < m_centerPoint.m_dimension; i++)
	{
		out.m_pLow[i] -= m_radius;
		out.m_pHigh[i] += m_radius;
	}
}

double Ball::getArea() const
{
	return std::pow(m_radius, m_centerPoint.m_dimension) *
		std::pow(M_PI, m_centerPoint.m_dimension / 2) /
		std::tgamma(m_centerPoint.m_dimension / 2 + 1);
}

double Ball::getMinimumDistance(const IShape &in) const
{
	return std::max(in.getMinimumDistance(m_centerPoint) - m_radius, 0.0);
}

bool Ball::containsRegion(const Region *region) const
{
	double sum = 0;
	for (uint32_t i = 0; i < m_centerPoint.m_dimension; i++)
	{
		double furthest = std::max(std::abs(m_centerPoint.m_pCoords[i] - region->m_pLow[i]),
			std::abs(region->m_pHigh[i] - m_centerPoint.m_pCoords[i]));
		sum += furthest * furthest;
	}
	return sum <= m_radius * m_radius;
}

bool Ball::containsLineSegment(const LineSegment *line) const
{
	double sum = 0;
	for (uint32_t i = 0; i < m_centerPoint.m_dimension; i++)
	{
		double d = line->m_pStartPoint[i] - m_centerPoint.m_pCoords[i];
		sum += d * d;
	}
	if (sum > m_radius * m_radius) return false;

	sum = 0;
	for (uint32_t i = 0; i < m_centerPoint.m_dimension; i++)
	{
		double d = line->m_pEndPoint[i] - m_centerPoint.m_pCoords[i];
		sum += d * d;
	}

	return sum <= m_radius * m_radius;
}

std::ostream &SpatialIndex::operator<<(std::ostream &os, const Ball &ball)
{
	os << ball.m_centerPoint << " " << ball.m_radius << " ";
	return os;
}
