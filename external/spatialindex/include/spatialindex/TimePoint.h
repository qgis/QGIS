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

#pragma once

namespace SpatialIndex
{
	class SIDX_DLL TimePoint : public Point, public ITimeShape
	{
	public:
		TimePoint();
		TimePoint(const double* pCoords, const Tools::IInterval& ti, uint32_t dimension);
		TimePoint(const double* pCoords, double tStart, double tEnd, uint32_t dimension);
		TimePoint(const Point& p, const Tools::IInterval& ti);
		TimePoint(const Point& p, double tStart, double tEnd);
		TimePoint(const TimePoint& p);
		~TimePoint() override;

		virtual TimePoint& operator=(const TimePoint& p);
		virtual bool operator==(const TimePoint& p) const;

		//
		// IObject interface
		//
		TimePoint* clone() override;

		//
		// ISerializable interface
		//
		uint32_t getByteArraySize() override;
		void loadFromByteArray(const uint8_t* data) override;
		void storeToByteArray(uint8_t** data, uint32_t& len) override;

		//
		// ITimeShape interface
		//
		bool intersectsShapeInTime(const ITimeShape& in) const override;
		bool intersectsShapeInTime(const Tools::IInterval& ivI, const ITimeShape& in) const override;
		bool containsShapeInTime(const ITimeShape& in) const override;
		bool containsShapeInTime(const Tools::IInterval& ivI, const ITimeShape& in) const override;
		bool touchesShapeInTime(const ITimeShape& in) const override;
		bool touchesShapeInTime(const Tools::IInterval& ivI, const ITimeShape& in) const override;
		double getAreaInTime() const override;
		double getAreaInTime(const Tools::IInterval& ivI) const override;
		double getIntersectingAreaInTime(const ITimeShape& r) const override;
		double getIntersectingAreaInTime(const Tools::IInterval& ivI, const ITimeShape& r) const override;

		//
		// IInterval interface
		//
		virtual Tools::IInterval& operator=(const Tools::IInterval&);
		double getLowerBound() const override;
		double getUpperBound() const override;
		void setBounds(double, double) override;
		bool intersectsInterval(const Tools::IInterval& ti) const override;
		bool intersectsInterval(Tools::IntervalType t, const double start, const double end) const override;
		bool containsInterval(const Tools::IInterval& ti) const override;
		Tools::IntervalType getIntervalType() const override;

		void makeInfinite(uint32_t dimension) override;
		void makeDimension(uint32_t dimension) override;

	public:
		double m_startTime;
		double m_endTime;

		friend SIDX_DLL std::ostream& operator<<(std::ostream& os, const TimePoint& pt);
	}; // TimePoint

	SIDX_DLL std::ostream& operator<<(std::ostream& os, const TimePoint& pt);
}
