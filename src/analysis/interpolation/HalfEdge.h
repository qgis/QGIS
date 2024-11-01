/***************************************************************************
                          HalfEdge.h  -  description
                             -------------------
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HALFEDGE_H
#define HALFEDGE_H

#include "qgis_analysis.h"

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * \class HalfEdge
 * \brief HalfEdge
 * \note Not available in Python bindings.
 */
class ANALYSIS_EXPORT HalfEdge
{
  protected:
    //! Number of the dual HalfEdge
    int mDual = -10;
    //! Number of the next HalfEdge
    int mNext = -10;
    //! Number of the point at which this HalfEdge points
    int mPoint = -10;
    //! True, if the HalfEdge belongs to a break line, FALSE otherwise
    bool mBreak = false;
    //! True, if the HalfEdge belongs to a constrained edge, FALSE otherwise
    bool mForced = false;

  public:
    //! Default constructor. Values for mDual, mNext, mPoint are set to -10 which means that they are undefined
    HalfEdge() = default;
    HalfEdge( int dual, int next, int point, bool mbreak, bool forced );

    //! Returns the number of the dual HalfEdge
    int getDual() const;
    //! Returns the number of the next HalfEdge
    int getNext() const;
    //! Returns the number of the point at which this HalfEdge points
    int getPoint() const;
    //! Returns, whether the HalfEdge belongs to a break line or not
    bool getBreak() const;
    //! Returns, whether the HalfEdge belongs to a constrained edge or not
    bool getForced() const;
    //! Sets the number of the dual HalfEdge
    void setDual( int d );
    //! Sets the number of the next HalfEdge
    void setNext( int n );
    //! Sets the number of point at which this HalfEdge points
    void setPoint( int p );
    //! Sets the break flag
    void setBreak( bool b );
    //! Sets the forced flag
    void setForced( bool f );
};

#ifndef SIP_RUN

inline HalfEdge::HalfEdge( int dual, int next, int point, bool mbreak, bool forced )
  : mDual( dual ), mNext( next ), mPoint( point ), mBreak( mbreak ), mForced( forced )
{
}

inline int HalfEdge::getDual() const
{
  return mDual;
}

inline int HalfEdge::getNext() const
{
  return mNext;
}

inline int HalfEdge::getPoint() const
{
  return mPoint;
}

inline bool HalfEdge::getBreak() const
{
  return mBreak;
}

inline bool HalfEdge::getForced() const
{
  return mForced;
}

inline void HalfEdge::setDual( int d )
{
  mDual = d;
}

inline void HalfEdge::setNext( int n )
{
  mNext = n;
}

inline void HalfEdge::setPoint( int p )
{
  mPoint = p;
}

inline void HalfEdge::setBreak( bool b )
{
  mBreak = b;
}

inline void HalfEdge::setForced( bool f )
{
  mForced = f;
}

#endif

#endif
