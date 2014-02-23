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

/** This class contains information how to simplify geometries fetched from a vector layer
 * @note added in 2.2
 */
class CORE_EXPORT QgsVectorSimplifyMethod
{
  public:
    //! construct a default object
    QgsVectorSimplifyMethod();
    //! copy constructor
    QgsVectorSimplifyMethod( const QgsVectorSimplifyMethod& rh );
    //! assignment operator
    QgsVectorSimplifyMethod& operator=( const QgsVectorSimplifyMethod& rh );

    /** Simplification flags for fast rendering of features */
    enum SimplifyHint
    {
      NoSimplification           = 0, //!< No simplification can be applied
      GeometrySimplification     = 1, //!< The geometries can be simplified using the current map2pixel context state
      AntialiasingSimplification = 2, //!< The geometries can be rendered with 'AntiAliasing' disabled because of it is '1-pixel size'
      FullSimplification         = 3, //!< All simplification hints can be applied ( Geometry + AA-disabling )
    };
    Q_DECLARE_FLAGS( SimplifyHints, SimplifyHint )

    /** Sets the simplification hints of the vector layer managed */
    void setSimplifyHints( SimplifyHints simplifyHints ) { mSimplifyHints = simplifyHints; }
    /** Gets the simplification hints of the vector layer managed */
    inline SimplifyHints simplifyHints() const { return mSimplifyHints; }

    /** Sets the simplification threshold of the vector layer managed */
    void setThreshold( float threshold ) { mThreshold = threshold; }
    /** Gets the simplification threshold of the vector layer managed */
    inline float threshold() const { return mThreshold; }

    /** Sets where the simplification executes, after fetch the geometries from provider, or when supported, in provider before fetch the geometries */
    void setForceLocalOptimization( bool localOptimization ) { mLocalOptimization = localOptimization; }
    /** Gets where the simplification executes, after fetch the geometries from provider, or when supported, in provider before fetch the geometries */
    inline bool forceLocalOptimization() const { return mLocalOptimization; }

    /** Sets the maximum scale at which the layer should be simplified */
    void setMaximumScale( float maximumScale ) { mMaximumScale = maximumScale; }
    /** Gets the maximum scale at which the layer should be simplified */
    inline float maximumScale() const { return mMaximumScale; }

  private:
    /** Simplification hints for fast rendering of features of the vector layer managed */
    SimplifyHints mSimplifyHints;
    /** Simplification threshold */
    float mThreshold;
    /** Simplification executes after fetch the geometries from provider, otherwise it executes, when supported, in provider before fetch the geometries */
    bool mLocalOptimization;
    /** Maximum scale at which the layer should be simplified (Maximum scale at which generalisation should be carried out) */
    float mMaximumScale;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsVectorSimplifyMethod::SimplifyHints )

#endif // QGSVECTORSIMPLIFYMETHOD_H
