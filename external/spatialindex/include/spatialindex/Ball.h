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

#pragma once

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace SpatialIndex
{

	class SIDX_DLL Ball: public Tools::IObject, public virtual IShape
	{
	public:
		Ball();
		Ball(double radius, const Point& center);
		Ball(double radius, const double *pCoords, uint32_t dimension);
		Ball(const Ball& b);
		~Ball() override;

		virtual Ball& operator=(const Ball& b);
		virtual bool operator==(const Ball& b) const;

		//
		// IObject interface
		//

		Ball* clone() override;

		//
		// ISerializable interface
		//

		uint32_t getByteArraySize() override;
		void loadFromByteArray(const uint8_t *data) override;
		void storeToByteArray(uint8_t **data, uint32_t &length) override;

		//
		// IShape interface
		//

		bool intersectsShape(const IShape &in) const override;
		bool containsShape(const IShape &in) const override;
		bool touchesShape(const IShape &in) const override;
		void getCenter(SpatialIndex::Point &out) const override;
		uint32_t getDimension() const override;
		void getMBR(SpatialIndex::Region &out) const override;
		double getArea() const override;
		double getMinimumDistance(const IShape &in) const override;

		virtual bool containsLineSegment(const SpatialIndex::LineSegment *line) const;
		virtual bool containsRegion(const SpatialIndex::Region *region) const;

		inline bool containsPoint(const Point *point) const
		{
			return getMinimumDistance(*point) <= m_centerPoint.m_dimension;
		}

		inline bool containsBall(const Ball *ball) const
		{
			return getMinimumDistance(ball->m_centerPoint) + ball->m_radius <= m_radius;
		}

	public:
		double m_radius{0.0};
		Point m_centerPoint;

	}; // Ball

	SIDX_DLL std::ostream& operator<<(std::ostream& os, const Ball& ball);

}

