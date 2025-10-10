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
	class SIDX_DLL LineSegment : public Tools::IObject, public virtual IShape
	{
	public:
		LineSegment();
		LineSegment(const double* startPoint, const double* endPoint, uint32_t dimension);
		LineSegment(const Point& startPoint, const Point& endPoint);
		LineSegment(const LineSegment& l);
		~LineSegment() override;

		virtual LineSegment& operator=(const LineSegment& p);
		virtual bool operator==(const LineSegment& p) const;

		//
		// IObject interface
		//
		LineSegment* clone() override;

		//
		// ISerializable interface
		//
		uint32_t getByteArraySize() override;
		void loadFromByteArray(const uint8_t* data) override;
		void storeToByteArray(uint8_t** data, uint32_t& length) override;

		//
		// IShape interface
		//
		bool intersectsShape(const IShape& in) const override;
		bool containsShape(const IShape& in) const override;
		bool touchesShape(const IShape& in) const override;
		void getCenter(Point& out) const override;
		uint32_t getDimension() const override;
		void getMBR(Region& out) const override;
		double getArea() const override;
		double getMinimumDistance(const IShape& in) const override;

		virtual bool intersectsLineSegment(const LineSegment& l) const;
		virtual bool intersectsRegion(const Region& p) const;
		virtual double getMinimumDistance(const Point& p) const;
		//virtual double getMinimumDistance(const Region& r) const;
		virtual double getRelativeMinimumDistance(const Point& p) const;
		virtual double getRelativeMaximumDistance(const Region& r) const;
		virtual double getAngleOfPerpendicularRay();

		virtual void makeInfinite(uint32_t dimension);
		virtual void makeDimension(uint32_t dimension);
        
	public:
		uint32_t m_dimension{0};
		double* m_pStartPoint{nullptr};
		double* m_pEndPoint{nullptr};

		friend class Region;
		friend class Point;
		friend SIDX_DLL std::ostream& operator<<(std::ostream& os, const LineSegment& pt);

    protected:

        //some helpers for intersects methods
        static double doubleAreaTriangle(const Point& a, const Point& b, const Point& c); 
        static bool leftOf(const Point& a, const Point& b, const Point& c); 
        static bool collinear(const Point& a, const Point& b, const Point& c); 
        static bool between(const Point& a, const Point& b, const Point& c); 
        static bool between(double a, double b, double c); 
        static bool intersectsProper(const Point& a, const Point& b, const Point& c, const Point& d); 
        static bool intersects(const Point& a, const Point& b, const Point& c, const Point& d); 

	}; // LineSegment

	SIDX_DLL std::ostream& operator<<(std::ostream& os, const LineSegment& pt);
}

