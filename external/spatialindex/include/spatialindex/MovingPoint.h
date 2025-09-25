/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2003, Marios Hadjieleftheriou
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
	class SIDX_DLL MovingPoint : public TimePoint, public IEvolvingShape
	{
	public:
		MovingPoint();
		MovingPoint(const double* pCoords, const double* pVCoords, const Tools::IInterval& ti, uint32_t dimension);
		MovingPoint(const double* pCoords, const double* pVCoords, double tStart, double tEnd, uint32_t dimension);
		MovingPoint(const Point& p, const Point& vp, const Tools::IInterval& ti);
		MovingPoint(const Point& p, const Point& vp, double tStart, double tEnd);
		MovingPoint(const MovingPoint& p);
		~MovingPoint() override;

		virtual MovingPoint& operator=(const MovingPoint& p);
		virtual bool operator==(const MovingPoint& p) const;

		virtual double getCoord(uint32_t index, double t) const;
		virtual double getProjectedCoord(uint32_t index, double t) const;
		virtual double getVCoord(uint32_t index) const;
		virtual void getPointAtTime(double t, Point& out) const;

		//
		// IObject interface
		//
		MovingPoint* clone() override;

		//
		// ISerializable interface
		//
		uint32_t getByteArraySize() override;
		void loadFromByteArray(const uint8_t* data) override;
		void storeToByteArray(uint8_t** data, uint32_t& len) override;

		//
		// IEvolvingShape interface
		//
		void getVMBR(Region& out) const override;
		void getMBRAtTime(double t, Region& out) const override;

		void makeInfinite(uint32_t dimension) override;
		void makeDimension(uint32_t dimension) override;

	private:
		void initialize(
			const double* pCoords, const double* pVCoords,
			double tStart, double tEnd, uint32_t dimension);

	public:
		double* m_pVCoords;

		friend SIDX_DLL std::ostream& operator<<(std::ostream& os, const MovingPoint& pt);
	}; // MovingPoint

	SIDX_DLL std::ostream& operator<<(std::ostream& os, const MovingPoint& pt);
}

