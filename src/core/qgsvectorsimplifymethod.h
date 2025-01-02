/***************************************************************************
    qgsvectorsimplifymethod.h
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alvaro Huarte
    email                : http://wiki.osgeo.org/wiki/Alvaro_Huarte
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORSIMPLIFYMETHOD_H
#define QGSVECTORSIMPLIFYMETHOD_H

#include <QFlags>
#include <QObject>

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"

/**
 * \ingroup core
 * \brief This class contains information how to simplify geometries fetched from a vector layer
 */
class CORE_EXPORT QgsVectorSimplifyMethod
{
    Q_GADGET
  public:
    //! construct a default object
    QgsVectorSimplifyMethod();

    //! Sets the simplification hints of the vector layer managed
    void setSimplifyHints( Qgis::VectorRenderingSimplificationFlags simplifyHints ) { mSimplifyHints = simplifyHints; }
    //! Gets the simplification hints of the vector layer managed
    inline Qgis::VectorRenderingSimplificationFlags simplifyHints() const { return mSimplifyHints; }

    //! Sets the local simplification algorithm of the vector layer managed
    void setSimplifyAlgorithm( Qgis::VectorSimplificationAlgorithm simplifyAlgorithm ) { mSimplifyAlgorithm = simplifyAlgorithm; }
    //! Gets the local simplification algorithm of the vector layer managed
    inline Qgis::VectorSimplificationAlgorithm simplifyAlgorithm() const { return mSimplifyAlgorithm; }

    //! Sets the tolerance of simplification in map units. Represents the maximum distance in map units between two coordinates which can be considered equal
    void setTolerance( double tolerance ) { mTolerance = tolerance; }
    //! Gets the tolerance of simplification in map units. Represents the maximum distance in map units between two coordinates which can be considered equal
    inline double tolerance() const { return mTolerance; }

    //! Sets the simplification threshold of the vector layer managed
    void setThreshold( float threshold ) { mThreshold = threshold; }
    //! Gets the simplification threshold of the vector layer managed
    inline float threshold() const { return mThreshold; }

    //! Sets where the simplification executes, after fetch the geometries from provider, or when supported, in provider before fetch the geometries
    void setForceLocalOptimization( bool localOptimization ) { mLocalOptimization = localOptimization; }
    //! Gets where the simplification executes, after fetch the geometries from provider, or when supported, in provider before fetch the geometries
    inline bool forceLocalOptimization() const { return mLocalOptimization; }

    //! Sets the maximum scale at which the layer should be simplified
    void setMaximumScale( float maximumScale ) { mMaximumScale = maximumScale; }
    //! Gets the maximum scale at which the layer should be simplified
    inline float maximumScale() const { return mMaximumScale; }

  private:
    //! Simplification hints for fast rendering of features of the vector layer managed
    Qgis::VectorRenderingSimplificationFlags mSimplifyHints;
    //! Simplification algorithm
    Qgis::VectorSimplificationAlgorithm mSimplifyAlgorithm = Qgis::VectorSimplificationAlgorithm::Distance;
    //! Simplification tolerance, it represents the maximum distance between two coordinates which can be considered equal
    double mTolerance = 1;
    //! Simplification threshold
    float mThreshold;
    //! Simplification executes after fetch the geometries from provider, otherwise it executes, when supported, in provider before fetch the geometries
    bool mLocalOptimization = true;
    //! Maximum scale at which the layer should be simplified (Maximum scale at which generalisation should be carried out)
    float mMaximumScale = 1;
};


#endif // QGSVECTORSIMPLIFYMETHOD_H
