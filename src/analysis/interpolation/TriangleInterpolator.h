/***************************************************************************
                          TriangleInterpolator.h  -  description
                             -------------------
    copyright            : (C) 2004 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef TINTERPOLATOR_H
#define TINTERPOLATOR_H

#include "Vector3D.h"

#include "qgis_sip.h"
#include "qgis_analysis.h"

class QgsPoint;

#define SIP_NO_FILE

/**
 * \ingroup analysis
 * This is an interface for interpolator classes for triangulations.
 * \note Not available in Python bindings.
*/
class ANALYSIS_EXPORT TriangleInterpolator
{
  public:
    virtual ~TriangleInterpolator() = default;
    //! Calculates the normal vector and assigns it to vec
    virtual bool calcNormVec( double x, double y, QgsPoint &result SIP_OUT ) = 0;
    //! Performs a linear interpolation in a triangle and assigns the x-,y- and z-coordinates to point
    virtual bool calcPoint( double x, double y, QgsPoint &result SIP_OUT ) = 0;
};

#endif








