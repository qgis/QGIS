/***************************************************************************
  qgsellipsoidutils.h
 --------------------
  Date                 : April 2017
  Copyright            : (C) 2017 by Nyall Dawson
  email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSELLIPSOIDUTILS_H
#define QGSELLIPSOIDUTILS_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgscoordinatereferencesystem.h"

/**
 * \class QgsEllipsoidUtils
 * \ingroup core
 * Contains utility functions for working with ellipsoids and querying the ellipsoid database.
 *
 * \since QGIS 3.0
*/
class CORE_EXPORT QgsEllipsoidUtils
{
  public:

    /**
     * Contains parameter definitions for an ellipsoid.
     * \since QGIS 3.0
     */
    struct EllipsoidParameters
    {
      //! Whether ellipsoid parameters are valid
      bool valid{ true };

      //! Semi-major axis
      double semiMajor{ -1.0 };
      //! Semi-minor axis
      double semiMinor{ -1.0 };

      //! Whether custom parameters alone should be used (semiMajor/semiMinor only)
      bool useCustomParameters{ false };

      //! Inverse flattening
      double inverseFlattening{ -1.0 };

      //! Associated coordinate reference system
      QgsCoordinateReferenceSystem crs;
    };

    /**
     * Returns the parameters for the specified \a ellipsoid.
     * Results are cached to allow for fast retrieval of parameters.
     */
    static EllipsoidParameters ellipsoidParameters( const QString &ellipsoid );

  private:

    // ellipsoid cache
    static QReadWriteLock sEllipsoidCacheLock;
    static QHash< QString, EllipsoidParameters > sEllipsoidCache;

};

#endif // QGSELLIPSOIDUTILS_H

