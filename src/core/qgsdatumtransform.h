/***************************************************************************
               qgsdatumtransform.h
               ------------------------
    begin                : Dec 2017
    copyright            : (C) 2017 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDATUMTRANSFORM_H
#define QGSDATUMTRANSFORM_H

#include "qgis_core.h"
#include <QString>

/**
 * Contains methods and classes relating the datum transformations.
 * \ingroup core
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsDatumTransform
{

  public:

    /**
     * Contains datum transform information.
     * \since QGIS 3.0
     */
    struct TransformPair
    {

      /**
       * Constructor for a TransformPair with the specified \a sourceTransformId
       * and \a destinationTransformId transforms.
       */
      TransformPair( int sourceTransformId = -1, int destinationTransformId = -1 )
        : sourceTransformId( sourceTransformId )
        , destinationTransformId( destinationTransformId )
      {}

      /**
        * ID for the datum transform to use when projecting from the source CRS.
        * \see QgsCoordinateTransform::datumTransformCrsInfo()
       */
      int sourceTransformId = -1;

      /**
       * ID for the datum transform to use when projecting to the destination CRS.
       * \see QgsCoordinateTransform::datumTransformCrsInfo()
       */
      int destinationTransformId = -1;

      bool operator==( const QgsDatumTransform::TransformPair &other ) const
      {
        return other.sourceTransformId == sourceTransformId && other.destinationTransformId == destinationTransformId;
      }

      bool operator!=( const QgsDatumTransform::TransformPair &other ) const
      {
        return other.sourceTransformId != sourceTransformId || other.destinationTransformId != destinationTransformId;
      }

    };

    /**
     * Contains datum transform information.
     * \since QGIS 3.0
     */
    struct TransformInfo
    {
      //! Datum transform ID
      int datumTransformId = -1;

      //! EPSG code for the transform, or 0 if not found in EPSG database
      int epsgCode = 0;

      //! Source CRS auth ID
      QString sourceCrsAuthId;

      //! Destination CRS auth ID
      QString destinationCrsAuthId;

      //! Source CRS description
      QString sourceCrsDescription;

      //! Destination CRS description
      QString destinationCrsDescription;

      //! Transform remarks
      QString remarks;

      //! Scope of transform
      QString scope;

      //! True if transform is the preferred transform to use for the source/destination CRS combination
      bool preferred = false;

      //! True if transform is deprecated
      bool deprecated = false;

    };
};

#endif // QGSDATUMTRANSFORM_H
