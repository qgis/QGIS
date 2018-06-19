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

class QgsCoordinateReferenceSystem;


/**
 * Contains methods and classes relating the datum transformations.
 * \ingroup core
 *
 * \see QgsCoordinateTransformContext
 * \see QgsCoordinateTransform
 *
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

      bool operator==( QgsDatumTransform::TransformPair other ) const
      {
        return other.sourceTransformId == sourceTransformId && other.destinationTransformId == destinationTransformId;
      }

      bool operator!=( QgsDatumTransform::TransformPair other ) const
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

    /**
     * Returns a list of datum transformations which are available for the given \a source and \a destination CRS.
     * \see datumTransformToProj()
     * \see datumTransformInfo()
     */
    static QList< QgsDatumTransform::TransformPair > datumTransformations( const QgsCoordinateReferenceSystem &source, const QgsCoordinateReferenceSystem &destination );

    /**
     * Returns a proj string representing the specified \a datumTransformId datum transform ID.
     * \see datumTransformations()
     * \see datumTransformInfo()
     * \see projStringToDatumTransformId()
     */
    static QString datumTransformToProj( int datumTransformId );

    /**
     * Returns the datum transform ID corresponding to a specified proj \a string.
     * Returns -1 if matching datum ID was not found.
     * \see datumTransformToProj()
     */
    static int projStringToDatumTransformId( const QString &string );

    /**
     * Returns detailed information about the specified \a datumTransformId.
     * If \a datumTransformId was not a valid transform ID, a TransformInfo with TransformInfo::datumTransformId of
     * -1 will be returned.
     * \see datumTransformations()
     * \see datumTransformToProj()
    */
    static QgsDatumTransform::TransformInfo datumTransformInfo( int datumTransformId );

  private:

    static void searchDatumTransform( const QString &sql, QList< int > &transforms );


};

#endif // QGSDATUMTRANSFORM_H
