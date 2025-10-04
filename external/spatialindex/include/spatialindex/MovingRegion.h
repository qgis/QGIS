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
	class SIDX_DLL MovingRegion : public TimeRegion, public IEvolvingShape
	{
        using Region::getLow;
        using Region::getHigh;
        using TimeRegion::intersectsRegionInTime;
        using TimeRegion::containsRegionInTime;
        using TimeRegion::combineRegionInTime;
        using TimeRegion::getCombinedRegionInTime;
        using TimeRegion::containsPointInTime;
        
	public:
		MovingRegion();
		MovingRegion(
			const double* pLow, const double* pHigh,
			const double* pVLow, const double* pVHigh,
			const Tools::IInterval& ti, uint32_t dimension);
		MovingRegion(
			const double* pLow, const double* pHigh,
			const double* pVLow, const double* pVHigh,
			double tStart, double tEnd, uint32_t dimension);
		MovingRegion(
			const Point& low, const Point& high,
			const Point& vlow, const Point& vhigh,
			const Tools::IInterval& ti);
		MovingRegion(
			const Point& low, const Point& high,
			const Point& vlow, const Point& vhigh,
			double tStart, double tEnd);
		MovingRegion(const Region& mbr, const Region& vbr, const Tools::IInterval& ivI);
		MovingRegion(const Region& mbr, const Region& vbr, double tStart, double tEnd);
		MovingRegion(const MovingPoint& low, const MovingPoint& high);
		MovingRegion(const MovingRegion& in);
		~MovingRegion() override;

		virtual MovingRegion& operator=(const MovingRegion& r);
		virtual bool operator==(const MovingRegion&) const;

		bool isShrinking() const;

		virtual double getLow(uint32_t index, double t) const;
		virtual double getHigh(uint32_t index, double t) const;
		virtual double getExtrapolatedLow(uint32_t index, double t) const;
		virtual double getExtrapolatedHigh(uint32_t index, double t) const;
		virtual double getVLow(uint32_t index) const;
		virtual double getVHigh(uint32_t index) const;

		virtual bool intersectsRegionInTime(const MovingRegion& r) const;
		virtual bool intersectsRegionInTime(const MovingRegion& r, Tools::IInterval& out) const;
		virtual bool intersectsRegionInTime(const Tools::IInterval& ivI, const MovingRegion& r, Tools::IInterval& ret) const;
		virtual bool containsRegionInTime(const MovingRegion& r) const;
		virtual bool containsRegionInTime(const Tools::IInterval& ivI, const MovingRegion& r) const;
		virtual bool containsRegionAfterTime(double t, const MovingRegion& r) const;

		virtual double getProjectedSurfaceAreaInTime() const;
		virtual double getProjectedSurfaceAreaInTime(const Tools::IInterval& ivI) const;

		virtual double getCenterDistanceInTime(const MovingRegion& r) const;
		virtual double getCenterDistanceInTime(const Tools::IInterval& ivI, const MovingRegion& r) const;

		virtual bool intersectsRegionAtTime(double t, const MovingRegion& r) const;
		virtual bool containsRegionAtTime(double t, const MovingRegion& r) const;

		virtual bool intersectsPointInTime(const MovingPoint& p) const;
		virtual bool intersectsPointInTime(const MovingPoint& p, Tools::IInterval& out) const;
		virtual bool intersectsPointInTime(const Tools::IInterval& ivI, const MovingPoint& p, Tools::IInterval& out) const;
		virtual bool containsPointInTime(const MovingPoint& p) const;
		virtual bool containsPointInTime(const Tools::IInterval& ivI, const MovingPoint& p) const;

		//virtual bool intersectsPointAtTime(double t, const MovingRegion& in) const;
		//virtual bool containsPointAtTime(double t, const MovingRegion& in) const;

		virtual void combineRegionInTime(const MovingRegion& r);
		virtual void combineRegionAfterTime(double t, const MovingRegion& r);
		virtual void getCombinedRegionInTime(MovingRegion& out, const MovingRegion& in) const;
		virtual void getCombinedRegionAfterTime(double t, MovingRegion& out, const MovingRegion& in) const;

		virtual double getIntersectingAreaInTime(const MovingRegion& r) const;
		virtual double getIntersectingAreaInTime(const Tools::IInterval& ivI, const MovingRegion& r) const;

		//
		// IObject interface
		//
		MovingRegion* clone() override;

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

		//
		// ITimeShape interface
		//
		double getAreaInTime() const override;
		double getAreaInTime(const Tools::IInterval& ivI) const override;
		double getIntersectingAreaInTime(const ITimeShape& r) const override;
		double getIntersectingAreaInTime(const Tools::IInterval& ivI, const ITimeShape& r) const override;

		void makeInfinite(uint32_t dimension) override;
		void makeDimension(uint32_t dimension) override;

	private:
		void initialize(
			const double* pLow, const double* pHigh,
			const double* pVLow, const double* pVHigh,
			double tStart, double tEnd, uint32_t dimension);

	public:
		class CrossPoint
		{
		public:
			double m_t;
			uint32_t m_dimension;
			uint32_t m_boundary;
			const MovingRegion* m_to;

		}; // CrossPoint

	public:
		double* m_pVLow{nullptr};
		double* m_pVHigh{nullptr};

		friend SIDX_DLL std::ostream& operator<<(std::ostream& os, const MovingRegion& r);
	}; // MovingRegion

	typedef Tools::PoolPointer<MovingRegion> MovingRegionPtr;
	SIDX_DLL std::ostream& operator<<(std::ostream& os, const MovingRegion& r);
}
